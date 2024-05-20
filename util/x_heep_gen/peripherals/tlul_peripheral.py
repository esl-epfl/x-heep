
from itertools import chain
from typing import Iterable
from x_heep_gen.peripherals.basic_peripheral import SvSignal
from .basic_peripheral import *

class TLULPeripheral(BasicPeripheral):
    def __init__(self, name: "str|None" = None, domain: "str|None" = None):
        super().__init__(name=name, domain=domain)



    @property
    def tlul_d2h_name(self) -> str:
        return f"{self.name}_{self._sp_name_suffix}_tl_d2h"
    
    @property
    def tlul_h2d_name(self) -> str:
        return f"{self.name}_{self._sp_name_suffix}_tl_h2d"

    def make_instantiation(self, rh: RoutingHelper) -> str:
        """docs"""

        return f"""reg_to_tlul #(
            .req_t(reg_pkg::reg_req_t),
            .rsp_t(reg_pkg::reg_rsp_t),
            .tl_h2d_t(tlul_pkg::tl_h2d_t),
            .tl_d2h_t(tlul_pkg::tl_d2h_t),
            .tl_a_user_t(tlul_pkg::tl_a_user_t),
            .tl_a_op_e(tlul_pkg::tl_a_op_e),
            .TL_A_USER_DEFAULT(tlul_pkg::TL_A_USER_DEFAULT),
            .PutFullData(tlul_pkg::PutFullData),
            .Get(tlul_pkg::Get)
        ) reg_to_tlul_{self.name}_{self._sp_name_suffix}_i (
            .tl_o({self.tlul_h2d_name}),
            .tl_i({self.tlul_d2h_name}),
            .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::{self.full_name.upper()}_IDX]),
            .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::{self.full_name.upper()}_IDX])
        );""" + "\n\n" + super().make_instantiation(rh)
    
    def make_instantiation_connections(self, rh: RoutingHelper) -> str:
        inst: str = f".tl_i({self.tlul_h2d_name}),"
        inst += f".tl_o({self.tlul_d2h_name}),"

        return super().make_instantiation_connections(rh) + inst
    
    def make_local_signals(self) -> Iterable[SvSignal]:
        sigs = [
            SvSignal("tlul_pkg::tl_h2d_t", self.tlul_h2d_name),
            SvSignal("tlul_pkg::tl_d2h_t", self.tlul_d2h_name),
        ]

        return chain(super().make_local_signals(), iter(sigs))
