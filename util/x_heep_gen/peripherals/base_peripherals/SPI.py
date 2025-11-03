from ..abstractions import BasePeripheral, DataConfiguration, PeripheralDomain

class SPI_flash(BasePeripheral):
    """
    Interface for external SPI flash memory access.

    Default length : 32KB
    """

    _name = "spi_flash"
    _length: int = 0x00008000

class SPI_memio(BasePeripheral):
    """
    Memory-mapped IO interface for SPI communication.

    Default length : 32KB
    """

    _name = "spi_memio"
    _length: int = 0x00008000