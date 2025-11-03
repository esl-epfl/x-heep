from ..abstractions import UserPeripheral, DataConfiguration, PeripheralDomain

class I2C(UserPeripheral, DataConfiguration):
    """
    Inter-Integrated Circuit communication interface.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson
    """

    _name = "i2c"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson"
