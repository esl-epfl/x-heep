from abc import *
from typing import Dict, List, Tuple


class SvHelper(ABC):
    @abstractmethod
    def get_def(self) -> str:
        pass





class SvInterface(SvHelper):
    def __init__(self, name: str) -> None:
        if type(name) is not str:
            raise TypeError("name should be of type str")
        self._name: str = name
        self._ports: Dict[str, List[Tuple[str,str]]] = dict()
        self._signals: List[Tuple[str,str]] = []

    def add_signal(self, t: str, name: str, ports: List[Tuple[str, str]]):
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
    
            
        
