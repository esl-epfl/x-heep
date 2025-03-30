from x_heep_gen.linker_section import LinkerSection
from x_heep_gen.system import XHeep, BusType
from x_heep_gen.peripherals import PeripheralName, PeripheralDomain


def config():
    system = XHeep(BusType.NtoM)
    system.add_ram_banks([32] * 2)
    system.add_ram_banks_il(2, 64, "data_interleaved")

    system.add_linker_section(LinkerSection.by_size("code", 0, 0x00000C800))
    system.add_linker_section(LinkerSection("data", 0x00000C800, None))

    # Peripherals
    peripheral_domain = system.configure_peripherals()
    # Only one input, offset of the peripheral in peripheral domain. If set to None, the offset will be automatically computed in system.validate()
    peripheral_domain.configure_rv_plic(0x00000000)
    peripheral_domain.configure_spi_host(0x00010000)
    peripheral_domain.configure_gpio(0x00020000)
    peripheral_domain.configure_i2c(0x00030000)
    peripheral_domain.configure_rv_timer(0x00040000)
    peripheral_domain.configure_spi2(0x00050000)
    peripheral_domain.configure_pdm2pcm(0x00060000)
    peripheral_domain.configure_i2s(0x00070000)

    peripheral_domain.remove_peripheral(
        PeripheralName.PDM2PCM
    )  # Not included in mcu_cfg.hjson

    # Here the system is build,
    # The missing gaps are filled, like the missing end address of the data section.
    system.build()
    if not system.validate():
        raise RuntimeError("there are errors")

    return system
