from dataclasses import *


@dataclass
class Node:
    """
    A dataclass representing a node of the routing helper.
    """
    name: str
    p_name: str = field(compare=False)