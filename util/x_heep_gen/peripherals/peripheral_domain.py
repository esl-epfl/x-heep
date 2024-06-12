from copy import deepcopy
from typing import Dict, Iterable, List, Optional, Tuple

from .sv_helper import SvSignal, SvSignalArray
from .basic_peripheral import BasicPeripheral
from ..signal_routing.node import Node
from ..signal_routing.routing_helper import RoutingHelper

class PeripheralDomain:
    """
    A peripheral domain

    :param str name:
    :param int address: the base address used to address this domain.
    :param int addr_size: the size reserved for this peripheral domain.
    :param bool has_clock_domain: Flag to tell if the domain should have it's own clock domain (currently unused)
    """
    def __init__(self, name: str, address: int, addr_size: int, has_clock_domain: bool = False):
        if type(name) is not str:
            raise TypeError("name should be of type str")
        if name == "":
            raise ValueError("name should not be empty")
        self._name: str = name
        self._peripherals: List[BasicPeripheral] = []
        self._io_ifs: Dict[str, SvSignalArray] = {}
        self._io_if_offsets: Dict[str, int] = {}
        self._node: Optional[Node] = None

        self._address: int = address
        self._addr_size: int = addr_size
        self._type: str = "normal"
        self._has_clock_domain: bool = has_clock_domain

    def get_type(self) -> str:
        """
        get the type of the domain

        :return: the kind of domain either `"normal"` or `"fixed"`
        :rtype: str
        """
        return self._type

    def add_peripheral(self, p: BasicPeripheral):
        """
        Add a peripheral to this domain.
        
        The peripheral is copied, further changes to this peripheral will not be taken in account.
        The smae peripheral can also be added multiple times.

        :param BasicPeripheral p: The peripheral to add
        """
        if not isinstance(p, BasicPeripheral):
            raise TypeError("peripheral should be an instance of BasicPeripheral")

        if p.name is None or len(p.name) == 0:
            raise RuntimeError("The peripheral should get a name before it is added to a PeripheralDomain object")

        new_p = deepcopy(p)
        new_p.domain = self._name
        self._peripherals.append(new_p)

    def specialize_names(self, used_names: "Dict[str, int]"):
        """
        This method sets suffixes for the peripheral name.

        :param Dict[str, int] used_names: the dictionary with the last suffix for each peripheral base name.
        """
        for p in self._peripherals:
            n = 0
            base_name = p.name
            if base_name in used_names:
                n = used_names.get(base_name)
            
            n = p.specialize_name(n)
            
            used_names.update({base_name: n})    


    def get_name(self) -> str:
        """
        :return: the name of the domain
        :rtype: str
        """
        return self._name
    
    
    def iter_peripherals(self) -> Iterable[BasicPeripheral]:
        """
        iterate through all peripherals

        :return: an iterator of peripherals
        :rtype: Iterable[BasicPeripheral]
        """
        return iter(self._peripherals)
    
    def register_connections(self, rh: RoutingHelper, p_node: Node):
        """
        Register all connections for this domain and peripherals.

        :param RoutingHelper rh: the routing helper
        :param Node p_node: the parent node.
        """
        node = Node("pd_" + self.get_name(), p_node.name)
        rh.register_node(node)
        self._node = node

        for p in self._peripherals:
            p.register_connections(rh, node)

    def build(self):
        """
        Does build operations for this peripheral domain.
        """
        pass

    def peripheral_count(self) -> int:
        """
        :return: the number of peripherals
        :rtype: int
        """
        return len(self._peripherals)
    
    def get_node(self) -> Node:
        """
        :return: The domains node.
        :rtype: Node
        """
        if self._node is None:
            raise RuntimeError("Node is not registered yet")
        
        return deepcopy(self._node)
    
    def get_address(self) -> int:
        """
        :return: the base address of this domain
        :rtype: int
        """
        return self._address
    
    def get_address_length(self) -> int:
        """
        :return: the amount of address reserved for this domain
        :rtype: int
        """
        return self._addr_size
    
    def addr_setup(self):
        """
        Does the callculation and for all addresses of the peripherals.
        """
        reserved: List[Tuple[int, int, int]] = []
        missing: List[Tuple[int, int]] = []
        for i, p in enumerate(self._peripherals):
            p.addr_setup()
            ab = p.get_addr_bits()
            offset = p.get_offset()
            size = p.get_address_length()

            if offset is not None:
                if ((self._address + offset) & (2**ab - 1)) != 0:
                    raise RuntimeError("peripheral offset is not aligned")
                reserved.append((offset, offset + size, i))
            else:
                missing.append((size, i))
        
        reserved.sort()
        missing.sort(key=lambda x: x[0], reverse=True) # we sort the peripherals that do not have an offset

        for (_, end, i), (start, _, j) in zip(reserved, reserved[1:]):
            if start < end:
                raise RuntimeError(f"peripheral {self._peripherals[i].name} and {self._peripherals[j].name} have overlapping memory range")
        
        
        for _, p_idx in missing:
            p = self._peripherals[p_idx]
            ab = p.get_addr_bits()
            size = p.get_address_length()
            for (_, end, _), (start, _, j) in zip(reserved, reserved[1:]):
                if ((end + self._address) & (2**ab - 1)) != 0:
                    end = ((end + self._address) | 2**ab - 1) + 1 - self._address # align potential start address
                
                if end + size <= start:
                    reserved.insert(j, (end, end + size, p_idx))
                    break
            else:
                end = 0
                if len(reserved) > 0:
                    end = reserved[-1][1]
                if ((end + self._address) & (2**ab - 1)) != 0:
                    end = ((end + self._address) | 2**ab - 1) + 1 - self._address # align potential start address
                reserved.append((end, end + size, p_idx))
        
        for offset, _, p_idx in reserved:
            p = self._peripherals[p_idx]
            p.set_offset(offset)
    
    def has_clock_domain(self) -> bool:
        """
        :retrurn: if this peripheral domain has a clock domain
        :rtype: bool
        """
        return self._has_clock_domain

class FixedDomain(PeripheralDomain):
    """
    A peripheral domain type for the `ao_peripheral` domain

    Should not be used for something else.
    """
    def __init__(self, name: str, address: int, addr_size: int):
        super().__init__(name, address, addr_size)
        self._type = "fixed"
    
    def specialize_names(self, used_names: Dict[str, int]):
        pass
    def addr_setup(self):
        pass