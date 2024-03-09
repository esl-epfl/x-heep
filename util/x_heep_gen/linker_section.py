from dataclasses import *

@dataclass
class LinkerSection():
    """
    Object representing a section in the linker configuration.
    """

    name: str
    """
    The name of the section
    
    The name can be anything that does not collide with section names used by the linker,
    except code and data that are used to configure the size of the code and data part.
    code and data do not only contain the actual .code and .data section but other related sections.
    """

    start: int
    """The start address"""

    end: int
    """The end address"""

    size: int = field(init=False)
    """The size in Bytes"""

    def __post_init__(self):
        self.size = self.end - self.start