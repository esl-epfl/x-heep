from ..abstractions import BasePeripheral, DataConfiguration, PeripheralDomain

class Bootrom(BasePeripheral):
    """
    Read-only memory containing the boot code executed at system startup.
    """

    _name = "bootrom"