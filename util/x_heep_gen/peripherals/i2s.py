
from typing import Any, Dict, List

import hjson
from .peripheral_helper import PeripheralConfigFactory, peripheral_from_file
from ..config_helpers import to_bool
from ..signal_routing.endpoints import DmaTriggerEP
from ..signal_routing.node import Node
from ..signal_routing.routing_helper import RoutingHelper

class I2SConfigFactory(PeripheralConfigFactory):
    """
    A class adding configureation information for i2s peripherals.
    """
    def dict_to_kwargs(self, d: hjson.OrderedDict) -> Dict[str, Any]:
        ret = dict()
        dma = None
        if "dma" in d:
            dma = to_bool(d.pop("dma"))
            if dma is None:
                raise RuntimeError("dma parameter should be a boolean.")
        else:
            raise RuntimeError("i2s require the dma argument to be set")
        
        ret["dma"] = dma

        ret.update(super().dict_to_kwargs(d))
        return ret

@peripheral_from_file("./hw/ip/i2s/data/i2s.hjson", config_factory_t=I2SConfigFactory)
class I2SPeripheral():
    """
    A peripheral used to represent an `i2s` periperhal.
    
    :param bool dma: should the dma triggers be connected?
    """
    def __init__(self, *args, **kwargs):
        if not "dma" in kwargs:
            raise TypeError("dma keyword argument is required")
        self._i2s_uses_dma: bool = kwargs.pop("dma")
        if type(self._i2s_uses_dma) is not bool:
            raise RuntimeError("dma flag should be of type bool")
        super().__init__(*args, **kwargs)
    
    def register_connections(self, rh: RoutingHelper, p_node: Node):
        super().register_connections(rh, p_node)
        if self._i2s_uses_dma:
            rh.add_source(p_node, f"{self.name}_{self._sp_name_suffix}_rx", DmaTriggerEP())

    def get_io_prefix(self) -> str:
        return "i2s_"
    
    def get_io_connections(self) -> List[Dict]:
        d = [
            {"name": "sck", "type": "inout", "width": 1},
            {"name": "ws" , "type": "inout", "width": 1},
            {"name": "sd" , "type": "inout", "width": 1},
        ]
        return d

    def make_instantiation_connections(self, rh: RoutingHelper) -> str:
        out = ""
        if self._i2s_uses_dma:
            out += f".i2s_rx_valid_o({rh.use_source_as_sv(f'{self.name}_{self._sp_name_suffix}_rx', self._p_node)}),"
        else:
            out += ".i2s_rx_valid_o(),"
        return super().make_instantiation_connections(rh) + out
    
    def get_io_suffix(self) -> Dict[str, str]:
        return {"i": "i", "o": "o", "oe": "oe_o"}