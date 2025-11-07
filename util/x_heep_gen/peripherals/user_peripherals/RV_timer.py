from ..abstractions import UserPeripheral

class RV_timer(UserPeripheral):
    """
    RISC-V timer peripheral.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson
    """

    _name = "rv_timer"
