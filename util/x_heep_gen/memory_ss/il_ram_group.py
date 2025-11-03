class ILRamGroup:
    """
    Represents information about a group of interleaved memory banks.
    """

    start: int
    """start address of the group"""

    size: int
    """size of the group"""

    n: int
    """number of banks in the group"""

    first_name: str
    """name of the first bank"""

    def __init__(self, start: int, size: int, n: int, first_name: str):
        self.start = start
        self.size = size
        self.n = n
        self.first_name = first_name

    def __str__(self) -> str:
        return f"ILRamGroup(start=0x{self.start:08X}, size={self.size:08X}, n={self.n}, first_name={self.first_name})"
