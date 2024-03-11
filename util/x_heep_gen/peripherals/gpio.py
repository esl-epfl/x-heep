from typing import Dict
from x_heep_gen.pads import IoInputEP, IoOutputEP, IoOutputEnEP, Pad
from x_heep_gen.peripherals.peripheral_helper import peripheral_from_file
from x_heep_gen.signal_routing.endpoints import InterruptEP, InterruptPlicEP
from x_heep_gen.signal_routing.node import Node
from x_heep_gen.signal_routing.routing_helper import RoutingHelper


@peripheral_from_file("./hw/vendor/pulp_platform_gpio/gpio_regs.hjson")
class GpioPeripheral():
    def __init__(self, *args, **kwargs) -> None:
        self._gpios_used: Dict[int, int] = kwargs.pop("gpios_used")
        self._intr_map: Dict[int, str] = kwargs.pop("intr_map", {i:"plic" for i in self._gpios_used.values()})
        super().__init__(*args, **kwargs)
    
    def register_connections(self, rh: RoutingHelper, p_node: Node):
        super().register_connections(rh, p_node)
        for p_num, io_num in self._gpios_used.items():
            rh.add_source(p_node, Pad.name_to_target_name("gpio", io_num)+"_i", IoInputEP(), Pad.name_to_target_name("gpio", io_num)+"_i")
            rh.add_source(p_node, Pad.name_to_target_name("gpio", io_num)+"_o", IoOutputEP(), Pad.name_to_target_name("gpio", io_num)+"_o")
            rh.add_source(p_node, Pad.name_to_target_name("gpio", io_num)+"_oe", IoOutputEnEP(), Pad.name_to_target_name("gpio", io_num)+"_oe")

            if self._intr_map[io_num] == "fast":
                rh.add_source(p_node, f"gpio_{io_num}_intr_o", InterruptEP())
            elif self._intr_map[io_num] == "plic":
                rh.add_source(p_node, f"gpio_{io_num}_intr_o", InterruptPlicEP())
        
    
    def make_instantiation(self, rh: RoutingHelper) -> str:
        out = ""
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_intr;"
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_in;"
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_out;"
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_out_en;"
        
        for i in range(32):
            if i in self._gpios_used:
                io_num = self._gpios_used[i]
                intr_sig = rh.use_source_as_sv(f"gpio_{io_num}_intr_o", self._p_node)
                in_sig = rh.use_source_as_sv(Pad.name_to_target_name("gpio", io_num)+"_i", self._p_node)
                out_sig = rh.use_source_as_sv(Pad.name_to_target_name("gpio", io_num)+"_o", self._p_node)
                oe_sig = rh.use_source_as_sv(Pad.name_to_target_name("gpio", io_num)+"_oe", self._p_node)
                

                out += f"assign {intr_sig} = {self.name}_{self._sp_name_suffix}_intr[{i}];"
                out += f"assign {self.name}_{self._sp_name_suffix}_in[{i}] = {in_sig};"
                out += f"assign {out_sig} = {self.name}_{self._sp_name_suffix}_out[{i}];"
                out += f"assign {oe_sig} = {self.name}_{self._sp_name_suffix}_out_en[{i}];"
            else:
                out += f"assign {self.name}_{self._sp_name_suffix}_in[{i}] = 1'b0;"

            out += "\n\n"
        
        return out + super().make_instantiation(rh) 
    
    def make_instantiation_connections(self, rh: RoutingHelper) -> str:
        out = ""
        out += f".gpio_in({self.name}_{self._sp_name_suffix}_in),"
        out += f".gpio_out({self.name}_{self._sp_name_suffix}_out),"
        out += f".gpio_tx_en_o({self.name}_{self._sp_name_suffix}_out_en),"
        out += f".gpio_in_sync_o(),"
        out += f".pin_level_interrupts_o({self.name}_{self._sp_name_suffix}_intr),"
        out += f".global_interrupt_o(),"

        return super().make_instantiation_connections(rh) + out