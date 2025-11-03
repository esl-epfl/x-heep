from ..abstractions import UserPeripheral, DataConfiguration, PeripheralDomain

class SPI_host(UserPeripheral, DataConfiguration):
    """
    Serial Peripheral Interface host controller.

    Default configuration file: ./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson
    """

    _name = "spi_host"
    _config_path = "./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson"

class SPI2(UserPeripheral):
    """
    Secondary Serial Peripheral Interface.
    """

    _name = "spi2"