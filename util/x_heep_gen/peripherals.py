import os.path as path
from abc import (
    ABC,
)  # Used to define abstract classes that cannot be instantiated, only well defined subclasses can be instantiated.
from enum import Enum


class Peripheral(ABC):
    """
    Basic description of a peripheral. These peripherals are not linked to a hjson file. This class cannot be instantiated.

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
    Abstract class for adding configuration of a peripheral. This class cannot be instantiated.
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


class OnOffPeripheral(Peripheral, ABC):
    """
    Abstract class representing optionnal peripherals. This class cannot be instantiated.
    """


# ------------------------------------------------------------
# Base Peripherals (mandatory peripherals)
# ------------------------------------------------------------


class SOC_ctrl(BasePeripheral, DataConfiguration):
    """
    System-on-Chip control peripheral for managing system-level functions and configuration.
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
    """

    _name = PeripheralName.UART
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson"


# ------------------------------------------------------------
# On-Off Peripherals (optional peripherals)
# ------------------------------------------------------------


class RV_plic(OnOffPeripheral, DataConfiguration):
    """
    RISC-V Platform Level Interrupt Controller.
    """

    _name = PeripheralName.RV_plic
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson"


class SPI_host(OnOffPeripheral, DataConfiguration):
    """
    Serial Peripheral Interface host controller.
    """

    _name = PeripheralName.SPI_host
    _config_path = "./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson"


class GPIO(OnOffPeripheral, DataConfiguration):
    """
    General Purpose Input/Output controller.
    """

    _name = PeripheralName.GPIO
    _config_path = "./hw/vendor/pulp_platform_gpio/gpio_regs.hjson"


class I2C(OnOffPeripheral, DataConfiguration):
    """
    Inter-Integrated Circuit communication interface.
    """

    _name = PeripheralName.I2C
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson"


class RV_timer(OnOffPeripheral, DataConfiguration):
    """
    RISC-V timer peripheral.
    """

    _name = PeripheralName.RV_timer
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson"


class SPI2(OnOffPeripheral):
    """
    Secondary Serial Peripheral Interface.
    """

    _name = PeripheralName.SPI2


class PDM2PCM(OnOffPeripheral, DataConfiguration):
    """
    Pulse-density modulation to pulse-code modulation converter.
    """

    _name = PeripheralName.PDM2PCM
    _config_path = "./hw/ip/pdm2pcm/data/pdm2pcm.hjson"


class I2S(OnOffPeripheral, DataConfiguration):
    """
    Inter-IC Sound interface.
    """

    _name = PeripheralName.I2S
    _config_path = "./hw/ip/i2s/data/i2s.hjson"


# ------------------------------------------------------------
# Util functions
# ------------------------------------------------------------


def minimal_config():
    """
    Returns all base peripherals.

    :return: The dictionary of base peripherals already instiated.
    :rtype: dict[str, Peripheral]
    """
    return {
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
        PeripheralName.UART: UART(
            0x000A0000
        ),  # keeping last comma for automatization if ao_peripheral are added
    }


def empty_config():
    """
    Returns an empty configuration.

    :return: The dictionary of all on off peripherals but not instiated.
    :rtype: dict[str, Peripheral]
    """
    return {
        PeripheralName.RV_plic: None,
        PeripheralName.SPI_host: None,
        PeripheralName.GPIO: None,
        PeripheralName.I2C: None,
        PeripheralName.RV_timer: None,
        PeripheralName.SPI2: None,
        PeripheralName.PDM2PCM: None,
        PeripheralName.I2S: None,  # keeping last comma for automatization if peripheral are added
    }


def get_total_length(peripherals: [Peripheral]) -> int:
    """
    Returns the memory size taken by peripherals.

    :param [Peripheral] peripherals: The list of peripherals to get the total length of.
    :return: The total length of the peripherals.
    :rtype: int
    """

    return sum(p.get_length() for p in peripherals) if len(peripherals) > 0 else 0x0


def get_peripheral_list(peripherals):
    """
    Used to ease the use of peripherals, gets list from dictionnary and removes not instantiated peripherals.

    :param peripherals: The peripherals to get the list of.
    :type peripherals: Dict[PeripheralName, Peripheral]
    :return: The list of peripherals.
    :rtype: List[Peripheral]
    """

    return list(filter(lambda x: x is not None, peripherals.values()))


def peripheral_offset_compute(domain_length, peripherals):
    """
    Compute the offset of the peripherals in the domain. If no error is raised, the peripherals are placed in the domain without overlapping.

    :param int domain_length: The length of the domain.
    :param Dict[PeripheralName, Peripheral] peripherals: The dictionary of peripherals to compute the offset of.
    """

    peripherals_list = get_peripheral_list(peripherals)

    peripherals_without_address = [
        p for p in peripherals_list if p.get_address() is None
    ].sort(key=lambda x: x.get_length(), reverse=True)
    peripherals_with_address = [
        p for p in peripherals_list if p.get_address() is not None
    ].sort(key=lambda x: x.get_address())

    free_space = [[0, domain_length]]

    nb_peripherals = (
        0 if peripherals_with_address == None else len(peripherals_with_address)
    )
    nb_peripherals = (
        0 if peripherals_without_address == None else len(peripherals_without_address)
    )

    # Works because peripherals_with_address is sorted by address
    # Splits the last free space into two new lists, one before the peripheral and one after
    for i in range(nb_peripherals):
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
    for i in range(nb_peripherals):
        fit = False  # Check if the peripheral fits in the free space
        for j in range(len(free_space)):
            if (
                peripherals_without_address[i].get_length()
                <= free_space[j][1] - free_space[j][0]
            ):
                peripherals_without_address[i].set_address(free_space[j][0])

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
