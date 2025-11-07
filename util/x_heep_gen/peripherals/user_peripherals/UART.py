from ..abstractions import UserPeripheral

class UART(UserPeripheral):
    """
    Universal Asynchronous Receiver/Transmitter for serial communication.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson
    """

    _name = "uart"
