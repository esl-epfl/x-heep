from enum import *
from typing import List


class Endpoint:
    """
    A class providing information about sources and targets that can be connected together.
    """
    def __init__(self):
        self.t: str = "logic"
        self.source_direction: str = "output"
        self.target_direction: str = "input"
        self.reserved = False
        self.used = False
        self.nc = True
    
    def single_use(self) -> bool:
        """
        Informs if a target with this endpoint informations can be used only once.

        :return: If it can be used only onde
        :rtype: bool
        """
        return True

    def use_target(self, name: str) -> bool:
        """
        Function used by the routing helper to update the class after each new use.

        :param str name: the name of the source
        :return: if the target is full, can no longer be connected to new sources, the current source is valid.
        :rtype: bool
        """
        self.used = True
        self.nc = False
        return True
    
    def target_nc(self) -> str:
        """
        If an Endpoint can be unconnected on the target side it should implement this method.

        :return: the sv code in case the target is not connected.
        :rtype: str
        """
        raise NotImplementedError("target_nc is not implemented for this endpoint")


class InterruptEP(Endpoint):
    """
    An Endpoint used to connect interrupts to the fast interrupt module.
    """
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
    """
    An endpoint to connect interrupts directly to the rv core
    """
    def __init__(self):
        super().__init__()
        self.t = "logic"
        self.source_direction: str = "output"
        self.target_direction: str = "input"
    
    def target_nc(self) -> str:
        return "1'b0"

class InterruptPlicEP(Endpoint):
    """
    An Endpoint to connect interrupts to the plic
    """
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
    """
    An Endpoint to connect dma triggers
    """
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
