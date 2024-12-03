from typing import Any, Dict, Optional

import hjson
from .basic_peripheral import BasicPeripheral
from .peripheral_helper import PeripheralConfigFactory, add_peripheral_factory, ip_block_paths
from ..signal_routing.node import Node
from ..signal_routing.routing_helper import RoutingHelper

class FixedPeripheralFactory(PeripheralConfigFactory):
    def dict_to_kwargs(self, d: hjson.OrderedDict) -> Dict[str, Any]:
        ret = dict()
        if "name" in d:
            ret["name"] = d.pop("name")
        else:
            raise RuntimeError("name is mandatory in fixed peripheral")
        
        if "suffix" in d:
            ret["suffix"] = d.pop("suffix")
        
        ret.update(super().dict_to_kwargs(d))
        return ret
class FixedPeripheral(BasicPeripheral):
    """
    A special kind of peripheral for hte fixed domain.

    name offset and address siue have to be set. 
    """
    def __init__(self, name: str, domain: Optional[str] = None, *, offset: int, addr_size: int, suffix: str = ""):
        if offset is None:
            raise RuntimeError("fixed peripherals must have the offset set.")
        if addr_size is None:
            raise RuntimeError("fixed peripherals must have addr_size set.")
        
        super().__init__(name, domain, offset, addr_size)

        self._sp_name_suffix = suffix

        for p in ip_block_paths:
            if p.endswith(f"{name}.hjson"):
                self._ip_block_path = p

    
    def register_connections(self, rh: RoutingHelper, p_node: Node):
        pass

    def set_specialized_name(self, suffix: str):
        pass

add_peripheral_factory("fixed", FixedPeripheralFactory, FixedPeripheral)