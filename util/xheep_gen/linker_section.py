from dataclasses import *

@dataclass
class LinkerSection():
    name: str
    start: int
    end: int
    size: int = field(init=False)

    def __post_init__(self):
        self.size = self.end - self.start