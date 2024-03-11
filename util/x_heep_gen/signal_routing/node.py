from dataclasses import *


@dataclass
class Node:
    name: str
    p_name: str = field(compare=False)