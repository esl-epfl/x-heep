from x_heep_gen.linker_section import LinkerSection
from x_heep_gen.system import XHeep, BusType

def config():
    system = XHeep(BusType.NtoM)
    system.add_ram_banks([32] * 2)
    system.add_ram_banks_il(2, 64, "data_interleaved")

    system.add_linker_section(LinkerSection.by_size("code", 0, 0x00000C800))
    system.add_linker_section(LinkerSection("data", 0x00000C800, None))

    # Here the system is build,
    # The missing gaps are filled, like the missing end address of the data section.
    system.build()
    if not system.validate():
        raise RuntimeError("there are errors")

    return system
