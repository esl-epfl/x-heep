from ..abstractions import UserPeripheral, DataConfiguration

class UART(UserPeripheral, DataConfiguration):
    """
    Universal Asynchronous Receiver/Transmitter for serial communication.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson
    """

    _name = "uart"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson"
