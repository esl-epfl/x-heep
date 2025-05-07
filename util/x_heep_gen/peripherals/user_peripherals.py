# User Peripherals
from .abstractions import UserPeripheral, DataConfiguration, PeripheralDomain

# User Peripherals Classes


class RV_plic(UserPeripheral, DataConfiguration):
    """
    RISC-V Platform Level Interrupt Controller.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson
    """

    _name = "rv_plic"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson"


class SPI_host(UserPeripheral, DataConfiguration):
    """
    Serial Peripheral Interface host controller.

    Default configuration file: ./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson
    """

    _name = "spi_host"
    _config_path = "./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson"


class GPIO(UserPeripheral, DataConfiguration):
    """
    General Purpose Input/Output controller.

    Default configuration file: ./hw/vendor/pulp_platform_gpio/gpio_regs.hjson
    """

    _name = "gpio"
    _config_path = "./hw/vendor/pulp_platform_gpio/gpio_regs.hjson"


class I2C(UserPeripheral, DataConfiguration):
    """
    Inter-Integrated Circuit communication interface.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson
    """

    _name = "i2c"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson"


class RV_timer(UserPeripheral, DataConfiguration):
    """
    RISC-V timer peripheral.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson
    """

    _name = "rv_timer"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson"


class SPI2(UserPeripheral):
    """
    Secondary Serial Peripheral Interface.
    """

    _name = "spi2"


class PDM2PCM(UserPeripheral, DataConfiguration):
    """
    Pulse-density modulation to pulse-code modulation converter.

    Default configuration file: ./hw/ip/pdm2pcm/data/pdm2pcm.hjson
    """

    _name = "pdm2pcm"
    _config_path = "./hw/ip/pdm2pcm/data/pdm2pcm.hjson"


class I2S(UserPeripheral, DataConfiguration):
    """
    Inter-IC Sound interface.

    Default configuration file: ./hw/ip/i2s/data/i2s.hjson
    """

    _name = "i2s"
    _config_path = "./hw/ip/i2s/data/i2s.hjson"


# Domain Class


class UserPeripheralDomain(PeripheralDomain):
    """
    Domain for user peripherals. All user peripherals must be added.

    Base address : 0x30000000
    Length :       0x00100000
    """

    def __init__(self):
        """
        Initialize the user peripheral domain.
        Base address : 0x30000000
        Length :       0x00100000

        At the beginning, there is no base peripheral. All non-added peripherals will be added during build().
        """
        super().__init__(
            name="User",
            base_address=0x30000000,
            length=0x00100000,
        )

    def add_peripheral(self, peripheral: UserPeripheral):
        """
        Add a peripheral to the domain if it is a UserPeripheral. If not, raise an error.

        :param UserPeripheral peripheral: The peripheral to add.
        """
        if not isinstance(peripheral, UserPeripheral):
            raise ValueError("Peripheral is not a UserPeripheral")
        self._peripherals.append(peripheral)

    def remove_peripheral(self, peripheral: UserPeripheral):
        """
        Remove a peripheral from the domain if it is a UserPeripheral.

        :param UserPeripheral peripheral: The peripheral to remove.
        """
        if peripheral not in self._peripherals:
            print(
                f"Warning : Peripheral {peripheral.get_name()} is not in the domain {self._name}"
            )
        self._peripherals.remove(peripheral)
