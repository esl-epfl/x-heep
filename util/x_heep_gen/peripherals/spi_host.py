from typing import Any, Dict

import hjson
from x_heep_gen.config_helpers import to_bool
from x_heep_gen.peripherals.peripheral_helper import PeripheralConfigFactory, peripheral_from_file
from x_heep_gen.signal_routing.endpoints import DmaTriggerEP
from x_heep_gen.signal_routing.node import Node
from x_heep_gen.signal_routing.routing_helper import RoutingHelper


class SpiHostConfigFactory(PeripheralConfigFactory):
    def dict_to_kwargs(self, d: hjson.OrderedDict) -> Dict[str, Any]:
        ret = dict()
        dma = None
        if "dma" in d:
            dma = to_bool(d.pop("dma"))
            if dma is None:
                raise RuntimeError("dma parameter should be a boolean.")
        else:
            raise RuntimeError("spi_host require the dma argument to be set")
        
        if "event_is_fast_intr" in d:
            eifi = to_bool(d["event_is_fast_intr"])
            if eifi is None:
                raise RuntimeError("event_is_fast_intr should be a boolean")
            ret["event_is_fast_intr"] = eifi
        
        ret["dma"] = dma

        ret.update(super().dict_to_kwargs(d))
        return ret

@peripheral_from_file("./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson", config_factory_t=SpiHostConfigFactory)
class SpiHostPeripheral():
    def __init__(self, *args, event_is_fast_intr=False, **kwargs):
        if not "dma" in kwargs:
            raise TypeError("dma keyword argument is required")
        self._spi_uses_dma: bool = kwargs.pop("dma")
        if type(self._spi_uses_dma) is not bool:
            raise RuntimeError("dma flag should be of type bool")

        super().__init__(*args, **kwargs)
        self.interrupt_dest = {
            "error": "plic",
            "spi_event": "fast" if event_is_fast_intr else "plic"
        }
        self.interrupt_handler = {
            "error": "error",
            "spi_event": ""
        }
    
    def register_connections(self, rh: RoutingHelper, p_node: Node):
        super().register_connections(rh, p_node)
        if self._spi_uses_dma:
            rh.add_source(p_node, f"{self.name}_{self._sp_name_suffix}_rx", DmaTriggerEP())
            rh.add_source(p_node, f"{self.name}_{self._sp_name_suffix}_tx", DmaTriggerEP())

    def make_instantiation_connections(self, rh: RoutingHelper) -> str:
        out = ""
        out += ".alert_rx_i(),"
        out += ".alert_tx_o(),"
        out += ".passthrough_i(spi_device_pkg::PASSTHROUGH_REQ_DEFAULT),"
        out += ".passthrough_o(),"
        if self._spi_uses_dma:
            out += f".rx_valid_o({rh.use_source_as_sv(f'{self.name}_{self._sp_name_suffix}_rx', self._p_node)}),"
            out += f".tx_ready_o({rh.use_source_as_sv(f'{self.name}_{self._sp_name_suffix}_tx', self._p_node)}),"
        else:
            out += ".rx_valid_o(),"
            out += ".tx_ready_o(),"
        return super().make_instantiation_connections(rh) + out