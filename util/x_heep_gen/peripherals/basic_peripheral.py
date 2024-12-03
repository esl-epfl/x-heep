from dataclasses import *
from typing import Dict, Generator, Iterable, List, Optional, Union

import reggen
from reggen.ip_block import IpBlock

from .sv_helper import SvSignal
from ..pads import IoInputEP, IoOutputEP, IoOutputEnEP, Pad
from ..signal_routing.node import Node
from ..signal_routing.routing_helper import RoutingHelper
from ..signal_routing.endpoints import InterruptEP, InterruptPlicEP



class BasicPeripheral:
    """
    A class representing peripherals in general.

    This class is manly used through `@peripheral_from_file`.

    :param Optional[str] name: the base name of the peripheral
    :param Optional[str] domain: the name of the domain
    :param Optional[int] offset: the address offset relative to the peripheral domain.
    :param Optional[int] addr_size: the amount of space that should be used for this peripheral.
    """
    def __init__(self, name: "str|None" = None, domain: "str|None" = None, offset: Optional[int] = None, addr_size: Optional[int] = None):
        self.name: str|None = name
        """The name of the peripheral"""

        self._sp_name_suffix: str|None = self.name

        self.domain: str|None = domain
        """
        The name of the domain the peripheral is in.
        
        This will be set by `PeripheralDomain`.
        """

        self._offset: Optional[int] = offset
        self._addr_size: Optional[int] = addr_size

        self._ip_block: Optional[IpBlock] = None
        self._ip_block_path: Optional[str] = None

        self.io_local_idx: Optional[int] = None

        self.interrupt_dest: Union[str, Dict[str, str]] = "plic"
        self.interrupt_handler: Dict[str, str] = dict()
        self.interrupt_handler_base: Dict[str, str] = dict()

    
    @property
    def name(self) -> str:
        return self._name
    
    @property
    def full_name(self) -> str:
        """The name with it's suffix, how it will actually be used in the generated output."""
        if self._sp_name_suffix is not None and self._sp_name_suffix != "":
            return f"{self._name}_{self._sp_name_suffix}"
        return self.name
    
    @name.setter
    def name(self, value: "str|None"):
        if type(value) is not str and value is not None:
            raise TypeError("value should be of type str")
        
        self._name = value

    @property
    def domain(self) -> str:
        return self._domain
    
    @domain.setter
    def domain(self, value: "str|None"):
        if type(value) is not str and value is not None:
            raise TypeError("value should be of type str")
        
        self._domain = value
    

    

    def set_specialized_name(self, suffix: str):
        """
        Set the the suffix of the name.

        :param str suffix: suffix of the name
        :raise TypeError: if argument does not have the right type.
        """
        if type(suffix) is not str:
            raise TypeError("argument suffix should be of type str")
        self._sp_name_suffix  = suffix


    def specialize_name(self, n: int) -> int:
        """
        Adds a name suffix to the name based on the integer provided.

        To specialize this methode in a subclass `set_specialized_name` can be overriden.
        
        :param int n: number to be suffixed
        :return: next value for this name, here n+1, but a subclass could do a bigger increment
        :rtype: int
        :raise TypeError: if argument does not have the right type.
        """
        if type(n) is not int:
            raise TypeError("argument n should be of type int")
        self.set_specialized_name(str(n))
        return n + 1

    def _intr_sig_name(self, sig: reggen.signal.Signal) -> str:
        """
        Methode providing the name of the interrupt sources.
        """
        return f"{self.name}_{self._sp_name_suffix}_{sig.name}_intr"
    
    def _io_sig_name(self, sig: List[dict]) -> str:
        """
        Methode providing the name of io sources.
        """
        return f"{self.name}{self._sp_name_suffix}_{sig['name']}"
    
    def register_connections(self, rh: RoutingHelper, p_node: Node):
        """
        register connections for this peripheral.

        :param RoutingHelper rh: the routing helper
        :param Node p_node: the parent node.
        """
        self._p_node = p_node
        if self._ip_block is None:
            raise RuntimeError("No ip block configured")
        
        for sig in self._ip_block.interrupts:
            if type(self.interrupt_dest) is str:
                intr_dest = self.interrupt_dest
            elif type(self.interrupt_dest) is dict:
                intr_dest = self.interrupt_dest[sig.name]
            else:
                raise NotImplementedError()
            
            handler_suffix = ""
            if sig.name in self.interrupt_handler:
                handler_suffix = f"{self.interrupt_handler[sig.name]}"
            
            handler_name = self.full_name
            if sig.name in self.interrupt_handler_base:
                handler_name = f"{self.interrupt_handler_base[sig.name]}"

            EP = None
            if intr_dest == "fast":
                EP = InterruptEP(handler=f"fic_irq_{handler_name}{handler_suffix}")
            elif intr_dest == "plic":
                EP = InterruptPlicEP(handler=f"handler_irq_{handler_name}{handler_suffix}")
            else:
                raise NotImplementedError(f"interrupt destination {self.interrupt_dest} is not implemented")
            
            rh.add_source(p_node, self._intr_sig_name(sig), EP)
        
        for sig in self.get_io_connections():
            for i in range(sig["width"]):
                iosn = self._io_sig_name(sig)
                if sig["type"] in ["input", "inout"]:
                    rh.add_source(p_node, f"{iosn}{i}_i", IoInputEP(), to=Pad.name_to_target_name(iosn, i)+"_i")
                if sig["type"] in ["output", "inout", "output_no_en"]:
                    rh.add_source(p_node, f"{iosn}{i}_o", IoOutputEP(), to=Pad.name_to_target_name(iosn, i)+"_o")
                if sig["type"] in ["output", "inout"]:
                    rh.add_source(p_node, f"{iosn}{i}_oe", IoOutputEnEP(), to=Pad.name_to_target_name(iosn, i)+"_oe")
            


    def make_instantiation_connections(self, rh: RoutingHelper) -> str:
        """
        Make the connections for the sv code.

        This function can be overriden to add more signals. Generally the content of this function should be prepended.
        
        :param routingHelper rh: The routing helper
        :return: the connections to the periperhals in sv with trainling comma.
        :rtype: str
        """
        if self._ip_block is None:
            raise RuntimeError("No ip block configured")
        
        inst: str = ".clk_i(clk_cg),"
        inst += ".rst_ni,"

        for sig in self._ip_block.interrupts:
            lsig_name = rh.use_source_as_sv(self._intr_sig_name(sig), self._p_node)
            inst += f".intr_{sig.name}_o({lsig_name}),"

        inst += self._make_io_connections(rh)

        return inst

    def make_instantiation_generics(self) -> List[str]:
        """
        Get the generics used for the instantiation

        This should be overriden to add generics.

        :return: a list of parameters in sv.
        :rtype: List[str]
        """
        return []

    def make_instantiation(self, rh: RoutingHelper) -> str:
        """
        Make the sv code for this peripheral.

        This function can be extended to add other modules, or other things needed in sv in order to use this peripheral.
        Example `TLULPeripheral`.
        
        :param routingHelper rh: The routing helper
        :return: the connections to the periperhal.
        :rtype: str
        """
        if self._ip_block is None:
            raise RuntimeError("No ip block configured")

        inst: str = self.name

        generics = self.make_instantiation_generics()
        if len(generics) != 0:
            generics = [f".{g}" for g in generics]
            inst += "#("
            inst += ','.join(generics)
            inst += ")"
        
        inst += f" {self.name}_{self._sp_name_suffix}_i("

        inst += self.make_instantiation_connections(rh)

        if inst[-1] == ',':
            inst = inst[:-1]

        inst += ");"
        
        return inst
    
    def make_local_signals(self) -> Iterable[SvSignal]:
        """
        make signals for use only in the peripheral domain.
        
        For new things the routing helper should be prefered.
        """
        return iter([])
    

    def get_io_connections(self) -> List[Dict]:
        """
        Get io connections.

        This should be overriden to add new or missing io_connections.

        The `type`, `name` and `width` entry from the dictionaries are used.

        :return: a list of dictionaries
        :rtype: List[Dict]
        """
        return self._ip_block.get_signals_as_list_of_dicts()
    
    def get_io_prefix(self) -> str:
        """
        :return: the perfix for io signals.
        :rtype: str

        This should be overriden if the default does not match the rtl code.
        """
        return "cio_"
    
    def get_io_suffix(self) -> Dict[str, str]:
        """
        :return: the suffixes for io signals.
        :rtype: Dict[str,str]

        This should be overriden if the default does not match the rtl code.
        The fileds that are required are `"i"`, `"o"` and `"oe"`
        """
        return {"i": "i", "o": "o", "oe": "en_o"}


    def _make_io_connections(self, rh: RoutingHelper) -> str:
        """
        Internal methode to make io connections,
        Override instead, `get_io_connections, `get_io_prefix` and `get_io_suffix`.
        """
        inst: str = ""
        for sig in self.get_io_connections():
            name = sig["name"]
            t = sig["type"]
            subs = []
            if t == "inout" or t == "input":
                subs.append("i")
            if t == "inout" or t == "output" or t == "output_no_en":
                subs.append("o")
            if t == "inout" or t == "output":
                subs.append("oe")
            for sub in subs:
                snames = [
                    rh.use_source_as_sv(f"{self._io_sig_name(sig)}{i}_{sub}", self._p_node)
                    for i in range(sig["width"])
                ]
                snames = ','.join(snames[::-1])
                if sig["width"] > 1:
                    snames = f"{{{snames}}}"
                inst += f".{self.get_io_prefix()}{name}_{self.get_io_suffix()[sub]}({snames}),"
        
        return inst

    
    def get_addr_bits(self) -> int:
        """
        :return: the number of address bits used by this peripehral
        :rtype: int
        """
        if self._ip_block is None:
            raise RuntimeError("No ip block configured")
        
        length = 0
        found = False
        for _, b in self._ip_block.reg_blocks.items():
            length = b.get_addr_width()
            if found:
                raise RuntimeError("IP has mor than one register block")
            found = True
        
        return length
    
    def get_min_length(self) -> int:
        """
        :return: The minimum address space needed by this peripheral.
        :rtype: int
        """
        return 2**max(self.get_addr_bits(), 2)
    
    def addr_setup(self):
        """
        Sets up the address of the peripheral
        """
        if self._addr_size is None:
            self._addr_size = self.get_min_length()
        
        elif self._addr_size < self.get_min_length():
            raise RuntimeError(f"A length of 0x{self._addr_size} was request for peripheral {self._name} {self._sp_name_suffix} but a length of at least {self.get_min_length()} is required.")
        
    def get_offset(self) -> Optional[int]:
        """
        :return: The offset relative to the domain
        :rtype: Optional[int]
        """
        return self._offset
    
    def get_offset_checked(self) -> int:
        """
        :return: The offset relative to the domain, if already set.
        :rtype: Optional[int]
        :raise RuntimeError: if the offset is not set yet.
        """
        if type(self._offset) is not int:
            raise RuntimeError("checked failed offset is not set properly")
        return self._offset
    
    def get_address_length(self) -> Optional[int]:
        """
        :return: the address length
        :rtype: Optional[int]
        """
        return self._addr_size
    
    def get_address_length_checked(self) -> int:
        """
        :return: The address length, if already set.
        :rtype: Optional[int]
        :raise RuntimeError: if the address length is not set yet.
        """
        if type(self._addr_size) is not int:
            raise RuntimeError("checked failed address size is not set properly")
        return self._addr_size

    def set_offset(self, offset: int):
        """
        Set the offset of this peripheral.

        It should be aligned to the size.
        The rtl can not know how this is aligned either.

        :param int offset: the offset
        """
        self._offset = offset

    def get_ip_path(self) -> Optional[str]:
        """
        Get the path of the file descibing this peripheral.

        :return: The path, if present
        :rtype: Optional[str]
        """
        return self._ip_block_path