from typing import Iterable
from enum import Enum
from .ram_bank import Bank, is_pow2, ILRamGroup
from .linker_section import LinkerSection

class BusType(Enum):
    """Enumeration of all supported bus types"""

    onetoM = 'onetoM'
    NtoM   = 'NtoM'



class XHeep():
    """
    This object represents the whole mcu.

    An instance of this object is also passed to the mako templates.

    :param BusType bus_type: The bus type chosen for this mcu.
    :param int ram_start_address: The address of the first ram bank. For now only 0 is tested. Defaults to 0.
    :raise TypeError: when parameters are of incorrect type.
    """

    IL_COMPATIBLE_BUS_TYPES: "set[BusType]" = set([BusType.NtoM])
    """Constant set of bus types that support interleaved memory banks"""
    
    
    
    def __init__(self, bus_type: BusType, ram_start_address: int = 0):
        if not type(bus_type) is BusType:
            raise TypeError(f"XHeep.bus_type should be of type BusType not {type(self._bus_type)}")
        if not type(ram_start_address) is int:
            raise TypeError("ram_start_address should be of type int")

        if ram_start_address != 0:
            ValueError(f"ram start address must be 0 instead of {ram_start_address}")

        self._bus_type: BusType = bus_type
        self._ram_start_address: int = ram_start_address
        self._ram_banks: list[Bank] = []
        self._ram_banks_il_idx: list[int] = []
        self._ram_banks_il_groups: list[ILRamGroup] = []
        self._il_banks_present: bool = False
        self._ram_next_idx: int = 1
        self._ram_next_addr: int = self._ram_start_address
        self._linker_sections: list[LinkerSection] = []
        self._used_section_names: set[str] = set()


    def add_ram_banks(self, bank_sizes: "list[int]", section_name: str):
        """
        Add ram banks in continous address mode to the system.
        The bank size should be a power of two and at least 1kiB.

        :param list[int] bank_sizes: list of bank sizes in kiB that should be added to the system
        :param str section_name: name of the section for the linker script the two first sections should be code and then data, the names must be unique and not be used by the linker for other purposes.
        :raise TypeError: when arguments are of wrong type
        :raise ValueError: when banks have an incorrect size.
        :raise ValueError: if the name was allready used for another section or the first and second are not code and data.
        :raise ValueError: if bank_sizes list is empty
        """
        if not type(bank_sizes) == list:
            raise TypeError("bank_sizes should be of type list")
        if not type(section_name) == str:
            raise TypeError("section_name should be of type str")
        if len(bank_sizes) == 0:
            raise ValueError("bank_sizes is empty")

        banks: list[Bank] = []
        for b in bank_sizes:
            banks.append(Bank(b, self._ram_next_addr, self._ram_next_idx, 0, 0))
            self._ram_next_addr = banks[-1]._end_address
            self._ram_next_idx += 1
        
        self._add_linker_section(banks, section_name)
        # Add all new banks if no error was raised
        self._ram_banks += banks

    


    def add_ram_banks_il(self, num: int, bank_size: int, section_name: str):
        """
        Add ram banks in interleaved mode to the system.
        The bank size should be a power of two and at least 1kiB,
        the number of banks should also be a power of two.

        :param int num: number of banks to add
        :param int bank_size: size of the banks in kiB
        :param str section_name: name of the section for the linker script the two first sections should be code and then data, the names must be unique and not be used by the linker for other purposes.
        :raise TypeError: when arguments are of wrong type
        :raise ValueError: when banks have an incorrect size or their number is not a power of two.
        :raise ValueError: if the name was allready used for another section or the first and second are not code and data.
        """
        if not self._bus_type in self.IL_COMPATIBLE_BUS_TYPES:
            raise RuntimeError(f"This system has a {self._bus_type} bus, one of {self.IL_COMPATIBLE_BUS_TYPES} is required for interleaved memory")
        if not type(num) == int:
            raise TypeError("num should be of type int")
        if not is_pow2(num):
            raise ValueError(f"A power of two is required for the number of banks, got {num}")
        if not type(section_name) == str:
            raise TypeError("section_name should be of type str")

        first_il = self.ram_numbanks()

        banks: list[Bank] = []
        for i in range(num):
            banks.append(Bank(bank_size, self._ram_next_addr, self._ram_next_idx, num.bit_length()-1, i))
            self._ram_next_idx += 1
        
        self._ram_next_addr = banks[-1]._end_address
        
        self._add_linker_section(banks, section_name)
        # Add all new banks if no error was raised
        self._ram_banks += banks

        indices = range(first_il, first_il + num)
        self._ram_banks_il_idx += indices
        self._ram_banks_il_groups.append(ILRamGroup(banks[0].start_address(), bank_size*num*1024, len(banks), banks[0].name()))
        self._il_banks_present = True
    


    def _add_linker_section(self, banks: "list[Bank]", name: str):
        """
        Internal function to add linker sections
        :param list[Bank] banks: list of banks that compose the section, assumed to be continous in memory
        :param str name: the name of the section.
        :raise ValueError: if the name was allready used for another section or the first and second are not code and data.
        """
        if name in self._used_section_names:
            raise ValueError("linker section names should be unique")
        
        if len(self._linker_sections) == 0 and name != "code":
            raise ValueError("The first linker section sould be called code.")
        
        if len(self._linker_sections) == 1 and name != "data":
            raise ValueError("The first linker section sould be called data.")
        
        self._used_section_names.add(name)
        self._linker_sections.append(LinkerSection(name, banks[0].start_address(), banks[-1].end_address()))
        


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
        return len(self._ram_banks)
    


    def ram_numbanks_il(self) -> int:
        return len(self._ram_banks_il_idx)
    


    def ram_numbanks_cont(self) -> int:
        return self.ram_numbanks() - self.ram_numbanks_il()
    


    def validate(self) -> bool:
        if not self.ram_numbanks() in range(2, 17):
            print(f"The number of banks should be between 2 and 16 instead of {self.ram_numbanks()}") #TODO: clarify upper limit
            return False
        
        if not ("code" in self._used_section_names and "data" in self._used_section_names):
            print("The code and data sections are needed")
            return False

        #TODO: clarify why
        if self._linker_sections[0].size < 32*1024:
            print("The code section must be at least 32KB, instead it is " + self._linker_sections[0].size)
            return False
        return True
    


    def ram_size_address(self) -> int:
        size = 0
        for bank in self._ram_banks:
            size += bank.size()
        return size
    


    def ram_il_size(self) -> int:
        size = 0
        for i in self._ram_banks_il_idx:
            size += self._ram_banks[i].size()
        return size
    


    def iter_ram_banks(self) -> Iterable[Bank]:
        return iter(self._ram_banks)



    def iter_cont_ram_banks(self) -> Iterable[Bank]:
        m = map((lambda b: None if b[0] in self._ram_banks_il_idx else b[1]), enumerate(self._ram_banks))
        return filter(lambda b: not b == None, m)
    


    def iter_il_ram_banks(self) -> Iterable[Bank]:
        m = map((lambda b: None if not b[0] in self._ram_banks_il_idx else b[1]), enumerate(self._ram_banks))
        return filter(lambda b: not b == None, m)


    def has_il_ram(self) -> bool:
        return self._il_banks_present
    


    def il_ram_start_address(self) -> "int|None":
        if not self.has_il_ram():
            return None
        
        return self._ram_banks[self._ram_banks_il_idx[0]].start_address()
    
    def il_ram_end_address(self) -> "int|None":
        if not self.has_il_ram():
            return None
        
        return self._ram_banks[self._ram_banks_il_idx[-1]].end_address()
    
    def il_ram_size(self) -> "int|None":
        if not self.has_il_ram():
            return None
        
        return self.il_ram_end_address() - self.il_ram_start_address()
    
    def iter_il_groups(self) -> Iterable[ILRamGroup]:
        return iter(self._ram_banks_il_groups)
    
    def iter_linker_sections(self) -> Iterable[LinkerSection]:
        return iter(self._linker_sections)