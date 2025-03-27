from x_heep_gen.linker_section import LinkerSection
from x_heep_gen.system import XHeep, BusType


def config():
    system = XHeep(BusType.NtoM)
    system.add_ram_banks([32] * 2)
    system.add_ram_banks_il(2, 64, "data_interleaved")

    system.add_linker_section(LinkerSection.by_size("code", 0, 0x00000C800))
    system.add_linker_section(LinkerSection("data", 0x00000C800, None))

    # Peripherals
    system.configure_peripherals()
    # Only one input, offset of the peripheral in peripheral domain. If set to None, the offset will be automatically computed in system.validate()
    rv_plic = RV_plic(0x00000000)
    spi_host = SPI_host(0x00010000)
    gpio = GPIO(0x00020000)
    i2c = I2C(0x00030000)
    rv_timer = RV_timer(0x00040000)
    spi2 = SPI2(0x00050000)
    pdm2pcm = PDM2PCM(0x00060000)
    i2s = I2S(0x00070000)

    system.add_peripheral(rv_plic)
    system.add_peripheral(spi_host)
    system.add_peripheral(gpio)
    system.add_peripheral(i2c)
    system.add_peripheral(rv_timer)
    system.add_peripheral(spi2)
    system.add_peripheral(pdm2pcm)
    system.add_peripheral(i2s)

    system.remove_peripheral(pdm2pcm)  # Not included in mcu_cfg.hjson

    # Here the system is build,
    # The missing gaps are filled, like the missing end address of the data section.
    system.build()
    if not system.validate():
        raise RuntimeError("there are errors")

    return system
