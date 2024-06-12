from abc import *
from typing import Dict, List, Tuple
"""
This modules add types to help the generation of sv code.
It does not a lot of checks but helps to move sv generation out of the other code.

Formatting is ignored, for this you should use a tool like verible for the post processing.
"""

class SvHelper(ABC):
    """
    General interface used by other classes in this module.
    """
    @abstractmethod
    def get_def(self) -> str:
        """
        Get the definition in system verilog.

        :return: the sv definition
        :rtype: str
        """
        pass





class SvInterface(SvHelper):
    """
    Represents a sv interface, for code generation.
    
    :param str name: the name of the interface."""
    def __init__(self, name: str) -> None:
        if type(name) is not str:
            raise TypeError("name should be of type str")
        self._name: str = name
        self._ports: Dict[str, List[Tuple[str,str]]] = dict()
        self._signals: List[Tuple[str,str]] = []

    def add_signal(self, t: str, name: str, ports: List[Tuple[str, str]]):
        """
        Add a signal.
        
        :param str t: the type of the signal.
        :param str name: the name of the signal.
        :param  List[Tuple[str, str]] ports: Add the ports of the signal. Each tuple is composed of the port name and in second the direction."""
        if type(t) is not str:
            raise TypeError("t should be of type str")
        if type(name) is not str:
            raise TypeError("name should be of type str")
        if not isinstance(ports, list):
            raise TypeError("ports should be an instance of list")
        
        self._signals.append((t, name))

        for port, direction in ports:
            if not port in self._ports:
                self._ports[port] = []
            self._ports[port].append((direction, name))
    
    def get_def(self) -> str:
        out = f"interface {self._name} ();"

        for t, name in self._signals:
            out += f"{t} {name};"

        for port, l in self._ports.items():
            out += f"modport {port} ("
            
            for direction, name in l:
                out += f"{direction} {name},"
            
            if out[-1] == ",":
                out = out[:-1]

            out += ");"

        out += "endinterface"
        return out
    
            
        
