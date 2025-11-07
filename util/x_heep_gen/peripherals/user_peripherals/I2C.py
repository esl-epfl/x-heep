from ..abstractions import UserPeripheral

class I2C(UserPeripheral):
    """
    Inter-Integrated Circuit communication interface.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson
    """

    _name = "i2c"
