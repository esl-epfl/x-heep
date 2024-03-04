def main():
    from .system import XHeep, BusType

    system = XHeep(BusType.NtoM)
    system.add_ram_banks([64], "code")
    system.add_ram_banks([64], "data")
    system.add_ram_banks_il(2, 64, "data_interleaved")

    if not system.validate():
        raise RuntimeError("there are errors")

    return system

if __name__ == '__main__':
    main()