from .peripheral_helper import peripheral_from_file
from .rv_timer import RvTimerPeripheral
from .spi_host import SpiHostPeripheral
from .i2s import I2SPeripheral
from .gpio import GpioPeripheral
from .rv_plic import RvPlicPeripheral
from .pdm2pcm import Pdm2PcmPeripheral
from .fixed_peripheral import FixedPeripheral

@peripheral_from_file("./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson")
class I2CPeripheral():
    pass


@peripheral_from_file("./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson")
class UartPeripheral():
    pass