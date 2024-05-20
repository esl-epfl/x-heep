
from typing import List
from x_heep_gen.signal_routing.routing_helper import RoutingHelper
from .basic_peripheral import BasicPeripheral


class RegIfacePeripheral(BasicPeripheral):
    def make_instantiation_connections(self, rh: RoutingHelper) -> str:
        inst: str = ""
        inst += f".reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::{self.full_name.upper()}_IDX]),"
        inst += f".reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::{self.full_name.upper()}_IDX]),"

        return super().make_instantiation_connections(rh) + inst
    
    def make_instantiation_generics(self) -> List[str]:
        req_gen = [
            "reg_req_t(reg_pkg::reg_req_t)",
            "reg_rsp_t(reg_pkg::reg_rsp_t)"
        ]
        return super().make_instantiation_generics() + req_gen