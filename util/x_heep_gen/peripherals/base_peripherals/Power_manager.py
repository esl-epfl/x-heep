from ..abstractions import BasePeripheral

class Power_manager(BasePeripheral):
    """
    Manages power states and clock gating for different system components.

    Default configuration file: ./hw/ip/power_manager/data/power_manager.hjson
    """

    _name = "power_manager"
