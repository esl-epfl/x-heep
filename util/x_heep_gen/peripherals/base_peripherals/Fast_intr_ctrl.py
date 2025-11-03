from ..abstractions import BasePeripheral, DataConfiguration

class Fast_intr_ctrl(BasePeripheral, DataConfiguration):
    """
    Fast interrupt controller for low-latency interrupt handling.

    Default configuration file: ./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson
    """

    _name = "fast_intr_ctrl"
    _config_path = "./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson"
