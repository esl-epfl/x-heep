def main():
    from .system import XHeep, BusType

    system = XHeep(BusType.NtoM)
    system.add_ram_banks([64]*2)
    system.add_ram_banks_il(2, 64)

    if not system.validate():
        raise RuntimeError("there are errors")

    return system

if __name__ == '__main__':
    main()