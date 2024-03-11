from x_heep_gen.linker_section import LinkerSection
from x_heep_gen.pads import PadManager
from x_heep_gen.system import XHeep, BusType

from x_heep_gen.peripherals.peripheral_domain import PeripheralDomain
from x_heep_gen.peripherals.peripherals import *


def config():
    system = XHeep(BusType.NtoM)
    system.add_ram_banks([32] * 2)
    system.add_ram_banks_il(2, 64, "data_interleaved")

    system.add_linker_section(LinkerSection.by_size("code", 0, 0x00000C800))
    system.add_linker_section(LinkerSection("data", 0x00000C800, None))

    on_off_domain = PeripheralDomain("peripheral", address=0x30000000, addr_size=0x00100000)
    on_off_domain.add_peripheral(RvTimerPeripheral())
    on_off_domain.add_peripheral(SpiHostPeripheral(dma=True, event_is_fast_intr=True))
    on_off_domain.add_peripheral(SpiHostPeripheral(dma=False))
    on_off_domain.add_peripheral(I2CPeripheral())
    on_off_domain.add_peripheral(I2SPeripheral(dma=True))
    on_off_domain.add_peripheral(RvPlicPeripheral())
    on_off_domain.add_peripheral(Pdm2PcmPeripheral())
    on_off_domain.add_peripheral(GpioPeripheral(gpios_used={i:i for i in range(0,8)}, intr_map={i:"fast" for i in range(0, 8)}))
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
