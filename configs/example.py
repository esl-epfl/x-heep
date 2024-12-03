from x_heep_gen.linker_section import LinkerSection
from x_heep_gen.pads import PadManager
from x_heep_gen.system import XHeep, BusType

from x_heep_gen.peripherals.peripheral_domain import FixedDomain, PeripheralDomain
from x_heep_gen.peripherals.peripherals import *


def config():
    system = XHeep(BusType.NtoM)
    system.add_ram_banks([32] * 2)
    system.add_ram_banks_il(2, 64, "data_interleaved")

    system.add_linker_section(LinkerSection.by_size("code", 0, 0x00000C800))
    system.add_linker_section(LinkerSection("data", 0x00000C800, None))

    ao_domain = FixedDomain("ao_peripheral", address=0x20000000, addr_size=0x00100000)
    ao_domain.add_peripheral(FixedPeripheral("soc_ctrl",              offset=0x00000000, addr_size=0x00010000))
    ao_domain.add_peripheral(FixedPeripheral("bootrom",               offset=0x00010000, addr_size=0x00010000))
    ao_domain.add_peripheral(FixedPeripheral("spi_flash",             offset=0x00020000, addr_size=0x00008000))
    ao_domain.add_peripheral(FixedPeripheral("spi_memio",             offset=0x00028000, addr_size=0x00008000))
    ao_domain.add_peripheral(FixedPeripheral("dma",                   offset=0x00030000, addr_size=0x00010000))
    ao_domain.add_peripheral(FixedPeripheral("power_manager",         offset=0x00040000, addr_size=0x00010000))
    ao_domain.add_peripheral(FixedPeripheral("rv_timer", suffix="ao", offset=0x00050000, addr_size=0x00010000))
    ao_domain.add_peripheral(FixedPeripheral("fast_intr_ctrl",        offset=0x00060000, addr_size=0x00010000))
    ao_domain.add_peripheral(FixedPeripheral("ext_peripheral",        offset=0x00070000, addr_size=0x00010000))
    ao_domain.add_peripheral(FixedPeripheral("pad_control",           offset=0x00080000, addr_size=0x00010000))
    system.add_domain(ao_domain)

    on_off_domain = PeripheralDomain("peripheral", address=0x30000000, addr_size=0x00100000)
    on_off_domain.add_peripheral(RvTimerPeripheral())
    on_off_domain.add_peripheral(SpiHostPeripheral(dma=True, event_is_fast_intr=True))
    on_off_domain.add_peripheral(SpiHostPeripheral(dma=False))
    on_off_domain.add_peripheral(I2CPeripheral())
    on_off_domain.add_peripheral(I2SPeripheral(dma=True))
    on_off_domain.add_peripheral(RvPlicPeripheral())
    on_off_domain.add_peripheral(Pdm2PcmPeripheral())
    on_off_domain.add_peripheral(GpioPeripheral(gpios_used={i:i for i in range(0,  8)}, intr_map={i:"fast" for i in range(0,  8)}))
    on_off_domain.add_peripheral(GpioPeripheral(gpios_used={i:i for i in range(8, 32)}, intr_map={i:"plic" for i in range(8, 32)}))
    on_off_domain.add_peripheral(UartPeripheral())
    system.add_domain(on_off_domain)

    with open("pad_cfg.hjson") as f:
        system.add_pad_manager(PadManager.load(f.read()))
    
    system.set_ext_intr(2)

    # Here the system is build,
    # The missing gaps are filled, like the missing end address of the data section.
    system.build()
    if not system.validate():
        raise RuntimeError("there are errors")
    
    return system
