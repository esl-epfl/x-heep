from enum import *
from typing import List


class Endpoint:
    def __init__(self):
        self.t: str = "logic"
        self.source_direction: str = "output"
        self.target_direction: str = "input"
        self.reserved = False
        self.used = False
        self.nc = True
    
    def single_use(self) -> bool:
        return True

    def use_target(self, name: str) -> bool:
        self.used = True
        self.nc = False
        return True
    
    def target_nc(self) -> str:
        raise NotImplementedError("target_nc is not implemented for this endpoint")

@unique
class EndpointType(Enum):
    TARGET = auto()
    SOURCE = auto()

class InterruptEP(Endpoint):
    def __init__(self, handler=""):
        super().__init__()
        self.t = "logic"
        self.source_direction: str = "output"
        self.target_direction: str = "input"
        self.count = 0
        self.source_names: List[str] = []
        self.handler = handler

    def single_use(self) -> bool:
        return False

    def use_target(self, name: str) -> bool:
        self.count += 1
        self.nc = False
        self.used = True
        
        if self.count < 15:
            return False
        
        return True

class InterruptDirectEP(Endpoint):
    def __init__(self):
        super().__init__()
        self.t = "logic"
        self.source_direction: str = "output"
        self.target_direction: str = "input"
    
    def target_nc(self) -> str:
        return "1'b0"

class InterruptPlicEP(Endpoint):
    def __init__(self, handler="handler_irq_dummy"):
        super().__init__()
        self.t = "logic"
        self.source_direction: str = "output"
        self.target_direction: str = "input"
        self.count = 0
        self.source_names: List[str] = []
        self.handler = handler

    def single_use(self) -> bool:
        return False

    def use_target(self, name: str) -> bool:
        self.count += 1
        self.nc = False
        self.used = True
        
        if self.count < 63: 
            return False
        
        return True
    
class DmaTriggerEP(Endpoint):
    def __init__(self):
        super().__init__()
        self.t = "logic"
        self.source_direction: str = "output"
        self.target_direction: str = "input"
        self.count = 0
        self.source_names: List[str] = []

    def single_use(self) -> bool:
        return False

    def use_target(self, name: str) -> bool:
        self.count += 1
        self.nc = False
        self.used = True
        
        if self.count < 16:
            return False
        
        return True
