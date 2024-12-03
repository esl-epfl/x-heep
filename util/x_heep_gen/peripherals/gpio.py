from typing import Any, Dict

import hjson
from .peripheral_helper import PeripheralConfigFactory, peripheral_from_file
from ..config_helpers import to_int
from ..pads import IoInputEP, IoOutputEP, IoOutputEnEP, Pad
from ..signal_routing.endpoints import InterruptEP, InterruptPlicEP
from ..signal_routing.node import Node
from ..signal_routing.routing_helper import RoutingHelper

class GpioPeripheralConfigFactory(PeripheralConfigFactory):
    """
    A class adding configuration information for gpio peripherals.
    """
    def dict_to_kwargs(self, d: hjson.OrderedDict) -> Dict[str, Any]:
        ret = dict()

        gpios_used = {}
        io_offset = 0
        if "io_offset" in d:
            io_offset = to_int(d.pop("io_offset"))
            if io_offset is None:
                raise RuntimeError("io_offset should be an integer")
        
        if "gpios_used" in d:
            gu = d.pop("gpios_used")
            if isinstance(gu, list):
                for g in gu:
                    g = to_int(g)
                    if g is None:
                        raise RuntimeError("gpio numbers should be integers")
                    gpios_used[g] = g + io_offset

            elif isinstance(gu, hjson.OrderedDict):
                if io_offset != 0:
                    print("Warning: io_offset is not used for dictionaries in gpio config")
                for k, v in gu.items():
                    k = to_int(k)
                    v = to_int(v)
                    if v is None or k is None:
                        raise RuntimeError("gpio numbers should be integers")
                    gpios_used[k] = v
            
            elif gu == "full":
                gpios_used.update({i:i+io_offset for i in range(32)})

            elif gu == "range":
                start = to_int(d.pop("start", 0))
                end = to_int(d.pop("end", 31))
                if end is None or start is None or start < 0 or  end > 31 or start > end:
                    raise RuntimeError("start and end should be integers, in range 0-31, start <= end")
                gpios_used.update({i:i+io_offset for i in range(start, end+1)})

            else:
                raise RuntimeError("gpios_used should either be a dictionary, a list, range or full")
            
        intr = d.pop("intr", "plic")
        if not intr in ["plic", "fast"]:
            raise RuntimeError("intr parameter of gpio should either be fast or plic")
        
        intr_map = {i:intr for i in gpios_used.values()}

        ret["intr_map"] = intr_map
        ret["gpios_used"] = gpios_used

        ret.update(super().dict_to_kwargs(d))
        return ret

@peripheral_from_file("./hw/vendor/pulp_platform_gpio/gpio_regs.hjson", config_factory_t=GpioPeripheralConfigFactory)
class GpioPeripheral():
    """
    A class representing a gpio peripheral.

    :param Dict[int, int] gpios_used: a dictionairy mapping internal numbers to external number used for port names, a missing number is an unconnected number. Default to `{}`.
    :param Dict[int, str] intr_map: a dictionary mapping internal oi numbers to interrupt destinations, the destination are either `"fast"` or `"plic"`, defaults to `"plic"` for each io used in `gpios_used`.
    """
    def __init__(self, *args, **kwargs) -> None:
        self._gpios_used: Dict[int, int] = kwargs.pop("gpios_used", {})
        self._intr_map: Dict[int, str] = kwargs.pop("intr_map", {i:"plic" for i in self._gpios_used.values()})
        super().__init__(*args, **kwargs)
    
    def register_connections(self, rh: RoutingHelper, p_node: Node):
        super().register_connections(rh, p_node)
        for p_num, io_num in self._gpios_used.items():
            rh.add_source(p_node, Pad.name_to_target_name("gpio", io_num)+"_i", IoInputEP(), Pad.name_to_target_name("gpio", io_num)+"_i")
            rh.add_source(p_node, Pad.name_to_target_name("gpio", io_num)+"_o", IoOutputEP(), Pad.name_to_target_name("gpio", io_num)+"_o")
            rh.add_source(p_node, Pad.name_to_target_name("gpio", io_num)+"_oe", IoOutputEnEP(), Pad.name_to_target_name("gpio", io_num)+"_oe")

            if self._intr_map[io_num] == "fast":
                rh.add_source(p_node, f"gpio_{io_num}_intr", InterruptEP(handler=f"fic_irq_gpio_{io_num}_intr"))
            elif self._intr_map[io_num] == "plic":
                rh.add_source(p_node, f"gpio_{io_num}_intr", InterruptPlicEP(handler=f"handler_irq_gpio_{io_num}_intr"))
        
    
    def make_instantiation(self, rh: RoutingHelper) -> str:
        out = ""
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_intr;"
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_in;"
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_out;"
        out += f"logic [32-1:0] {self.name}_{self._sp_name_suffix}_out_en;"
        
        for i in range(32):
            if i in self._gpios_used:
                io_num = self._gpios_used[i]
                intr_sig = rh.use_source_as_sv(f"gpio_{io_num}_intr", self._p_node)
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