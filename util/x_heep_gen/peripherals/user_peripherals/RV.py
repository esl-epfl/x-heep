from ..abstractions import UserPeripheral, DataConfiguration, PeripheralDomain

class RV_plic(UserPeripheral, DataConfiguration):
    """
    RISC-V Platform Level Interrupt Controller.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson
    """

    _name = "rv_plic"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson"

class RV_timer(UserPeripheral, DataConfiguration):
    """
    RISC-V timer peripheral.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson
    """

    _name = "rv_timer"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson"

