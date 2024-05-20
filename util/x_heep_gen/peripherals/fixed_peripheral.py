from typing import Optional
from x_heep_gen.peripherals.basic_peripheral import BasicPeripheral
from x_heep_gen.signal_routing.node import Node
from x_heep_gen.signal_routing.routing_helper import RoutingHelper
from .peripheral_helper import ip_block_paths

class FixedPeripheral(BasicPeripheral):
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