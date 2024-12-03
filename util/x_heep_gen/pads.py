from copy import deepcopy
from enum import *
from itertools import chain
from typing import Dict, Iterable, List, Optional, Tuple
import hjson

from .signal_routing.endpoints import Endpoint

from .config_helpers import to_bool, to_int
from .signal_routing.routing_helper import RoutingHelper
from .signal_routing.node import Node

@unique
class PadActive(Enum):
    """Pad active level"""
    LOW = "low"
    HIGH = "high"

@unique
class PadType(Enum):
    """Type of pads"""
    INPUT = "input"
    OUTPUT = "output"
    INOUT = "inout"

@unique
class IODirection(Enum):
    """Direction of an io signal"""
    INPUT = "input"
    OUTPUT = "output"
    


class IoEP(Endpoint):
    """Endpoint representing a general internal io connections"""
    def __init__(self):
        super().__init__()
        self.t = "logic"
        self.reserved = True

class IoInputEP(IoEP):
    """Endpoint representing an input internal io connections"""
    def __init__(self):
        super().__init__()
        self.source_direction: str = "input"
        self.target_direction: str = "output"

    def target_nc(self) -> str:
        return ""



class IoOutputEP(IoEP):
    """Endpoint representing an output internal io connections"""
    def __init__(self):
        super().__init__()
        self.source_direction: str = "output"
        self.target_direction: str = "input"

    def target_nc(self) -> str:
        return "1'b0"

class IoOutputEnEP(IoEP):
    """Endpoint representing an output enable internal io connections"""
    def __init__(self):
        super().__init__()
        self.source_direction: str = "output"
        self.target_direction: str = "input"

    def target_nc(self) -> str:
        return "1'b1"




MAPPING_DICT = {
    'top' : 'core_v_mini_mcu_pkg::TOP',
    'right' : 'core_v_mini_mcu_pkg::RIGHT',
    'bottom' : 'core_v_mini_mcu_pkg::BOTTOM',
    'left' : 'core_v_mini_mcu_pkg::LEFT'
}

CELL_PATTERNS = {
PadType.INPUT:
"""
  pad_cell_input #(
      .PADATTR({attr_bits}),
      .SIDE({side})
  ) pad_{name}_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o({in}),
      .pad_io({io_signal}),
      .pad_attributes_i({attr})
  );
""",
PadType.OUTPUT:
"""
  pad_cell_output #(
      .PADATTR({attr_bits}),
      .SIDE({side})
  ) pad_{name}_i (
      .pad_in_i({out}),
      .pad_oe_i({out_e}),
      .pad_out_o(),
      .pad_io({io_signal}),
      .pad_attributes_i({attr})
  );
""",
PadType.INOUT:
"""
  pad_cell_inout #(
      .PADATTR({attr_bits}),
      .SIDE({side})
  ) pad_{name}_i (
      .pad_in_i({out}),
      .pad_oe_i({out_e}),
      .pad_out_o({in}),
      .pad_io({io_signal}),
      .pad_attributes_i({attr})
  );
"""
}

class Pad:
    """
    A class representing a pad or an element of a pad multilpexer.
    """
    def __init__(self, name: str, num: int, pad_type: PadType, keep_internal: bool = False, pad_active: PadActive = PadActive.HIGH, num_offset = 0, mapping: str = "top", no_num: bool = False):
        """

        :param str name: the name of the pad
        :param int num: the number of pads (width)
        :param PadType pad_type: the type of pad.
        :param bool keep_internal: keep_internal parameter
        :param PadActive pad_active: the level where the pad is active
        :param int num_offset: the offset to the pad numbers
        :param str mapping: the position of the pad
        :param bool no_num: if True the number is not added to the pad signal name.
        """
        self._name = name
        self._num = num
        self._pad_type = pad_type
        self._keep_internal = keep_internal
        self._pad_type = pad_type
        self._pad_active = pad_active
        self._num_offset = num_offset
        self._mapping = mapping
        self._no_num = no_num

    @staticmethod
    def from_config(pad_name: str, pad_config: hjson.OrderedDict, in_mux: bool = False) -> "Pad":
        """
        Make a pad from a configuration dictionary

        :param str pad_name: the name of the pad
        :param hjson.OrderedDict pad_config: the dictionary containing the configuration
        :param bool in_mux: signal that the function is called recursively to read multiplexed pads.
        :return: A pad or multiplexed pad
        :rtype: Pad
        """
        if type(pad_name) is not str:
            raise TypeError("pad_name should be of type str")
        if type(pad_config) is not hjson.OrderedDict:
            raise TypeError("pad_config should be of type hjson.OrderdDict")
        
        num = None
        if not "num" in pad_config:
            num = 1
        else:
            num = to_int(pad_config["num"])
        if num is None:
            raise RuntimeError("the num parameter should have an integer")
        
        if not "type" in pad_config:
            raise RuntimeError("pads must have a type field.")
        
        pad_type_str = pad_config["type"]
        pad_type_values = list(map(lambda x: x.value, PadType.__members__.values()))

        if not pad_type_str in pad_type_values:
            raise RuntimeError(f"the type of a pad should be one of {pad_type_values}")
        pad_type = PadType(pad_type_str)

        no_num = False
        if "no_num" in pad_config:
            no_num = to_bool(pad_config["no_num"])
            if no_num is None:
                raise RuntimeError("no_num should be a boolean")

        keep_internal = False
        if "keep_internal" in pad_config:
            keep_internal = to_bool(pad_config["keep_internal"])
            if keep_internal is None:
                raise RuntimeError("keep_internal should be a boolean")
        
        pad_active = PadActive.HIGH
        if "active" in pad_config:
            pad_active_str = pad_config["active"]
            pad_active_values = list(map(lambda x: x.value, PadActive.__members__.values()))
            if not pad_active_str in pad_active_values:
                raise RuntimeError(f"the active field of a pad should be one of {pad_active_values}")
            pad_active = PadActive(pad_active_str)            

        num_offset = 0
        if "num_offset" in pad_config:
            num_offset = to_int(pad_config["num_offset"])
            if num_offset is None:
                raise RuntimeError("num_offset field should have an integer as parameter")

        mapping = "top"
        if "mapping" in pad_config:
            mapping = pad_config["mapping"]
            if type(mapping) is not str:
                raise RuntimeError("mapping should be a string")
            if not mapping in ["left", "right", "top", "bottom"]:
                raise RuntimeError("mapping should be bottom, top, left or right")

        if "mux" in pad_config:
            if in_mux:
                raise RuntimeError("nested muxed pads are not supported.")
            
            mux_raw = pad_config["mux"]
            if type(mux_raw) is not hjson.OrderedDict:
                raise RuntimeError("the mux field of a pad should be a dictionary.")
            
            mux: List[Pad] = []
            for key, value in mux_raw.items():
                if type(key) is not str:
                    raise RuntimeError("names in the mux field should be strings")
                if type(value) is not hjson.OrderedDict:
                    raise RuntimeError("values in the mux field should be dictionaries")
                
                mux.append(Pad.from_config(key, value, in_mux=True))

            return MuxedPad(pad_name, num, pad_type, mux_in=mux, keep_internal=keep_internal, pad_active=pad_active, num_offset=num_offset, mapping=mapping, no_num=no_num)
        else:
            return Pad(pad_name, num, pad_type, keep_internal, pad_active, num_offset, mapping=mapping, no_num=no_num)
    
    @staticmethod
    def name_to_target_name(name: str, i: int) -> str:
        """
        gets the name used for io targets without suffix
        
        Example: `Pad.name_to_target_name(pad_name, index)+"_i"`

        :param str name: the name of the pad
        :param int i: the index of the pad
        :return: the name of the target without suffix
        :rtype: str
        """
        return f"io_pad_target_{name}_{i}"
    
    def target_name(self, i: int) -> str:
        """
        like `name_to_target_name` but relative to the instance

        :param int i: the index of the pad relative to this instance offset
        :return: the name of the target without suffix
        :rtype: str
        """
        return self.name_to_target_name(self._name, i + self._num_offset)
    
    def io_name(self, i:int) -> str:
        """
        Gets the name of the outward going singal.
        
        :param int i: the index of the pad relative to this instance offset
        :return: the name of the outward facing signal.
        :rtype: str
        """
        suffix = "n" if self._pad_active == PadActive.LOW else ""
        suffix += {
            PadType.INPUT: "i",
            PadType.OUTPUT: "o",
            PadType.INOUT: "io",
        }[self._pad_type]

        num = f"_{i + self._num_offset}" if not self._no_num else ""

        return f"{self._name}{num}_{suffix}"

    def _internal_pre_route(self, rh: RoutingHelper, node: Node, route_root: bool):
        """
        Internal methode for pre routing
        """
        for i in range(self._num):
            if self._pad_type in [PadType.INPUT, PadType.INOUT]:
                rh.add_target(node, self.target_name(i)+"_i", IoInputEP())
            if self._pad_type in [PadType.OUTPUT, PadType.INOUT]:
                rh.add_target(node, self.target_name(i)+"_o", IoOutputEP())
                rh.add_target(node, self.target_name(i)+"_oe", IoOutputEnEP())
    
    def pre_route(self, rh: RoutingHelper, pad_ring_node: Node):
        """
        Task to be done before routing

        :param RoutingHelper rh: the routing helper
        :param Node pad_ring_node: the pad ring node
        """
        self._internal_pre_route(rh, pad_ring_node, True)

    def _make_pad_cell_tsig(self, i: int) -> str:
        """Internal method to get the name of the signals of the pad cells"""
        return self.target_name(i)

    def make_pad_cell(self, rh: RoutingHelper, p_node: Node, attr_bits: int) -> str:
        """
        Makes the sv code of the pad cells.

        :param RoutingHelper rh: the routing helper
        :param Node p_node: the parent node
        :param int attr_bits: the number of attr bits
        :return: sv code
        :rtype: str
        """
        pads = ""
        for i in range(self._num):
            signals = dict()
            if self._pad_type in [PadType.INPUT, PadType.INOUT]:
                signals["in"] = rh.use_target_as_sv(self._make_pad_cell_tsig(i)+"_i", p_node)
            if self._pad_type in [PadType.OUTPUT, PadType.INOUT]:
                signals["out"] = rh.use_target_as_sv(self._make_pad_cell_tsig(i)+"_o", p_node)
                signals["out_e"] = rh.use_target_as_sv(self._make_pad_cell_tsig(i)+"_oe", p_node)

            pads += CELL_PATTERNS[self._pad_type].format(
                name = f"{self._name}_{i}",
                **signals,
                io_signal = self.io_name(i),
                side = MAPPING_DICT[self._mapping],
                attr_bits = attr_bits,
                attr = f"pad_attributes_i[core_v_mini_mcu_pkg::{self.idx_name(i)}]" if attr_bits != 0 else "'0"
            )
        return pads
    
    def get_name(self) -> str:
        """
        :return: the name of this pad
        :rtype: str
        """
        return self._name
    
    def get_num(self) -> int:
        """
        :return: the number of pads represented by the instance
        :rtype: int
        """
        return self._num
    
    def get_type(self) -> PadType:
        """
        :return: the type of pad
        :rytpe: PadType
        """
        return self._pad_type
    
    def make_root_io_ports(self, internal: bool) -> str:
        """
        Makes the sv code for the definition of outwards facing singals.

        :param bool internal: if True this is used to make an internal connection
        :return: the sv code
        :rtype: str
        """
        if self._keep_internal is True and internal is False:
            return ""
        
        out = ""
        for i in range(self._num):
            out += f"inout wire {self.io_name(i)},"
        
        return out
    
    def make_root_io_ports_use(self, internal) -> str:
        """
        Makes the sv code for the use of outwards facing singals in module instantiation.

        :param bool internal: if True this is used to make an internal connection
        :return: the sv code
        :rtype: str
        """
        if self._keep_internal is True and internal is False:
            return ""
        
        out = ""
        for i in range(self._num):
            out += f".{self.io_name(i)},"
        
        return out
    
    def idx_name(self, i: int) -> str:
        """
        The name used as index in arrays conserning pads

        :param int i: the index of the pad relative to this instance offset
        :return: The index name of this pad.
        :rtype: str
        """
        return f"PAD_{self._name}_{self._num_offset + i}".upper()
    
    def iterate_pad_index(self) -> Iterable[str]:
        """
        Iterate all possible result of `idx_name()`

        :return: an iterator over all index names
        :rtype: Iterable[str]
        """
        return iter([self.idx_name(i) for i in range(self._num)])





class MuxedPad(Pad):
    """
    A representation of a multiplexed pad
    """
    def __init__(self, *args, mux_in: List[Pad], **kwargs):
        """
        
        :param List[Pad] mux_in: list of pads that should act as one multiplexed pad.
        :param str name: the name of the pad
        :param int num: the number of pads (width)
        :param PadType pad_type: the type of pad.
        :param bool keep_internal: keep_internal parameter
        :param PadActive pad_active: the level where the pad is active
        :param int num_offset: the offset to the pad numbers
        :param str mapping: the position of the pad
        :param bool no_num: if True the number is not added to the pad signal name.
        """
        super().__init__(*args, **kwargs)
        self._sub_pads = mux_in
    
    def get_mux_num(self) -> int:
        """
        :return the number of multiplexed pads
        :rtype: int
        """
        return len(self._sub_pads)
    
    def muxer_connection_name(self, i: int) -> str:
        """
        Internal name for connections to the muxer

        :param int i: the index of the pad relative to this instance offset
        :return: the name
        :rtype: str
        """
        return f"muxer_pad_connector_{self._name}_{i + self._num_offset}"

    def pre_route(self, rh: RoutingHelper, pad_ring_node: Node):
        for p in self._sub_pads:
            p._internal_pre_route(rh, rh.get_root_node(), False)
    
        
        for i in range(self._num):
            #rh.add_source(pad_ring_node, self.io_source_name(i), IoRootEP())

            cname = self.muxer_connection_name(i)
            if self._pad_type in [PadType.INPUT, PadType.INOUT]:
                rh.add_target(pad_ring_node, cname+"_i", IoInputEP())
                rh.add_source(rh.get_root_node(), cname+"_i", IoInputEP(), to = cname+"_i")
            if self._pad_type in [PadType.OUTPUT, PadType.INOUT]:
                rh.add_target(pad_ring_node, cname+"_o", IoOutputEP())
                rh.add_source(rh.get_root_node(), cname+"_o", IoOutputEP(), to = cname+"_o")
                
                rh.add_target(pad_ring_node, cname+"_oe", IoOutputEnEP())
                rh.add_source(rh.get_root_node(), cname+"_oe", IoOutputEnEP(), to = cname+"_oe")
    
    def _make_pad_cell_tsig(self, i: int) -> str:
        return self.muxer_connection_name(i)


    def _make_muxer_case_entry(self, rh: RoutingHelper, sub_p: Pad, name: str, i: int) -> str:
        """Internal methode to help making case entries"""
        out = f"{name}: begin\n"
        if sub_p._pad_type in [PadType.INPUT, PadType.INOUT]:
            sig_periph_in = rh.use_target_as_sv(sub_p.target_name(i)+"_i", rh.get_root_node())
            sig_to_pad_in = rh.use_source_as_sv(self.muxer_connection_name(i)+"_i", rh.get_root_node())
            if sig_periph_in != "":
                out += f"{sig_periph_in} = {sig_to_pad_in};"

        if sub_p._pad_type in [PadType.OUTPUT, PadType.INOUT]:
            sig_periph_o  = rh.use_target_as_sv(sub_p.target_name(i)+"_o", rh.get_root_node())
            sig_to_pad_o  = rh.use_source_as_sv(self.muxer_connection_name(i)+"_o", rh.get_root_node())
            sig_periph_oe = rh.use_target_as_sv(sub_p.target_name(i)+"_oe", rh.get_root_node())
            sig_to_pad_oe = rh.use_source_as_sv(self.muxer_connection_name(i)+"_oe", rh.get_root_node())
            out += f"{sig_to_pad_o} = {sig_periph_o};"
            out += f"{sig_to_pad_oe} = {sig_periph_oe};"
        elif self._pad_type in [PadType.OUTPUT, PadType.INOUT]:
            sig_to_pad_o  = rh.use_source_as_sv(self.muxer_connection_name(i)+"_o", rh.get_root_node())
            sig_to_pad_oe = rh.use_source_as_sv(self.muxer_connection_name(i)+"_oe", rh.get_root_node())
            out += f"{sig_to_pad_o} = 1'b0;"
            out += f"{sig_to_pad_oe} = 1'b0;"
        out += "end\n"
        return out

    def make_muxer(self, rh: RoutingHelper) -> str:
        """
        Make the sv code of multiplexers

        :param RoutingHelper rh: the routing helper
        :return: sv code
        :rtype: str
        """
        out = ""
        for i in range(self._num):
            out += "always_comb begin\n"
            for p in self._sub_pads:
                if p.get_type() in [PadType.INPUT, PadType.INOUT]:
                    sig_periph = rh.use_target_as_sv(p.target_name(i)+"_i", rh.get_root_node())
                    if sig_periph == "":
                        print(f"Warning: {p.target_name(i)+'_i'} is not connected maybe a peripheral is missing.")
                    else:
                        out += f"{sig_periph} = 1'b0;"
            out += f"unique case (pad_muxes[core_v_mini_mcu_pkg::{self.idx_name(i)}])"
            for j, p in enumerate(self._sub_pads):
                out += self._make_muxer_case_entry(rh, p, str(j), i)

            out += self._make_muxer_case_entry(rh, self._sub_pads[0], "default", i)

            out += "endcase\n"
            out += "end\n\n"
        return out
    
    def iterate_pad_index_with_num(self) -> Iterable[Tuple[str, int]]:
        """
        Iterate all possible result of `idx_name()` combined with the internal number

        :return: an iterator over all index names
        :rtype: Iterable[Tuple[str, int]]
        """
        return iter([(self.idx_name(i), len(self._sub_pads)) for i in range(self._num)])



class PadManager:
    """
    A class managing all pads
    """
    def __init__(self):
        self._pads: List[Pad] = []
        self._pad_ring_node: Optional[Node] = None
        self._attr_bits: int = 0

    def add_from_config(self, config: hjson.OrderedDict):
        """
        Add pads from a configuration file
        
        :param hjson.OrderedDict config: the configuration file content
        """
        self._attr_bits = to_int(config.pop("attributes", {}).pop("bits"))

        if "pads" in config:
            for pad, pad_config in config["pads"].items():
                if type(pad) is not str:
                    raise RuntimeError("pad names should be strings in configuration files")
                if type(pad_config) is not hjson.OrderedDict:
                    raise RuntimeError("pad configuration should be dictionaries.")
                
                self._pads.append(Pad.from_config(pad, pad_config))

    @staticmethod
    def load(src: str) -> "PadManager":
        """
        Creates an instance pased on a configuration file
        
        :param str src: the content of the hjson file
        :return: the pad manager instance
        :rtype: PadManager
        """
        manager = PadManager()

        config = hjson.loads(src, parse_int=int, object_pairs_hook=hjson.OrderedDict)
        manager.add_from_config(config)

        return manager

    def pre_route(self, rh: RoutingHelper):
        """
        Task to be done before routing

        :param RoutingHelper rh: the routing helper
        """
        self._pad_ring_node = Node("pad_ring", rh.get_root_node().name)
        rh.register_node(self._pad_ring_node)

        for pad in self._pads:
            pad.pre_route(rh, self._pad_ring_node)

    def make_pad_cells(self, rh: RoutingHelper) -> str:
        """
        Makes the sv code of the pad cells.

        :param RoutingHelper rh: the routing helper
        :return: sv code
        :rtype: str
        """
        out = ""
        for p in self._pads:
            out += p.make_pad_cell(rh, self._pad_ring_node, self._attr_bits)
            out += "\n\n"
        
        return out

    def make_muxers(self, rh: RoutingHelper) -> str:
        """
        Makes the sv code of the multiplexers.

        :param RoutingHelper rh: the routing helper
        :return: sv code
        :rtype: str
        """
        out = ""
        for p in self._pads:
            if isinstance(p, MuxedPad):
                out += p.make_muxer(rh)

        return out

    def make_root_io_ports(self, internal=False) -> str:
        """
        Makes the sv code for the definition of outwards facing singals.

        :param bool internal: if True this is used to make an internal connection
        :return: the sv code
        :rtype: str
        """
        out = ""
        for p in self._pads:
            out += p.make_root_io_ports(internal)
        return out
    
    def make_root_io_ports_use(self, internal=False) -> str:
        """
        Makes the sv code for the use of outwards facing singals in module instantiation.

        :param bool internal: if True this is used to make an internal connection
        :return: the sv code
        :rtype: str
        """
        out = ""
        for p in self._pads:
            out += p.make_root_io_ports_use(internal)
        return out


    def get_pad_ring_node(self) -> Node:
        """
        Get the node of the pad_ring
        
        :return: the pad_ring node
        :rtype: Node
        """
        return deepcopy(self._pad_ring_node)
    
    def iterate_pad_index(self) -> Iterable[str]:
        """
        Iterate all possible pad index name

        :return: an iterator over all index names
        :rtype: Iterable[Tuple[str, int]]
        """
        return chain.from_iterable([p.iterate_pad_index() for p in self._pads])
    
    def iterate_muxed_pad_index(self) -> Iterable[str]:
        """
        Iterate all possible muxed pad index name

        :return: an iterator over all index names
        :rtype: Iterable[Tuple[str, int]]
        """
        return chain.from_iterable([p.iterate_pad_index() for p in filter(lambda p: isinstance(p, MuxedPad), self._pads)])
    
    def iterate_muxed_pad_index_with_num(self) -> Iterable[Tuple[str, int]]:
        """
        Iterate all possible muxed pad index name, with the width of each name

        :return: an iterator over all index names
        :rtype: Iterable[Tuple[str, int]]
        """
        return chain.from_iterable([p.iterate_pad_index_with_num() for p in filter(lambda p: isinstance(p, MuxedPad), self._pads)])
    
    def get_pad_num(self) -> int:
        """
        :return: the number of pads in the system
        :rtype: int
        """
        count = 0
        for p in self._pads:
            count += p.get_num()
        return count
    
    def get_muxed_pad_num(self) -> int:
        """
        :return: the number of muxed pads in the system
        :rtype: int
        """
        count = 0
        for p in filter(lambda p: isinstance(p, MuxedPad), self._pads):
            count += p.get_num()
        return count
    
    def get_attr_bits(self) -> int:
        """
        :return: the number of attr_bits
        :rtype: int
        """
        return self._attr_bits
    
    def get_mk_ctrl(self) -> bool:
        """
        :return: shortcut to test if muxed pads are present or attributes bits are used.
        :rtype: bool
        """
        return self.get_muxed_pad_num() > 0 or self.get_attr_bits() != 0
    
    def get_max_mux_bitlengh(self) -> int:
        """
        :return: the maximum number of bits needed for all muxed pads control
        :rtype: int
        """
        pads = filter(lambda p: isinstance(p, MuxedPad), self._pads)
        return (max([p.get_mux_num() for p in pads])-1).bit_length()