from ..abstractions import BasePeripheral, DataConfiguration

class Power_manager(BasePeripheral, DataConfiguration):
    """
    Manages power states and clock gating for different system components.

    Default configuration file: ./hw/ip/power_manager/data/power_manager.hjson
    """

    _name = "power_manager"
    _config_path = "./hw/ip/power_manager/data/power_manager.hjson"
