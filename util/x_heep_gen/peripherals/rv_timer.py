from .peripheral_helper import peripheral_from_file
from .tlul_peripheral import TLULPeripheral

from reggen.ip_block import IpBlock

@peripheral_from_file("./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson")
class RvTimerPeripheral():

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.interrupt_dest = "fast"

    def specialize_name(self, n: int) -> int:
        """
        Adds a name suffix to the name based on the integer provided.

        It has to be called again when name is changed.

        :param int n: number to be suffixed
        :return: next value for this name, here n+2, but a subclass could do an other increment
        :rtype: int
        :raise TypeError: if argument does not have the right type.
        """
        if type(n) is not int:
            raise TypeError("argument n should be of type int")
        
        self.set_specialized_name(f"{n}_{n+1}")
        self.interrupt_handler_base = {
            "timer_expired_0_0" : f"{self.name}_{n}",
            "timer_expired_1_0" : f"{self.name}_{n + 1}",
        }
        return n + 2