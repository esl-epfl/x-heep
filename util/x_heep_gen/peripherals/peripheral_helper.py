
import functools
from typing import Any, Dict, List, Optional, Set, Type
import hjson
import os
from reggen.bus_interfaces import BusProtocol
from reggen.ip_block import IpBlock

from ..config_helpers import to_int
from .basic_peripheral import BasicPeripheral
from .reg_iface_peripheral import RegIfacePeripheral
from .tlul_peripheral import TLULPeripheral

ip_block_paths: Set[str] = { # prefill with fixed only peripherals
    "./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson",
    "./hw/ip/power_manager/data/power_manager.hjson",
    "./hw/ip/dma/data/dma.hjson",
    "./hw/ip/soc_ctrl/data/soc_ctrl.hjson"
}
"""All the paths used to make peripherals headers"""

class PeripheralConfigFactory():
    """
    A class providing individual peripheral configuration options.

    Peripheral makers should implement `dict_to_kwargs` to parse additional parameters. If `__init__` is overriden `super()` has to be called or its functionality has to be replaced.

    :param Type[BasicPeripheral]: The type of the peripheral to configure.
    """
    def __init__(self, t: Type[BasicPeripheral]) -> None:
        self.t = t

    def dict_to_kwargs(self, d: hjson.OrderedDict) -> Dict[str, Any]:
        """
        Converts a dictionary of parameters to a dictionary of kwargs to create the peripheral class.

        Here the parameters can be checked a first time.

        This provides an implementation for `offset` and `length`parameter.

        :param hjson.OrderedDict d:
        :return: kwargs
        :rtype: Dict[str, Any]
        """
        ret = dict()
        if "offset" in d:
            addr = to_int(d.pop("offset"))
            if addr is None:
                raise RuntimeError("If address is provided it should be an integer")
            ret["offset"] = addr

        if "length" in d:
            length = to_int(d.pop("length"))
            if length is None:
                raise RuntimeError("If address is provided it should be an integer")
            ret["addr_size"] = length
        
        return ret
        

    def from_odict(self, p_dict: hjson.OrderedDict) -> BasicPeripheral:
        """
        Creates the peripheral instance based on a config file dictionary.

        :param hjson:OrderedDict p_dict: parameters
        :return: The peripheral instance.
        :rtype: BasicPeripheral
        """
        return self.t(**self.dict_to_kwargs(p_dict))


peripheral_factories: Dict[str, PeripheralConfigFactory] = dict()
def add_peripheral_factory(name: str, t: Type[PeripheralConfigFactory], periph_t: Type[BasicPeripheral]):
    """manually add a peripheral factory"""
    peripheral_factories[name] = t(periph_t)

def peripheral_from_file(path: str, name: str = "", config_factory_t: Type[PeripheralConfigFactory] = PeripheralConfigFactory):
    """
    wrapper for peripheral classes

    It automatically adds parent classes based on a file, also it extracts interrupts and io signals if provided.

    :param str path: the path of the file
    :param str name: the name if the name in the file is wrong. defualt to the one provided in the file.
    :param Type[PeripheralConfigFactory] config_factory_t: the class to generate the configs info for configuration files, if no special options are needed a sane default is used.
    """
    if "X_HEEP_PROJECT_ROOT" in os.environ:
        path = os.environ["X_HEEP_PROJECT_ROOT"]+path
    ip_block_paths.add(path)
    name_c = name
    def deco_peripheral_from_file(cls):
        name = name_c
        ip_block = IpBlock.from_path(path, [])
        if name == "":
            name = ip_block.name
        if_list = ip_block.bus_interfaces.interface_list
        if len(if_list) == 0:
            raise NotImplementedError("Peripheral description without bus interface are not supported")
        if len(if_list) > 1:
            raise NotImplementedError("Peripheral decription with more than one bus interface are not supported")
        if if_list[0]["is_host"]:
            raise NotImplementedError("Only device interfaces are supported")
        if "protocol" not in if_list[0]:
            raise ValueError("Protocol not specified")
        
        proto = if_list[0]["protocol"]

        

        Base = None
        if proto is BusProtocol.TLUL:
            Base = TLULPeripheral
        elif proto is BusProtocol.REG_IFACE:
            Base = RegIfacePeripheral
        else:
            raise RuntimeError
        
        @functools.wraps(cls, updated=())
        class PeriphWrapper(cls, Base):
            IP_BLOCK = ip_block
            def __init__(self, domain: "str | None" = None, **kwargs):
                super().__init__(name, domain, **kwargs)
                self._ip_block = self.IP_BLOCK
                self._ip_block_path = path
        
        add_peripheral_factory(name, config_factory_t, PeriphWrapper)

        return PeriphWrapper
    
    return deco_peripheral_from_file
    