# Base Peripherals (mandatory peripherals)
from .abstractions import BasePeripheral, DataConfiguration, PeripheralDomain
from copy import deepcopy
import math

# Base Peripherals Classes


class SOC_ctrl(BasePeripheral, DataConfiguration):
    """
    System-on-Chip control peripheral for managing system-level functions and configuration.

    Default configuration file: ./hw/ip/soc_ctrl/data/soc_ctrl.hjson
    """

    _name = "soc_ctrl"
    _config_path = "./hw/ip/soc_ctrl/data/soc_ctrl.hjson"


class Bootrom(BasePeripheral):
    """
    Read-only memory containing the boot code executed at system startup.
    """

    _name = "bootrom"


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


class DMA(BasePeripheral, DataConfiguration):
    """
    Direct Memory Access controller for efficient data transfer between memory and peripherals.

    Default configuration file: ./hw/ip/dma/data/dma.hjson

    :param int ch_length: The length of each channel in the DMA.
    :param int length: The length of the DMA.
    :param int num_channels: The number of channels in the DMA.
    :param int num_master_ports: The number of master ports in the DMA.
    :param int num_channels_per_master_port: The number of channels per master port in the DMA.
    """

    _name = "dma"
    _config_path = "./hw/ip/dma/data/dma.hjson"

    def __init__(
        self,
        address: int = None,
        length: int = None,
        ch_length: int = 0x100,
        num_channels: int = 0x1,
        num_master_ports: int = 0x1,
        num_channels_per_master_port: int = 0x1,
        fifo_depth: int = 0x4,
        addr_mode: str = "yes",
        subaddr_mode: str = "yes",
        hw_fifo_mode: str = "yes",
        zero_padding: str = "yes",
    ):
        """
        Initialize the DMA peripheral.

        :param int address: The virtual (in peripheral domain) memory address of the dma.
        :param int length: The length of the DMA.
        :param int ch_length: The length of each channel in the DMA.
        :param int num_channels: The number of channels in the DMA.
        :param int num_master_ports: The number of master ports in the DMA.
        :param int num_channels_per_master_port: The number of channels per master port in the DMA.
        """
        super().__init__(address, length)
        self._ch_length = ch_length
        self._num_channels = num_channels
        self._num_master_ports = num_master_ports
        self._num_channels_per_master_port = num_channels_per_master_port
        self._fifo_depth = fifo_depth
        self._addr_mode = 0 if addr_mode == "no" else 1
        self._subaddr_mode = 0 if subaddr_mode == "no" else 1
        self._hw_fifo_mode = 0 if hw_fifo_mode == "no" else 1
        self._zero_padding = 0 if zero_padding == "no" else 1

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

    def get_fifo_depth(self):
        """
        Get the depth of the DMA FIFO.
        """
        return self._fifo_depth

    def set_fifo_depth(self, value: int):
        """
        Set the depth of the DMA FIFO.
        """
        self._fifo_depth = value

    def set_addr_mode(self, value: str):
        """
        Set the address mode of the DMA.
        """
        if value not in ["yes", "no"]:
            raise ValueError("Invalid address mode. Must be 'yes' or 'no'.")

        if value == "yes":
            self._addr_mode = 1
        else:
            self._addr_mode = 0

    def get_addr_mode(self):
        """
        Get the address mode of the DMA.
        """
        return self._addr_mode

    def set_subaddr_mode(self, value: str):
        """
        Set the subaddress mode of the DMA.
        """
        if value not in ["yes", "no"]:
            raise ValueError("Invalid subaddress mode. Must be 'yes' or 'no'.")

        if value == "yes":
            self._subaddr_mode = 1
        else:
            self._subaddr_mode = 0

    def get_subaddr_mode(self):
        """
        Get the subaddress mode of the DMA.
        """
        return self._subaddr_mode

    def set_hw_fifo_mode(self, value: str):
        """
        Set the hardware FIFO mode of the DMA.
        """
        if value not in ["yes", "no"]:
            raise ValueError("Invalid hardware FIFO mode. Must be 'yes' or 'no'.")

        if value == "yes":
            self._hw_fifo_mode = 1
        else:
            self._hw_fifo_mode = 0

    def get_hw_fifo_mode(self):
        """
        Get the hardware FIFO mode of the DMA.
        """
        return self._hw_fifo_mode

    def set_zero_padding(self, value: str):
        """
        Set the zero padding mode of the DMA.
        """
        if value not in ["yes", "no"]:
            raise ValueError("Invalid zero padding mode. Must be 'yes' or 'no'.")

        if value == "yes":
            self._zero_padding = 1
        else:
            self._zero_padding = 0

    def get_zero_padding(self):
        """
        Get the zero padding mode of the DMA.
        """
        return self._zero_padding

    def get_xbar_array(self):
        """
        Get the DMA xbar array.
        """
        if self.get_num_master_ports() > 1:
            # Computation of full_masters_xbars
            temp_full_masters_xbars = math.floor(
                self.get_num_channels() / self.get_num_channels_per_master_port()
            )
            if (
                temp_full_masters_xbars < self.get_num_master_ports()
                and temp_full_masters_xbars * self.get_num_channels_per_master_port()
                == self.get_num_channels()
            ):
                full_masters_xbars = temp_full_masters_xbars - 1
            else:
                full_masters_xbars = temp_full_masters_xbars
            last = self.get_num_channels_per_master_port() * full_masters_xbars

            # Array initialization
            array_xbar_gen = [0] * self.get_num_master_ports()

            # Computation of the number of xbar channels for each master port
            for i in range(self.get_num_master_ports()):
                if i < full_masters_xbars:
                    array_xbar_gen[i] = self.get_num_channels_per_master_port()
                else:
                    array_xbar_gen[i] = min(
                        self.get_num_channels() - last,
                        self.get_num_channels()
                        - last
                        - (self.get_num_master_ports() - i - 1),
                    )
                    last = last + array_xbar_gen[i]

            if sum(array_xbar_gen) != self.get_num_channels() or 0 in array_xbar_gen:
                exit("Error in the DMA xbar generation: wrong parameters")

            print(", ".join(map(str, array_xbar_gen)))
            return ", ".join(map(str, array_xbar_gen))
        else:
            if self.get_num_channels_per_master_port() != self.get_num_channels():
                exit(
                    "With 1 master port, the number of DMA channels per master port has to be equal to the number of DMA channels"
                )
            return "default: 1"

    def validate(self):
        """
        Checks if the DMA peripheral is valid (number of channels between 0 and 256, master ports between 0 and number of channels, channels per master port between 0 and number of channels, number of channels per master port is not 0 if number of channels is not 1).
        """
        valid = True
        if self.get_num_channels() > 256 or self.get_num_channels() == 0:
            print("Number of DMA channels has to be between 0 and 256, excluded")
            valid = False

        if (
            self.get_num_master_ports() > self.get_num_channels()
            or self.get_num_master_ports() == 0
        ):
            print(
                "Number of DMA master ports has to be between 0 and "
                + str(self.get_num_channels())
                + ", 0 excluded"
            )
            valid = False

        if (
            self.get_num_channels_per_master_port() > self.get_num_channels()
            and self.get_num_channels() != 1
        ) or self.get_num_channels_per_master_port() == 0:
            print(
                "Number of DMA channels per system bus master ports has to be between 0 and "
                + str(self.get_num_channels())
                + ", excluded"
            )
            valid = False

        return valid


class Power_manager(BasePeripheral, DataConfiguration):
    """
    Manages power states and clock gating for different system components.

    Default configuration file: ./hw/ip/power_manager/data/power_manager.hjson
    """

    _name = "power_manager"
    _config_path = "./hw/ip/power_manager/data/power_manager.hjson"


class RV_timer_ao(BasePeripheral):
    """
    RISC-V timer peripheral for system timing and scheduling.
    """

    _name = "rv_timer_ao"


class Fast_intr_ctrl(BasePeripheral, DataConfiguration):
    """
    Fast interrupt controller for low-latency interrupt handling.

    Default configuration file: ./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson
    """

    _name = "fast_intr_ctrl"
    _config_path = "./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson"


class Ext_peripheral(BasePeripheral):
    """
    Interface for external peripheral connections.
    """

    _name = "ext_peripheral"


class Pad_control(BasePeripheral):
    """
    Controls the configuration of IO pads.
    """

    _name = "pad_control"


class GPIO_ao(BasePeripheral):
    """
    General Purpose Input/Output controller.
    """

    _name = "gpio_ao"


class UART(BasePeripheral, DataConfiguration):
    """
    Universal Asynchronous Receiver/Transmitter for serial communication.

    Default configuration file: ./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson
    """

    _name = "uart"
    _config_path = "./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson"


# Domain Class


class BasePeripheralDomain(PeripheralDomain):
    """
    Domain for base peripherals. All base peripherals must be added.

    Start address : 0x20000000
    Length :       0x00100000
    """

    # List of all base peripherals names
    _default_base_peripherals = [
        SOC_ctrl(),
        Bootrom(),
        SPI_flash(),
        SPI_memio(),
        DMA(),
        Power_manager(),
        RV_timer_ao(),
        Fast_intr_ctrl(),
        Ext_peripheral(),
        Pad_control(),
        GPIO_ao(),
        UART(),
    ]

    def __init__(self, start_address: int = 0x20000000, length: int = 0x00100000):
        """
        Initialize the base peripheral domain.
        Start address : 0x20000000
        Length :       0x00100000

        At the beginning, there is no base peripheral. All non-added peripherals will be added during build().
        """
        super().__init__(
            name="Base",
            start_address=start_address,
            length=length,
        )

    def add_peripheral(self, peripheral: BasePeripheral):
        """
        Add a peripheral to the domain if it is a BasePeripheral. If not, raise an error.

        :param BasePeripheral peripheral: The peripheral to add.
        """
        if not isinstance(peripheral, BasePeripheral):
            raise ValueError("Peripheral is not a BasePeripheral")
        self._peripherals.append(peripheral)

    def remove_peripheral(self, peripheral: BasePeripheral):
        """
        Remove a peripheral from the domain if it is a BasePeripheral.

        :param BasePeripheral peripheral: The peripheral to remove.
        """
        if peripheral not in self._peripherals:
            print(
                f"Warning : Peripheral {peripheral.get_name()} is not in the domain {self._name}"
            )
        self._peripherals.remove(peripheral)

    def add_missing_peripherals(self):
        """
        Add missing peripherals to the domain.
        """
        # Add all default peripherals
        peripherals_to_add = [deepcopy(p) for p in self._default_base_peripherals]

        # Remove peripherals that are already in the domain to obtain the list of missing peripherals
        for peripheral in self._peripherals:
            for p in peripherals_to_add:
                if type(peripheral) == type(p):
                    peripherals_to_add.remove(p)
                    break

        # Add the missing peripherals
        for p in peripherals_to_add:
            self.add_peripheral(p)

    def get_all_dmas(self):
        """
        Get the DMA peripherals.

        :return: The DMA peripherals.
        :rtype: list[DMA]
        """
        dmas = []
        for p in self._peripherals:
            if isinstance(p, DMA):
                dmas.append(deepcopy(p))
        if len(dmas) == 0:
            raise ValueError("No DMA peripheral found")
        return dmas

    def get_dma(self):
        """
        Get the main DMA peripheral (the first appended DMA peripheral).

        :return: The DMA peripheral.
        :rtype: DMA
        """
        return self.get_all_dmas()[0]

    # Validate functions

    def __check_all_peripherals_added(self):
        """
        Check if all base peripherals are added.
        """

        # For each default peripheral, there should be at least one instance in the domain
        all_peripherals_added = True
        for default_peripheral in self._default_base_peripherals:
            added = False
            for peripheral in self._peripherals:
                if type(peripheral) == type(default_peripheral):
                    added = True
                    break
            if not added:
                all_peripherals_added = False
                print(
                    f"Peripheral {peripheral.get_name()} is not in the domain {self._name}"
                )

        return all_peripherals_added

    def validate(self):
        """
        Validate the base peripheral domain. Checks if all base peripherals are added, if they don't overlap and if their configuration paths are valid. Checks also if dmas are valid.

        :return: True if the base peripheral domain is valid, False otherwise.
        :rtype: bool
        """
        dma_valid = True
        for dma in self.get_all_dmas():
            dma_valid &= dma.validate()

        return self.__check_all_peripherals_added() and super().validate() and dma_valid
