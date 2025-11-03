from ..abstractions import UserPeripheral,DataConfiguration

class I2S(UserPeripheral, DataConfiguration):
    """
    Inter-IC Sound interface.

    Default configuration file: ./hw/ip/i2s/data/i2s.hjson
    """

    _name = "i2s"
    _config_path = "./hw/ip/i2s/data/i2s.hjson"
