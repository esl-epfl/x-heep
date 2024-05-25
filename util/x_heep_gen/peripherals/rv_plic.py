from typing import Set
from x_heep_gen.peripherals.peripheral_helper import peripheral_from_file
from x_heep_gen.signal_routing.endpoints import InterruptDirectEP, InterruptPlicEP
from x_heep_gen.signal_routing.node import Node
from x_heep_gen.signal_routing.routing_helper import RoutingHelper


@peripheral_from_file("./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson", name="rv_plic")
class RvPlicPeripheral():
    def _rv_plic_target_name(self) -> str:
        return "plic_target"
    
    def _rv_plic_source_name_irq(self) -> str:
        return f"{self.name}_{self._sp_name_suffix}_irq"
    
    def _rv_plic_source_name_msip(self) -> str:
        return f"{self.name}_{self._sp_name_suffix}_msip"

    def register_connections(self, rh: RoutingHelper, p_node: Node):
        super().register_connections(rh, p_node)
        rh.add_target(p_node, self._rv_plic_target_name(), InterruptPlicEP())
        rh.add_source(p_node, self._rv_plic_source_name_irq(), InterruptDirectEP(), "irq_external")
        rh.add_source(p_node, self._rv_plic_source_name_msip(), InterruptDirectEP(), "irq_software")

    def make_instantiation_connections(self, rh: RoutingHelper) -> str:
        out = f".intr_src_i({self.name}_{self._sp_name_suffix}_intr_vector),"
        out += ".irq_id_o(),"
        out += f".irq_o({rh.use_source_as_sv(self._rv_plic_source_name_irq(), self._p_node)}),"
        out += f".msip_o({rh.use_source_as_sv(self._rv_plic_source_name_msip(), self._p_node)}),"


        return super().make_instantiation_connections(rh) + out

    def make_instantiation(self, rh: RoutingHelper) -> str:
        intr_sources = rh.use_target_as_sv_multi(self._rv_plic_target_name(), self._p_node)

        out = ""
        out += f"logic [rv_plic_reg_pkg::NumTarget-1:0] {self.name}_{self._sp_name_suffix}_irq_plic;"
        out += f"logic [rv_plic_reg_pkg::NumSrc-1:0] {self.name}_{self._sp_name_suffix}_intr_vector;\n\n"
        out += f"assign {self.name}_{self._sp_name_suffix}_intr_vector[0] = 1'b0;  // ID [0] is a special case and must be tied to zero.\n"
        for i, intr in enumerate(intr_sources):
            out += f"assign {self.name}_{self._sp_name_suffix}_intr_vector[{i+1}] = {intr};"
        for i in range(i+1, 64):
             out += f"assign {self.name}_{self._sp_name_suffix}_intr_vector[{i}] = 1'b0;"
        out += "\n\n"

        return out + super().make_instantiation(rh)
    
    def make_intr_defs(self, rh: RoutingHelper) -> str:
        out = ""
        intr_sources = rh.use_target_as_sv_multi(self._rv_plic_target_name(), self._p_node)
        for i, intr in enumerate(intr_sources):
            intr = intr.split(".")[-1]
            out += f"#define {intr.upper()} {i+1}\n"
        return out
    
    def make_handler_array(self, rh: RoutingHelper) -> str:
        out = "handler_irq_dummy,"
        intr_sources = rh.use_target_as_sv_multi(self._rv_plic_target_name(), self._p_node)
        for i, intr in enumerate(intr_sources):
            ep: InterruptPlicEP = rh.get_source_ep_copy(intr.split(".")[-1])
            out += f"{ep.handler},"
        for _ in range(i+1, 64):
             out += "handler_irq_dummy,"
        return out
    
    def make_handler_set(self, rh: RoutingHelper) -> Set[str]:
        out = {"handler_irq_dummy"}
        intr_sources = rh.use_target_as_sv_multi(self._rv_plic_target_name(), self._p_node)
        for intr in intr_sources:
            ep: InterruptPlicEP = rh.get_source_ep_copy(intr.split(".")[-1])
            out.add(ep.handler)
        return out
