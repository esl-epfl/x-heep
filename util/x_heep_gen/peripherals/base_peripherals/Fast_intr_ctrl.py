from ..abstractions import BasePeripheral

class Fast_intr_ctrl(BasePeripheral):
    """
    Fast interrupt controller for low-latency interrupt handling.

    Default configuration file: ./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson
    """

    _name = "fast_intr_ctrl"
