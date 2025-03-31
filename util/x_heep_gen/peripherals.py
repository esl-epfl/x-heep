import os.path as path
from abc import (
    ABC,
)  # Used to define abstract classes that cannot be instantiated, only well defined subclasses can be instantiated.
from enum import Enum
from copy import deepcopy  # Used to create a deep copy of the peripheral


class Peripheral(ABC):
    """
    Basic description of a peripheral. These peripherals are not linked to a hjson file, they only have a memory range. This class cannot be instantiated.

    :param int address: The virtual (in peripheral domain) memory address of the peripheral, the base address should be known by the creator of the class.
    :param int length: The size taken in memory by the peripheral
    """

    _length: int = int("0x00010000", 16)  # default length of 64KB
    _name: str
    _address: int = None

    def __init__(self, offset=None):
        """
        Initialize the peripheral with a given address.

        :param int offset: The virtual (in peripheral domain) memory address of the peripheral. If None, the offset will be automatically compute during build function.
        """
        if type(offset) == int and offset >= 0x00000000:
            self._address = offset
        else:
            self._address = None

    def get_address(self):
        """
        :return: The virtual (in peripheral domain) memory address of the peripheral. If not set, return None.
        :rtype: int
        """
        return self._address

    def set_address(self, address):
        """
        Set the virtual (in peripheral domain) memory address of the peripheral.
        """
        self._address = address

    def get_length(self):
        """
        :return: The length of the peripheral.
        :rtype: int
        """
        return self._length

    def get_name(self):
        """
        :return: The name of the peripheral.
        :rtype: str
        """
        return self._name


class PeripheralName(Enum):
    """
    Enum for the names of the peripherals.
    """

    # Always on peripherals
    SOC_ctrl = "soc_ctrl"
    Bootrom = "bootrom"
    SPI_flash = "spi_flash"
    SPI_memio = "spi_memio"
    DMA = "dma"
    Power_manager = "power_manager"
    RV_timer_ao = "rv_timer_ao"
    Fast_intr_ctrl = "fast_intr_ctrl"
    Ext_peripheral = "ext_peripheral"
    Pad_control = "pad_control"
    GPIO_ao = "gpio_ao"
    UART = "uart"

    # Optional peripherals
    RV_plic = "rv_plic"
    SPI_host = "spi_host"
    GPIO = "gpio"
    I2C = "i2c"
    RV_timer = "rv_timer"
    SPI2 = "spi2"
    PDM2PCM = "pdm2pcm"
    I2S = "i2s"


class DataConfiguration(ABC):
    """
    Abstract class for adding a more sofisticated configuration to a peripheral, acts as an interface in Java. This class cannot be instantiated.

    :param str config_path: The path to the hjson file that describes the peripheral (interface and registers).
    """

    _config_path: str

    def custom_configuration(self, config_path: str):
        """
        Select a custom configuration for the peripheral.

        :param str config_path: The path to the hjson file that describes the peripheral. If the path does not exist, a FileNotFoundError will be raised.
        """
        if not path.exists(config_path):
            raise FileNotFoundError(f"The config file {config_path} does not exist")
        self._config_path = config_path

    def get_config_path(self):
        """
        :return: The path to the hjson file that describes the peripheral.
        :rtype: str
        """
        return self._config_path


class BasePeripheral(Peripheral, ABC):
    """
    Abstract class representing always-on peripherals. This class cannot be instantiated.
    """


class UserPeripheral(Peripheral, ABC):
    """
    Abstract class representing user-configurable peripherals. This class cannot be instantiated.
    """


# ------------------------------------------------------------
# Base Peripherals (mandatory peripherals)
# ------------------------------------------------------------


class SOC_ctrl(BasePeripheral, DataConfiguration):
    """
    System-on-Chip control peripheral for managing system-level functions and configuration.

    Default configuration file: ./hw/ip/soc_ctrl/data/soc_ctrl.hjson
    """

    _name = PeripheralName.SOC_ctrl
    _config_path = "./hw/ip/soc_ctrl/data/soc_ctrl.hjson"


class Bootrom(BasePeripheral):
    """
    Read-only memory containing the boot code executed at system startup.
    """

    _name = PeripheralName.Bootrom


class SPI_flash(BasePeripheral):
    """
    Interface for external SPI flash memory access.
    """

    _name = PeripheralName.SPI_flash
    _length: int = 0x00008000


class SPI_memio(BasePeripheral):
    """
    Memory-mapped IO interface for SPI communication.
    """

    _name = PeripheralName.SPI_memio
    _length: int = 0x00008000


class DMA(BasePeripheral, DataConfiguration):
    """
    Direct Memory Access controller for efficient data transfer between memory and peripherals.

    Default configuration file: ./hw/ip/dma/data/dma.hjson

    :param int ch_length: The length of each channel in the DMA.
    :param int num_channels: The number of channels in the DMA.
    :param int num_master_ports: The number of master ports in the DMA.
    :param int num_channels_per_master_port: The number of channels per master port in the DMA.
    """

    _name = PeripheralName.DMA
    _config_path = "./hw/ip/dma/data/dma.hjson"

    def __init__(
        self,
        address: int,
        ch_length: int = 0x100,
        num_channels: int = 0x1,
        num_master_ports: int = 0x1,
        num_channels_per_master_port: int = 0x1,
    ):
        """
        Initialize the DMA peripheral.

        :param int address: The virtual (in peripheral domain) memory address of the dma.
        :param int ch_length: The length of each channel in the DMA.
        :param int num_channels: The number of channels in the DMA.
        :param int num_master_ports: The number of master ports in the DMA.
        :param int num_channels_per_master_port: The number of channels per master port in the DMA.
        """
        super().__init__(address)
        self._ch_length = ch_length
        self._num_channels = num_channels
        self._num_master_ports = num_master_ports
        self._num_channels_per_master_port = num_channels_per_master_port

    def set_ch_length(self, value: int):
        """
        Set the length of each channel in the DMA.
        """
        self._ch_length = value

    def get_ch_length(self):
        """
        Get the length of each channel in the DMA.
        """
        return self._ch_length

    def set_num_channels(self, value: int):
        """
        Set the number of channels in the DMA.
        """
        self._num_channels = value

    def get_num_channels(self):
        """
        Get the number of channels in the DMA.
        """
        return self._num_channels

    def set_num_master_ports(self, value: int):
        """
        Set the number of master ports in the DMA.
        """
        self._num_master_ports = value

    def get_num_master_ports(self):
        """
        Get the number of master ports in the DMA.
        """
        return self._num_master_ports

    def set_num_channels_per_master_port(self, value: int):
        """
        Set the number of channels per master port in the DMA.
        """
        self._num_channels_per_master_port = value

    def get_num_channels_per_master_port(self):
        """
        Get the number of channels per master port in the DMA.
        """
        return self._num_channels_per_master_port


class Power_manager(BasePeripheral, DataConfiguration):
    """
    Manages power states and clock gating for different system components.

    Default configuration file: ./hw/ip/power_manager/data/power_manager.hjson
    """

    _name = PeripheralName.Power_manager
    _config_path = "./hw/ip/power_manager/data/power_manager.hjson"


class RV_timer_ao(BasePeripheral):
    """
    RISC-V timer peripheral for system timing and scheduling.
    """

    _name = PeripheralName.RV_timer_ao


class Fast_intr_ctrl(BasePeripheral, DataConfiguration):
    """
    Fast interrupt controller for low-latency interrupt handling.

    Default configuration file: ./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson
    """

    _name = PeripheralName.Fast_intr_ctrl
    _config_path = "./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson"


class Ext_peripheral(BasePeripheral):
    """
    Interface for external peripheral connections.
    """

    _name = PeripheralName.Ext_peripheral


class Pad_control(BasePeripheral):
    """
    Controls the configuration of IO pads.
    """

    _name = PeripheralName.Pad_control


class GPIO_ao(BasePeripheral):
    """
    General Purpose Input/Output controller.
    """

    _name = PeripheralName.GPIO_ao


class UART(BasePeripheral, DataConfiguration):
    """
    Universal Asynchronous Receiver/Transmitter for serial communication.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson
    """

    _name = PeripheralName.UART
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson"


# ------------------------------------------------------------
# User Peripherals (optional peripherals)
# ------------------------------------------------------------


class RV_plic(UserPeripheral, DataConfiguration):
    """
    RISC-V Platform Level Interrupt Controller.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson
    """

    _name = PeripheralName.RV_plic
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson"


class SPI_host(UserPeripheral, DataConfiguration):
    """
    Serial Peripheral Interface host controller.

    Default configuration file: ./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson
    """

    _name = PeripheralName.SPI_host
    _config_path = "./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson"


class GPIO(UserPeripheral, DataConfiguration):
    """
    General Purpose Input/Output controller.

    Default configuration file: ./hw/vendor/pulp_platform_gpio/gpio_regs.hjson
    """

    _name = PeripheralName.GPIO
    _config_path = "./hw/vendor/pulp_platform_gpio/gpio_regs.hjson"


class I2C(UserPeripheral, DataConfiguration):
    """
    Inter-Integrated Circuit communication interface.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson
    """

    _name = PeripheralName.I2C
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson"


class RV_timer(UserPeripheral, DataConfiguration):
    """
    RISC-V timer peripheral.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson
    """

    _name = PeripheralName.RV_timer
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson"


class SPI2(UserPeripheral):
    """
    Secondary Serial Peripheral Interface.
    """

    _name = PeripheralName.SPI2


class PDM2PCM(UserPeripheral, DataConfiguration):
    """
    Pulse-density modulation to pulse-code modulation converter.

    Default configuration file: ./hw/ip/pdm2pcm/data/pdm2pcm.hjson
    """

    _name = PeripheralName.PDM2PCM
    _config_path = "./hw/ip/pdm2pcm/data/pdm2pcm.hjson"


class I2S(UserPeripheral, DataConfiguration):
    """
    Inter-IC Sound interface.

    Default configuration file: ./hw/ip/i2s/data/i2s.hjson
    """

    _name = PeripheralName.I2S
    _config_path = "./hw/ip/i2s/data/i2s.hjson"


# ------------------------------------------------------------
# Peripheral domain
# ------------------------------------------------------------


class PeripheralDomain:
    """
    Class representing the peripheral domain. Peripherals can only be added and modified through configure and add functions, all modifications outside these functions won't be recorded.

    :param int base_peripheral_start_address: The start address for the base peripherals.
    :param int user_peripheral_start_address: The start address for the user peripherals.
    :param int base_peripheral_length: The length for the base peripherals.
    :param int user_peripheral_length: The length for the user peripherals.
    :param dict[PeripheralName, Peripheral] base_peripherals: The base peripherals. The keys are the PeripheralName enum values.
    :param dict[PeripheralName, Peripheral] user_peripherals: The user peripherals. The keys are the PeripheralName enum values.
    """

    _base_peripheral_start_address = (
        0x20000000  # default start address for base peripherals
    )
    _base_peripheral_length = 0x00100000  # default length for base peripherals

    """
    Base peripherals are always present in the system.
    """
    _base_peripherals = {
        PeripheralName.SOC_ctrl: SOC_ctrl(0x00000000),
        PeripheralName.Bootrom: Bootrom(0x00010000),
        PeripheralName.SPI_flash: SPI_flash(0x00020000),
        PeripheralName.SPI_memio: SPI_memio(0x00028000),
        PeripheralName.DMA: DMA(0x00030000),
        PeripheralName.Power_manager: Power_manager(0x00040000),
        PeripheralName.RV_timer_ao: RV_timer_ao(0x00050000),
        PeripheralName.Fast_intr_ctrl: Fast_intr_ctrl(0x00060000),
        PeripheralName.Ext_peripheral: Ext_peripheral(0x00070000),
        PeripheralName.Pad_control: Pad_control(0x00080000),
        PeripheralName.GPIO_ao: GPIO_ao(0x00090000),
        PeripheralName.UART: UART(0x000A0000),
    }

    _user_peripheral_start_address = (
        0x30000000  # default start address for user peripherals
    )
    _user_peripheral_length = 0x00100000  # default length for user peripherals

    """
    User peripherals are optional peripherals that can be added to the system.
    """
    _user_peripherals = {
        PeripheralName.RV_plic: None,
        PeripheralName.SPI_host: None,
        PeripheralName.GPIO: None,
        PeripheralName.I2C: None,
        PeripheralName.RV_timer: None,
        PeripheralName.SPI2: None,
        PeripheralName.PDM2PCM: None,
        PeripheralName.I2S: None,
    }

    def __init__(
        self,
        base_peripherals_start_address=0x20000000,
        user_peripherals_start_address=0x30000000,
    ):
        """
        Initialize the peripheral domain. Peripherals domains are 1MB length each. Base peripherals are automatically placed in the domain, but no user peripherals are instianted.

        :param int base_peripherals_start_address: The start address for the base peripherals.
        :param int user_peripherals_start_address: The start address for the user peripherals.
        """
        self._base_peripheral_start_address = base_peripherals_start_address
        self._user_peripheral_start_address = user_peripherals_start_address

    def get_peripheral(self, peripheral_name: PeripheralName):
        """
        Returns a copy of the peripheral, all modifications in the copy will not be recoreded.

        :param PeripheralName peripheral_name: The name of the peripheral to get.
        :return: The peripheral.
        :rtype: Peripheral
        """
        return (
            deepcopy(self._base_peripherals[peripheral_name])
            if peripheral_name in self._base_peripherals.keys()
            else deepcopy(self._user_peripherals[peripheral_name])
        )

    def get_base_peripherals(self):
        """
        Returns a copy of the base peripherals, all modifications in the copy will not be recoreded.
        """
        return deepcopy(self._base_peripherals)

    def get_user_peripherals(self):
        """
        Returns a copy of the user peripherals, all modifications in the copy will not be recoreded.
        """
        return deepcopy(self._user_peripherals)

    def get_peripherals(self):
        """
        Returns a copy of the peripherals, all modifications in the copy will not be recoreded.
        """
        return deepcopy(self._base_peripherals) + deepcopy(self._user_peripherals)

    def add_peripheral(self, peripheral: Peripheral):
        """
        Add a peripheral to the domain. If the peripheral is already in the user domain, it will be replaced. If the peripheral is a base peripheral, it will raise an error.

        :param Peripheral peripheral: The peripheral to add.
        """
        if peripheral.get_name() in self._base_peripherals.keys():
            raise ValueError(f"Peripheral {peripheral.get_name()} is a base peripheral")
        if peripheral.get_name() not in self._user_peripherals.keys():
            raise ValueError(
                f"Peripheral {peripheral.get_name()} not found in the domain"
            )

        self._user_peripherals[peripheral.get_name()] = peripheral.deepcopy()

    def remove_peripheral(self, peripheral_name: PeripheralName):
        """
        Remove a peripheral from the domain. If the peripheral is not in the user domain, it will raise an error.

        :param PeripheralName peripheral_name: The name of the peripheral to remove.
        """
        if peripheral_name in self._base_peripherals.keys():
            raise ValueError(f"Peripheral {peripheral_name} is a base peripheral")
        if peripheral_name not in self._user_peripherals.keys():
            raise ValueError(f"Peripheral {peripheral_name} not found in the domain")

        self._user_peripherals[peripheral_name] = None

    def get_base_peripherals_base_address(self):
        """
        Returns the base address of the base peripherals.
        """
        return self._base_peripheral_start_address

    def get_user_peripherals_base_address(self):
        """
        Returns the base address of the user peripherals.
        """
        return self._user_peripheral_start_address

    def get_base_peripherals_length(self):
        """
        Returns the memory size taken by base peripherals.

        :return: The total length of the base peripherals.
        :rtype: int
        """

        return self._base_peripheral_length

    def get_user_peripherals_length(self):
        """
        Returns the memory size taken by user peripherals.

        :return: The total length of the user peripherals.
        :rtype: int
        """

        return self._user_peripheral_length

    def get_base_peripheral_list(self):
        """
        Returns the list of base peripherals.

        :return: The list of peripherals.
        :rtype: List[Peripheral]
        """

        return list(
            filter(lambda x: x is not None, deepcopy(self._base_peripherals).values())
        )

    def get_user_peripheral_list(self):
        """
        Returns the list of instantiated user peripherals.

        :return: The list of peripherals.
        :rtype: List[Peripheral]
        """

        return list(
            filter(lambda x: x is not None, deepcopy(self._user_peripherals).values())
        )

    # Base peripherals configuration functions

    def configure_soc_ctrl(self, offset=None, config_path=None):
        """
        Configure the soc_ctrl peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check SOC_CTRL documentation for more information).
        """
        peripheral = (
            SOC_ctrl(offset)
            if self._base_peripherals[PeripheralName.SOC_ctrl] is None
            else self._base_peripherals[PeripheralName.SOC_ctrl].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._base_peripherals[PeripheralName.SOC_ctrl] = peripheral

    def configure_bootrom(self, offset=None):
        """
        Configure the bootrom peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            Bootrom(offset)
            if self._base_peripherals[PeripheralName.Bootrom] is None
            else self._base_peripherals[PeripheralName.Bootrom].set_address(offset)
        )
        self._base_peripherals[PeripheralName.Bootrom] = peripheral

    def configure_spi_memio(self, offset=None):
        """
        Configure the spi_memio peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            SPI_memio(offset)
            if self._base_peripherals[PeripheralName.SPI_memio] is None
            else self._base_peripherals[PeripheralName.SPI_memio].set_address(offset)
        )
        self._base_peripherals[PeripheralName.SPI_memio] = peripheral

    def configure_spi_flash(self, offset=None):
        """
        Configure the spi_flash peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            SPI_flash(offset)
            if self._base_peripherals[PeripheralName.SPI_flash] is None
            else self._base_peripherals[PeripheralName.SPI_flash].set_address(offset)
        )
        self._base_peripherals[PeripheralName.SPI_flash] = peripheral

    def configure_dma(
        self,
        offset=None,
        config_path=None,
        ch_length=None,
        num_channels=None,
        num_master_ports=None,
        num_channels_per_master_port=None,
    ):
        """
        Configure the dma peripheral. All parameters are optional, only the ones that are not None will be set (the others will keep their values).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check DMA documentation for more information).
        :param int ch_length: The length of each channel in the DMA.
        :param int num_channels: The number of channels in the DMA.
        :param int num_master_ports: The number of master ports in the DMA.
        :param int num_channels_per_master_port: The number of channels per master port in the DMA.
        """
        peripheral = (
            DMA(offset)
            if self._base_peripherals[PeripheralName.DMA] is None
            else self._base_peripherals[PeripheralName.DMA].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        if ch_length is not None:
            peripheral.set_ch_length(ch_length)
        if num_channels is not None:
            peripheral.set_num_channels(num_channels)
        if num_master_ports is not None:
            peripheral.set_num_master_ports(num_master_ports)
        if num_channels_per_master_port is not None:
            peripheral.set_num_channels_per_master_port(num_channels_per_master_port)
        self._base_peripherals[PeripheralName.DMA] = peripheral

    def configure_power_manager(self, offset=None, config_path=None):
        """
        Configure the power_manager peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check POWER_MANAGER documentation for more information).
        """
        peripheral = (
            Power_manager(offset)
            if self._base_peripherals[PeripheralName.Power_manager] is None
            else self._base_peripherals[PeripheralName.Power_manager].set_address(
                offset
            )
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._base_peripherals[PeripheralName.Power_manager] = peripheral

    def configure_rv_timer_ao(self, offset=None):
        """
        Configure the rv_timer_ao peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            RV_timer_ao(offset)
            if self._base_peripherals[PeripheralName.RV_timer_ao] is None
            else self._base_peripherals[PeripheralName.RV_timer_ao].set_address(offset)
        )
        self._base_peripherals[PeripheralName.RV_timer_ao] = peripheral

    def configure_fast_intr_ctrl(self, offset=None, config_path=None):
        """
        Configure the fast_intr_ctrl peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check FAST_INTR_CTRL documentation for more information).
        """
        peripheral = (
            Fast_intr_ctrl(offset)
            if self._base_peripherals[PeripheralName.Fast_intr_ctrl] is None
            else self._base_peripherals[PeripheralName.Fast_intr_ctrl].set_address(
                offset
            )
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._base_peripherals[PeripheralName.Fast_intr_ctrl] = peripheral

    def configure_ext_peripheral(self, offset=None):
        """
        Configure the ext_peripheral peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            Ext_peripheral(offset)
            if self._user_peripherals[PeripheralName.Ext_peripheral] is None
            else self._user_peripherals[PeripheralName.Ext_peripheral].set_address(
                offset
            )
        )
        self._user_peripherals[PeripheralName.Ext_peripheral] = peripheral

    def configure_pad_control(self, offset=None):
        """
        Configure the pad_control peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            Pad_control(offset)
            if self._user_peripherals[PeripheralName.Pad_control] is None
            else self._user_peripherals[PeripheralName.Pad_control].set_address(offset)
        )
        self._user_peripherals[PeripheralName.Pad_control] = peripheral

    def configure_gpio_ao(self, offset=None):
        """
        Configure the gpio_ao peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            GPIO_ao(offset)
            if self._user_peripherals[PeripheralName.GPIO_ao] is None
            else self._user_peripherals[PeripheralName.GPIO_ao].set_address(offset)
        )
        self._user_peripherals[PeripheralName.GPIO_ao] = peripheral

    def configure_uart(self, offset=None, config_path=None):
        """
        Configure the uart peripheral.

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check UART documentation for more information).
        """
        peripheral = (
            UART(offset)
            if self._user_peripherals[PeripheralName.UART] is None
            else self._user_peripherals[PeripheralName.UART].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.UART] = peripheral

    # User peripherals configuration functions

    def configure_rv_plic(self, offset=None, config_path=None):
        """
        Configure the rv_plic peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check RV_PLIC documentation for more information).
        """
        peripheral = (
            RV_plic(offset)
            if self._user_peripherals[PeripheralName.RV_plic] is None
            else self._user_peripherals[PeripheralName.RV_plic].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.RV_plic] = peripheral

    def configure_spi_host(self, offset=None, config_path=None):
        """
        Configure the spi_host peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check SPI_HOST documentation for more information).
        """
        peripheral = (
            SPI_host(offset)
            if self._user_peripherals[PeripheralName.SPI_host] is None
            else self._user_peripherals[PeripheralName.SPI_host].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.SPI_host] = peripheral

    def configure_gpio(self, offset=None, config_path=None):
        """
        Configure the gpio peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check GPIO documentation for more information).
        """
        peripheral = (
            GPIO(offset)
            if self._user_peripherals[PeripheralName.GPIO] is None
            else self._user_peripherals[PeripheralName.GPIO].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.GPIO] = peripheral

    def configure_i2c(self, offset=None, config_path=None):
        """
        Configure the i2c peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check I2C documentation for more information).
        """
        peripheral = (
            I2C(offset)
            if self._user_peripherals[PeripheralName.I2C] is None
            else self._user_peripherals[PeripheralName.I2C].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.I2C] = peripheral

    def configure_rv_timer(self, offset=None, config_path=None):
        """
        Configure the rv_timer peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check RV_TIMER documentation for more information).
        """
        peripheral = (
            RV_timer(offset)
            if self._user_peripherals[PeripheralName.RV_timer] is None
            else self._user_peripherals[PeripheralName.RV_timer].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.RV_timer] = peripheral

    def configure_spi2(self, offset=None):
        """
        Configure the spi2 peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        """
        peripheral = (
            SPI2(offset)
            if self._user_peripherals[PeripheralName.SPI2] is None
            else self._user_peripherals[PeripheralName.SPI2].set_address(offset)
        )
        self._user_peripherals[PeripheralName.SPI2] = peripheral

    def configure_pdm2pcm(self, offset=None, config_path=None):
        """
        Configure the pdm2pcm peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check PDM2PCM documentation for more information).
        """
        peripheral = (
            PDM2PCM(offset)
            if self._user_peripherals[PeripheralName.PDM2PCM] is None
            else self._user_peripherals[PeripheralName.PDM2PCM].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.PDM2PCM] = peripheral

    def configure_i2s(self, offset=None, config_path=None):
        """
        Configure the i2s peripheral (creates a new one if it doesn't exist).

        :param int offset: The offset of the peripheral. If None, the offset will be automatically compute during build function.
        :param str config_path: The path to the hjson file that describes the peripheral. If None, the default configuration file will be used (check I2S documentation for more information).
        """
        peripheral = (
            I2S(offset)
            if self._user_peripherals[PeripheralName.I2S] is None
            else self._user_peripherals[PeripheralName.I2S].set_address(offset)
        )
        if config_path is not None:
            peripheral.custom_configuration(config_path)
        self._user_peripherals[PeripheralName.I2S] = peripheral

    # Private functions

    def __get_base_peripherals_list(self):
        """
        Get the list of base peripherals.
        """
        return list(filter(lambda x: x is not None, self._base_peripherals.values()))

    def __get_user_peripherals_list(self):
        """
        Get the list of user peripherals.
        """
        return list(filter(lambda x: x is not None, self._user_peripherals.values()))

    def __peripheral_offset_compute(self, domain_length, peripherals):
        """
        Compute the offset of the peripherals in the domain. If no error is raised, the peripherals are placed in the domain without overlapping.

        :param int domain_length: The length of the domain.
        :param Dict[PeripheralName, Peripheral] peripherals: The dictionary of peripherals to compute the offset of. The dictionary contains all peripherals, so it will update them without going through add and configure functions.
        """

        peripherals_without_address = [
            p for p in peripherals.values() if p is not None and p.get_address() is None
        ].sort(key=lambda x: x.get_length(), reverse=True)
        peripherals_with_address = [
            p
            for p in peripherals.values()
            if p is not None and p.get_address() is not None
        ].sort(key=lambda x: x.get_address())

        free_space = [[0, domain_length]]

        num_peripherals_with_address = (
            0 if peripherals_with_address == None else len(peripherals_with_address)
        )
        num_peripherals_without_address = (
            0
            if peripherals_without_address == None
            else len(peripherals_without_address)
        )

        # Works because peripherals_with_address is sorted by address
        # Splits the last free space into two new lists, one before the peripheral and one after
        for i in range(num_peripherals_with_address):
            # Removes last free space to split it
            last_free_space = free_space[-1]
            free_space.pop()

            # Checks if the peripheral domain is in free space
            if peripherals_with_address[i].get_address() < last_free_space[0]:
                if i == 0:
                    raise ValueError(
                        f"Peripheral {peripherals_with_address[i].get_name()} has an address that starts before the first free space ({peripherals_with_address[i].get_name()} starts at {hex(peripherals_with_address[i].get_address())} but first free space starts at {hex(last_free_space[0])})"
                    )
                else:
                    raise ValueError(
                        f"Peripheral {peripherals_with_address[i].get_name()} has an address that starts in {peripherals_with_address[i-1].get_name()} domain ({peripherals_with_address[i].get_name()} starts at {hex(peripherals_with_address[i].get_address())} but {peripherals_with_address[i-1].get_name()} ends at {hex(last_free_space[0])}"
                    )

            if (
                peripherals_with_address[i].get_address()
                + peripherals_with_address[i].get_length()
            ) > last_free_space[1]:
                raise ValueError(
                    f"Peripheral {peripherals_with_address[i].get_name()} has an address that ends after the last free space ({peripherals_with_address[i].get_name()} ends at {hex(peripherals_with_address[i].get_address() + peripherals_with_address[i].get_length())} but last free space ends at {hex(last_free_space[1])})"
                )

            # If the peripheral starts after the last free space, add it to the free space before the peripheral
            if last_free_space[0] > peripherals_with_address[i].get_address():
                free_space.append(
                    [last_free_space[0], peripherals_with_address[i].get_address()]
                )

            # If the peripheral ends before the last free space, add it to the free space after the peripheral
            if (
                peripherals_with_address[i].get_address()
                + peripherals_with_address[i].get_length()
            ) < last_free_space[1]:
                free_space.append(
                    [
                        peripherals_with_address[i].get_address()
                        + peripherals_with_address[i].get_length(),
                        last_free_space[1],
                    ]
                )

        # Place peripherals where in the first free space where they fit, works because peripherals_without_address is sorted by length in descending order
        offsets = (
            {}
        )  # Will contain the offsets of the peripherals, and then update the peripherals with the offsets if they all fit
        for i in range(num_peripherals_without_address):
            fit = False  # Check if the peripheral fits in the free space
            for j in range(len(free_space)):
                if (
                    peripherals_without_address[i].get_length()
                    <= free_space[j][1] - free_space[j][0]
                ):
                    offsets[peripherals_without_address[i].get_name()] = free_space[j][
                        0
                    ]

                    # Either remove space if the peripheral exactly fits the space, or shrinks the space
                    if (
                        free_space[j][0] + peripherals_without_address[i].get_length()
                        == free_space[j][1]
                    ):
                        free_space.pop(j)
                    else:
                        free_space[j][0] += peripherals_without_address[i].get_length()

                    # Ends search for free space
                    fit = True
                    print(
                        f"Placed peripheral {peripherals_without_address[i].get_name()} at {hex(free_space[j][0])}"
                    )
                    break
            if not fit:
                raise ValueError(
                    f"Could not find a free space large enough for peripheral {peripherals_without_address[i].get_name()} with length {hex(peripherals_without_address[i].get_length())}"
                )

        for name, offset in offsets.items():
            peripherals[name].set_address(offset)

    def __check_peripheral_non_overlap(self, peripherals, domain_length):
        """
        Check if the peripherals do not overlap.

        :param dict[PeripheralName, Peripheral] peripherals: The peripherals to check.
        :param int domain_length: The length of the domain.
        :return: True if the peripherals do not overlap, False otherwise.
        :rtype: bool
        """
        peripherals_sorted = sorted(
            filter(lambda x: x != None, peripherals.values()),
            key=lambda x: x.get_address(),
        )  # Filter out None values and sort by offset in growing order
        return_bool = True

        if len(peripherals_sorted) == 0:
            return return_bool

        for i in range(len(peripherals_sorted) - 1):
            if (
                peripherals_sorted[i].get_address() + peripherals_sorted[i].get_length()
                > peripherals_sorted[i + 1].get_address()
            ):
                print(
                    f"The peripheral {peripherals_sorted[i].get_name()} overflows over the domain (starts at {peripherals_sorted[i].get_address():#08X} and ends at {peripherals_sorted[i].get_address() + peripherals_sorted[i].get_length():#08X}, peripheral {peripherals_sorted[i+1].get_name()} starts at {peripherals_sorted[i+1].get_address():#08X})."
                )
                return_bool = False
            if peripherals_sorted[i].get_address() >= domain_length:
                print(
                    f"The peripheral {peripherals_sorted[i].get_name()} is out of the domain (starts at {peripherals_sorted[i].get_address():#08X}, domain ends at {domain_length:#08X})."
                )
                return_bool = False

        # Check if the last peripheral is out of the domain
        if (
            peripherals_sorted[-1].get_address() + peripherals_sorted[-1].get_length()
            > domain_length
        ):
            print(
                f"The peripheral {peripherals_sorted[-1].get_name()} is out of the domain (starts at {peripherals_sorted[-1].get_address():#08X}, domain ends at {domain_length:#08X})."
            )
            return_bool = False
        if peripherals_sorted[-1].get_address() >= domain_length:
            print(
                f"The peripheral {peripherals_sorted[-1].get_name()} is out of the domain (starts at {peripherals_sorted[-1].get_address():#08X}, domain ends at {domain_length:#08X})."
            )
            return_bool = False

        return return_bool

    # Validation functions

    def check_peripheral_domains_no_overlap(self):
        """
        Check if the peripherals domains overlap.

        :return: True if the domains do not overlap, False otherwise.
        :rtype: bool
        """

        ret = True
        if (
            self._base_peripheral_start_address < self._user_peripheral_start_address
            and self._base_peripheral_start_address + self._base_peripheral_length
            > self._user_peripheral_start_address
        ):
            print(
                f"The base peripheral domain (ends at {self._base_peripheral_start_address + self._base_peripheral_length:#08X}) overflows over user peripheral domain (starts at {self._user_peripheral_start_address:#08X})."
            )
            ret = False
        if (
            self._user_peripheral_start_address < self._base_peripheral_start_address
            and self._user_peripheral_start_address + self._user_peripheral_length
            > self._base_peripheral_start_address
        ):
            print(
                f"The user peripheral domain (ends at {self._user_peripheral_start_address + self._user_peripheral_length:#08X}) overflows over base peripheral domain (starts at {self._base_peripheral_start_address:#08X})."
            )
            ret = False
        if self._user_peripheral_start_address == self._base_peripheral_start_address:
            print(
                f"The base peripheral domain and the user peripheral domain should not start at the same address (current addresses are {self._base_peripheral_start_address:#08X} and {self._user_peripheral_start_address:#08X})."
            )
            ret = False
        if self._base_peripheral_start_address < 0x10000:  # from mcu_gen.py
            print(
                f"Always on peripheral start address must be greater than 0x10000, current address is {self._base_peripheral_start_address:#08X}."
            )
            ret = False
        return ret

    def check_peripheral_non_overlap(self):
        """
        Check if the peripherals do not overlap.
        """
        ret = True

        self.__peripheral_offset_compute(
            self._user_peripheral_length, self._user_peripherals
        )
        self.__peripheral_offset_compute(
            self._base_peripheral_length, self._base_peripherals
        )

        if not self.__check_peripheral_non_overlap(
            self._base_peripherals, self._base_peripheral_length
        ):
            ret = False
        if not self.__check_peripheral_non_overlap(
            self._user_peripherals, self._user_peripheral_length
        ):
            ret = False
        return ret

    def check_paths(self, peripherals_list):
        return_bool = True
        for p in peripherals_list:
            if isinstance(p, DataConfiguration):
                # path stands for os.path
                if not path.exists(p.get_config_path()) and not path.exists(
                    p.get_config_path() + ".tpl"
                ):
                    print(
                        f"The peripheral {p.get_name()} has an invalid path : {p.get_config_path()}"
                    )
                    return_bool = False
        return return_bool
