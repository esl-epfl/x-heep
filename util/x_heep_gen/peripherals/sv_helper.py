from dataclasses import dataclass


@dataclass
class SvSignal():
    """
    A dataclass to hold information about special local signals used with peripherals
    """
    t: str
    name: str

    def to_inst_declaration(self, comma: bool) -> str:
        """
        get the sv code used for instantiation.
        """
        return f"{self.t} {self.name}" + ("," if comma else "")

@dataclass
class SvSignalArray(SvSignal):
    width: int

    def to_inst_declaration(self, comma: bool) -> str:
        return super().to_inst_declaration(False) + f"[{self.width}-1:0]" + ("," if comma else "")