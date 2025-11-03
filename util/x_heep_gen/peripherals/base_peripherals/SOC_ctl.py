from ..abstractions import BasePeripheral, DataConfiguration, PeripheralDomain

class SOC_ctrl(BasePeripheral, DataConfiguration):
    """
    System-on-Chip control peripheral for managing system-level functions and configuration.

    Default configuration file: ./hw/ip/soc_ctrl/data/soc_ctrl.hjson
    """

    _name = "soc_ctrl"
    _config_path = "./hw/ip/soc_ctrl/data/soc_ctrl.hjson"
