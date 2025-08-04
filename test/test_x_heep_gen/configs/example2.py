from x_heep_gen.linker_section import LinkerSection
from x_heep_gen.system import XHeep, BusType
from x_heep_gen.peripherals.base_peripherals import (
    BasePeripheralDomain,
    SOC_ctrl,
    Bootrom,
    SPI_flash,
    SPI_memio,
    DMA,
    Power_manager,
    RV_timer_ao,
    Fast_intr_ctrl,
    Ext_peripheral,
    Pad_control,
    GPIO_ao,
    UART,
)
from x_heep_gen.peripherals.user_peripherals import (
    UserPeripheralDomain,
    RV_plic,
    SPI_host,
    GPIO,
    I2C,
    RV_timer,
    SPI2,
    PDM2PCM,
    I2S,
)


def config():
    # System without any user peripheral

    system = XHeep(BusType.NtoM)
    system.add_ram_banks([32] * 2)
    system.add_ram_banks_il(2, 64, "data_interleaved")

    system.add_linker_section(LinkerSection.by_size("code", 0, 0x00000C800))
    system.add_linker_section(LinkerSection("data", 0x00000C800, None))

    # Peripheral domains initialization
    base_peripheral_domain = BasePeripheralDomain()
    user_peripheral_domain = UserPeripheralDomain()

    # Base peripherals. All base peripherals must be added. They can be either added with "add_peripheral" or "add_missing_peripherals" (adds all base peripherals).
    base_peripheral_domain.add_missing_peripherals()

    # User peripherals. All are optional. They must be added with "add_peripheral".
    # No user peripheral is added

    # Add the peripheral domains to the system
    system.add_peripheral_domain(base_peripheral_domain)
    system.add_peripheral_domain(user_peripheral_domain)

    # Here the system is build,
    # The missing gaps are filled, like the missing end address of the data section.
    system.build()
    if not system.validate():
        raise RuntimeError("there are errors")

    return system
