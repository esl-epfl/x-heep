from copy import deepcopy
from dataclasses import dataclass
from typing import Dict, Generator, Iterable, List, Optional, Set, Union
from enum import Enum


from .pads import IoInputEP, IoOutputEP, IoOutputEnEP, PadManager, Pad
from .ram_bank import Bank, is_pow2, ILRamGroup
from .linker_section import LinkerSection
from .peripherals.peripheral_domain import PeripheralDomain
from .peripherals.sv_helper import SvSignalArray
from .signal_routing.endpoints import DmaTriggerEP, InterruptDirectEP, InterruptEP, InterruptPlicEP
from .signal_routing.node import Node
from .signal_routing.routing_helper import RoutingHelper

class BusType(Enum):
    """Enumeration of all supported bus types"""

    onetoM = 'onetoM'
    NtoM   = 'NtoM'


@dataclass
class Override():
    """
    Bundles information that can be overriden in the XHeep class.
    """
    bus_type: Optional[BusType]
    numbanks: Optional[int]
    numbanks_il: Optional[int]



class XHeep():
    """
    This object represents the whole mcu.

    An instance of this object is also passed to the mako templates.

    :param BusType bus_type: The bus type chosen for this mcu.
    :param int ram_start_address: The address of the first ram bank. For now only 0 is tested. Defaults to 0.
    :param Optional[Override] override: configs to be overriden
    :raise TypeError: when parameters are of incorrect type.
    """

    IL_COMPATIBLE_BUS_TYPES: "Set[BusType]" = set([BusType.NtoM])
    """Constant set of bus types that support interleaved memory banks"""
    
    
    
    def __init__(self, bus_type: BusType, ram_start_address: int = 0, override: Optional[Override] = None):
        if not type(bus_type) is BusType:
            raise TypeError(f"XHeep.bus_type should be of type BusType not {type(self._bus_type)}")
        if not type(ram_start_address) is int:
            raise TypeError("ram_start_address should be of type int")

        if ram_start_address != 0:
            raise ValueError(f"ram start address must be 0 instead of {ram_start_address}")

        self._bus_type: BusType = bus_type
        if override is not None and override.bus_type is not None:
            self._bus_type = override.bus_type


        self._ram_start_address: int = ram_start_address
        self._ram_banks: List[Bank] = []
        self._ram_banks_il_idx: List[int] = []
        self._ram_banks_il_groups: List[ILRamGroup] = []
        self._il_banks_present: bool = False
        self._ram_next_idx: int = 1
        self._ram_next_addr: int = self._ram_start_address
        self._linker_sections: List[LinkerSection] = []
        self._used_section_names: Set[str] = set()

        self._ignore_ram_continous: bool = False
        self._ignore_ram_interleaved: bool = False
        
        self._peripheral_domains: List[PeripheralDomain] = []
        self._domain_names: Set[str] = set()

        self._pad_manager: Optional[PadManager] = None

        self._ext_intr_num: int = 0

        self._init_fixed_nodes()


        if override is not None and override.numbanks is not None:
            self.add_ram_banks([32]*override.numbanks)
            self._ignore_ram_continous = True
        if override is not None and override.numbanks_il is not None:
            self._ignore_ram_interleaved = True
            self._override_numbanks_il = override.numbanks_il


    def _init_fixed_nodes(self):
        self._routing_helper: RoutingHelper = RoutingHelper()
        
        self._root_node = self._routing_helper.get_root_node()
        self._routing_helper.add_source(self._root_node, "rst_n", IoInputEP(), Pad.name_to_target_name("rst", 0)+"_i")
        self._routing_helper.add_source(self._root_node, "clk", IoInputEP(), Pad.name_to_target_name("clk", 0)+"_i")
        
        self._routing_helper.add_source(self._root_node, "dma_ext_rx", DmaTriggerEP())
        self._routing_helper.add_source(self._root_node, "dma_ext_tx", DmaTriggerEP())

        self._mcu_node = Node("core_v_mini_mcu", self._root_node.name)
        self._routing_helper.register_node(self._mcu_node)
        self._routing_helper.add_target(self._mcu_node, "fast_irq_target", InterruptEP())
        self._routing_helper.add_target(self._mcu_node, "irq_software", InterruptDirectEP())
        self._routing_helper.add_target(self._mcu_node, "irq_external", InterruptDirectEP())
        self._routing_helper.add_target(self._mcu_node, "rv_timer_0_direct_irq", InterruptDirectEP())
        
        self._routing_helper.add_source(self._mcu_node, "jtag_tck", IoInputEP(), Pad.name_to_target_name("jtag_tck", 0)+"_i")
        self._routing_helper.add_source(self._mcu_node, "jtag_tms", IoInputEP(), Pad.name_to_target_name("jtag_tms", 0)+"_i")
        self._routing_helper.add_source(self._mcu_node, "jtag_trst_n", IoInputEP(), Pad.name_to_target_name("jtag_trst", 0)+"_i")
        self._routing_helper.add_source(self._mcu_node, "jtag_tdi", IoInputEP(), Pad.name_to_target_name("jtag_tdi", 0)+"_i")
        self._routing_helper.add_source(self._mcu_node, "jtag_tdo", IoOutputEP(), Pad.name_to_target_name("jtag_tdo", 0)+"_o")

        self._ao_periph_node = Node("ao_periph", self._mcu_node.name)
        self._routing_helper.register_node(self._ao_periph_node)
        self._routing_helper.add_source(self._ao_periph_node, "boot_select", IoInputEP(), Pad.name_to_target_name("boot_select", 0)+"_i")
        self._routing_helper.add_source(self._ao_periph_node, "execute_from_flash", IoInputEP(), Pad.name_to_target_name("execute_from_flash", 0)+"_i")
        self._routing_helper.add_source(self._ao_periph_node, "exit_valid", IoOutputEP(), Pad.name_to_target_name("exit_valid", 0)+"_o")

        self._routing_helper.add_source(self._ao_periph_node, "spi_flash_sck_o", IoOutputEP(), Pad.name_to_target_name("spi_flash_sck", 0)+"_o")
        self._routing_helper.add_source(self._ao_periph_node, "spi_flash_sck_en_o", IoOutputEnEP(), Pad.name_to_target_name("spi_flash_sck", 0)+"_oe")
        for i in range(2):
            self._routing_helper.add_source(self._ao_periph_node, f"spi_flash_csb_{i}_o", IoOutputEP(), Pad.name_to_target_name("spi_flash_csb", i)+"_o")
            self._routing_helper.add_source(self._ao_periph_node, f"spi_flash_csb_{i}_en_o", IoOutputEnEP(), Pad.name_to_target_name("spi_flash_csb", i)+"_oe")
        for i in range(4):
            self._routing_helper.add_source(self._ao_periph_node, f"spi_flash_sd_{i}_o", IoOutputEP(), Pad.name_to_target_name("spi_flash_sd", i)+"_o")
            self._routing_helper.add_source(self._ao_periph_node, f"spi_flash_sd_{i}_en_o", IoOutputEnEP(), Pad.name_to_target_name("spi_flash_sd", i)+"_oe")
            self._routing_helper.add_source(self._ao_periph_node, f"spi_flash_sd_{i}_i", IoInputEP(), Pad.name_to_target_name("spi_flash_sd", i)+"_i")
    
        self._routing_helper.add_target(self._ao_periph_node, "dma_default_target", DmaTriggerEP())
        self._routing_helper.add_source(self._ao_periph_node, "spi_flash_dma_rx", DmaTriggerEP())
        self._routing_helper.add_source(self._ao_periph_node, "spi_flash_dma_tx", DmaTriggerEP())
        self._routing_helper.add_source(self._ao_periph_node, "spi_flash_intr", InterruptEP(handler="fic_irq_spi_flash"))

        self._routing_helper.add_source(self._ao_periph_node, "rv_timer_0_intr", InterruptDirectEP(), "rv_timer_0_direct_irq")
        self._routing_helper.add_source(self._ao_periph_node, "rv_timer_1_intr", InterruptEP(handler="fic_irq_rv_timer_1"))

        self._routing_helper.add_source(self._ao_periph_node, "dma_done_intr", InterruptEP(handler="fic_irq_dma"))
        self._routing_helper.add_source(self._ao_periph_node, "dma_window_intr", InterruptPlicEP(handler="handler_irq_dma"))

    def add_ram_banks(self, bank_sizes: "List[int]", section_name: str = ""):
        """
        Add ram banks in continuous address mode to the system.
        The bank size should be a power of two and at least 1kiB.

        :param List[int] bank_sizes: list of bank sizes in kiB that should be added to the system
        :param str section_name: If not empty adds automatically a linker section for this banks. The names must be unique and not be used by the linker for other purposes.
        :raise TypeError: when arguments are of wrong type
        :raise ValueError: when banks have an incorrect size.
        :raise ValueError: if the name was allready used for another section or the first and second are not code and data.
        :raise ValueError: if bank_sizes list is empty
        """

        if self._ignore_ram_continous:
            return

        if not type(bank_sizes) == list:
            raise TypeError("bank_sizes should be of type list")
        if not type(section_name) == str:
            raise TypeError("section_name should be of type str")
        if len(bank_sizes) == 0:
            raise ValueError("bank_sizes is empty")

        banks: List[Bank] = []
        for b in bank_sizes:
            banks.append(Bank(b, self._ram_next_addr, self._ram_next_idx, 0, 0))
            self._ram_next_addr = banks[-1]._end_address
            self._ram_next_idx += 1
        
        if section_name != "":
            self.add_linker_section_for_banks(banks, section_name)
        # Add all new banks if no error was raised
        self._ram_banks += banks

    


    def add_ram_banks_il(self, num: int, bank_size: int, section_name: str = "", ignore_ignore: bool = False):
        """
        Add ram banks in interleaved mode to the system.
        The bank size should be a power of two and at least 1kiB,
        the number of banks should also be a power of two.

        :param int num: number of banks to add
        :param int bank_size: size of the banks in kiB
        :param str section_name: If not empty adds automatically a linker section for this banks. The names must be unique and not be used by the linker for other purposes.
        :param bool ignore_ignore: Ignores the fact that an override was set. For internal uses to apply this override.
        :raise TypeError: when arguments are of wrong type
        :raise ValueError: when banks have an incorrect size or their number is not a power of two.
        :raise ValueError: if the name was allready used for another section or the first and second are not code and data.
        """
        if self._ignore_ram_interleaved and not ignore_ignore:
            return

        if not self._bus_type in self.IL_COMPATIBLE_BUS_TYPES:
            raise RuntimeError(f"This system has a {self._bus_type} bus, one of {self.IL_COMPATIBLE_BUS_TYPES} is required for interleaved memory")
        if not type(num) == int:
            raise TypeError("num should be of type int")
        if not is_pow2(num):
            raise ValueError(f"A power of two is required for the number of banks, got {num}")
        if not type(section_name) == str:
            raise TypeError("section_name should be of type str")

        first_il = self.ram_numbanks()

        banks: List[Bank] = []
        for i in range(num):
            banks.append(Bank(bank_size, self._ram_next_addr, self._ram_next_idx, num.bit_length()-1, i))
            self._ram_next_idx += 1
        
        self._ram_next_addr = banks[-1]._end_address
        
        if section_name != "":
            self.add_linker_section_for_banks(banks, section_name)
        # Add all new banks if no error was raised
        self._ram_banks += banks

        indices = range(first_il, first_il + num)
        self._ram_banks_il_idx += indices
        self._ram_banks_il_groups.append(ILRamGroup(banks[0].start_address(), bank_size*num*1024, len(banks), banks[0].name()))
        self._il_banks_present = True


        self._io_ifs: Dict[str, SvSignalArray] = {}

    


    def add_linker_section_for_banks(self, banks: "List[Bank]", name: str):
        """
        Function to add linker sections coupled to some banks.
        :param List[Bank] banks: list of banks that compose the section, assumed to be continuous in memory
        :param str name: the name of the section.
        :raise ValueError: if the name was allready used for another section or the first and second are not code and data.
        """
        if name in self._used_section_names:
            raise ValueError("linker section names should be unique")
        
        self._used_section_names.add(name)
        self._linker_sections.append(LinkerSection(name, banks[0].start_address(), banks[-1].end_address()))
    
    def add_linker_section(self, section: LinkerSection):
        """
        Function to add a linker section.
        :param LinkerSection section: Linker section to add.
        :param str name: the name of the section.
        :raise ValueError: if the name was allready used for another section or the first and second are not code and data.
        """

        if not isinstance(section, LinkerSection):
            raise TypeError("section should be an instance of LinkerSection")
        
        section.check()

        if section.name in self._used_section_names:
            raise ValueError("linker section names should be unique")

        self._used_section_names.add(section.name)
        self._linker_sections.append(deepcopy(section))
        


    def bus_type(self) -> BusType:
        """
        :return: the configured bus type
        :rtype: BusType
        """
        return self._bus_type
    
    def ram_start_address(self) -> int:
        """
        :return: the address of the first ram bank.
        :rtype: int
        """
        return self._ram_start_address

    def ram_numbanks(self) -> int:
        """
        :return: the number of banks.
        :rtype: int
        """
        return len(self._ram_banks)
    


    def ram_numbanks_il(self) -> int:
        """
        :return: the number of interleaved banks.
        :rtype: int
        """
        return len(self._ram_banks_il_idx)
    


    def ram_numbanks_cont(self) -> int:
        """
        :return: the number of continuous banks.
        :rtype: int
        """
        return self.ram_numbanks() - self.ram_numbanks_il()
    


    def validate(self) -> bool:
        """
        Does some basics checks on the configuration

        This should be called before using the XHeep object to generate the project.

        :return: the validity of the configuration
        :rtype: bool
        """
        if not self.ram_numbanks() in range(2, 17):
            print(f"The number of banks should be between 2 and 16 instead of {self.ram_numbanks()}") #TODO: clarify upper limit
            return False
        
        if not ("code" in self._used_section_names and "data" in self._used_section_names):
            print("The code and data sections are needed")
            return False
        
        for l in self._linker_sections:
            l.check()

        ret = True
        old_sec: Union[LinkerSection,None] = None

        for i, sec in enumerate(self._linker_sections):
            if i == 0 and sec.name != "code":
                print("The first linker section sould be called code.")
                ret = False
            elif i == 1 and sec.name != "data":
                print("The second linker section sould be called data.")
                ret = False

            if old_sec is not None:
                if sec.start < old_sec.end:
                    print(f"Section {sec.name} and {old_sec.name} overlap.")

            start = sec.start
            found_start = False
            found_end = False
            for b in self._ram_banks:
                if found_start:
                    if b.start_address() > start:
                        print(f"Section {sec.name} has a memory hole starting at {start:#08X}")
                        ret = False
                        found_end = True
                        break
                    else:
                        start = b.end_address()

                if sec.start >= b.start_address() and sec.start < b.end_address():
                    found_start = True
                    start = b.end_address()
                
                if sec.end <= b.end_address() and sec.end > b.start_address():
                    found_end = True
                    break

            if not found_start:
                print(f"Section {sec.name} does not start in any ram bank.")
                ret = False

            if not found_end:
                ret = False
                print(f"Section {sec.name} does not end in any ram bank.")

            old_sec = sec
        
        return ret
    


    def ram_size_address(self) -> int:
        """
        :return: the size of the addressable ram memory.
        :rtype: int
        """
        size = 0
        for bank in self._ram_banks:
            size += bank.size()
        return size
    


    def ram_il_size(self) -> int:
        """
        :return: the memory size of the interleaved sizes.
        :rtype: int
        """
        size = 0
        for i in self._ram_banks_il_idx:
            size += self._ram_banks[i].size()
        return size
    


    def iter_ram_banks(self) -> Iterable[Bank]:
        """
        :return: an iterator over all banks.
        :rtype: Iterable[Bank]
        """
        return iter(self._ram_banks)



    def iter_cont_ram_banks(self) -> Iterable[Bank]:
        """
        :return: an iterator over all continuous banks.
        :rtype: Iterable[Bank]
        """
        m = map((lambda b: None if b[0] in self._ram_banks_il_idx else b[1]), enumerate(self._ram_banks))
        return filter(None, m)
    


    def iter_il_ram_banks(self) -> Iterable[Bank]:
        """
        :return: an iterator over all interleaved banks.
        :rtype: Iterable[Bank]
        """
        m = map((lambda b: None if not b[0] in self._ram_banks_il_idx else b[1]), enumerate(self._ram_banks))
        return filter(None, m)



    def has_il_ram(self) -> bool:
        """
        :return: `True` if the system has interleaved ram.
        :rtype: bool
        """
        return self._il_banks_present
    
    

    def iter_il_groups(self) -> Iterable[ILRamGroup]:
        """
        :return: an iterator over the interleaved ram bank groups.
        :rtype: Iterable[ILRamGroup]
        """
        return iter(self._ram_banks_il_groups)
    


    def iter_linker_sections(self) -> Iterable[LinkerSection]:
        """
        :return: an iterator over the linker sections
        :rtype: Iterable[LinkerSection]
        """
        return iter(self._linker_sections)
    


    def iter_bank_numwords(self) -> Generator[int, None, None]:
        """
        Iterates over the size of the ram banks in number of words.

        :return: Generator over the sizes
        :rtype: Generator[int, None, None]
        """
        sizes = set()
        for b in self._ram_banks:
            if b.size() not in sizes:
                sizes.add(b.size())
                yield b.size() // 4
    
    def build(self):
        """
        Makes the system ready to be used.

        - Aplies the overrides for the interleaved memory as the normal memory needs to be configured first.
        - Sorts the linker sections by starting address.
        - Inferes the missing linker section ends with the start of the next section if present. If not it uses the end of the last memory bank.
        """
        if self._ignore_ram_interleaved:
            sec_name = ""
            if self.ram_numbanks() > 1:
                sec_name = "data_interleaved"
            self.add_ram_banks_il(self._override_numbanks_il, 32, sec_name, ignore_ignore=True) #Add automatically a section for compatibility purposes.


        self._linker_sections.sort(key=lambda l: l.start)

        old_sec: Optional[LinkerSection] = None
        for sec in self._linker_sections:
            if old_sec is not None:
                old_sec.end = sec.start
            
            if sec.end is None:
                old_sec = sec
            else:
                old_sec = None
        if old_sec is not None:
            if len(self._ram_banks) == 0:
                raise RuntimeError("There is no ram bank to infere the end of a section")
            old_sec.end = self._ram_banks[-1].end_address()
    
        self._periph_build()

        for i in range(self._ext_intr_num):
            self._routing_helper.add_source(self._root_node, f"ext_intr_{i}", InterruptPlicEP())

        if self._pad_manager is not None:
            self._pad_manager.pre_route(self._routing_helper)

        self._routing_helper.route()

        for pd in self._peripheral_domains:
            pd.addr_setup()


    def add_domain(self, pd: PeripheralDomain):
        """
        Adds a peripheral domain to the system.

        The domain should have a unique name.

        :param PeriperalDomain pd: The peripheral domain to be added.
        :raise TypeError: if the argument is not an instance of PeripheralDomain
        :raise RuntimeError: if the domain name is not unique.
        """
        if not isinstance(pd, PeripheralDomain):
            raise TypeError("argument should be an instance of PeripheralDomain")
        
        name = pd.get_name()
        if name in self._domain_names:
            raise RuntimeError("a domain with the same name was allready added")
        
        self._peripheral_domains.append(deepcopy(pd))

    def iter_peripheral_domains(self) -> Iterable[PeripheralDomain]:
        return iter(self._peripheral_domains)
    
    def num_peripheral_domains(self) -> int:
        return len(self._peripheral_domains)
    
    def iter_peripheral_domains_normal(self) -> Iterable[PeripheralDomain]:
        return filter(lambda d: d.get_type() == "normal", self._peripheral_domains)
    
    def iter_peripheral_domains_fixed(self) -> Iterable[PeripheralDomain]:
        return filter(lambda d: d.get_type() == "fixed", self._peripheral_domains)

    def _periph_build(self):
        used_names = {
            "rv_timer": 2,
        }
        for pd in self._peripheral_domains:
            pd.specialize_names(used_names)
            pd.register_connections(self._routing_helper, self._mcu_node)

        for pd in self._peripheral_domains:
            pd.build()

#            for name, sig in pd.iter_io_interfaces():
#                width = 0
#                if name in self._io_ifs:
#                    width = self._io_ifs[name].width
#                
#                lsig = deepcopy(sig)
#                lsig.width = width + sig.width
#                pd.set_io_if_offset(name, width)
#                
#                self._io_ifs.update({name: lsig})
#
#    
#    def make_io_interfaces(self) -> List[str]:
#        names: Set[str] = set()
#        ifs: List[str] = []
#
#        for domain in self._peripheral_domains:
#            for periph in domain.iter_peripherals():
#                name = periph.make_io_interface_name()
#                if name is not None and name not in names:
#                    names.add(name)
#                    ifs.append(periph.make_io_interface())
#        
#        return ifs

    def get_rh(self) -> RoutingHelper:
        return self._routing_helper
    
    def get_mcu_node(self) -> Node:
        return self._mcu_node
    
    def get_ao_node(self) -> Node:
        return self._ao_periph_node
    
    def add_pad_manager(self, pad_manager: PadManager):
        if not isinstance(pad_manager, PadManager):
            raise TypeError("pad_manager should be an instance of PadManager")
        if self._pad_manager is not None:
            raise RuntimeError("The pad manager is already set")
        
        self._pad_manager = pad_manager

    def get_pad_manager(self) -> PadManager:
        return self._pad_manager
    
    def set_ext_intr(self, num: int):
        self._ext_intr_num = num

    def get_ext_intr(self) -> int:
        return self._ext_intr_num