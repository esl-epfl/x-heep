from dataclasses import dataclass


@dataclass
class ILRamGroup:
    """
    Dataclass to represent information about interleaved memory banks group.
    """

    start: int
    """start address of the group"""

    size: int
    """size of the group"""

    n: int
    """number of banks in the group"""

    first_name: str
    """name of the first bank"""
