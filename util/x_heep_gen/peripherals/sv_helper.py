from dataclasses import dataclass


@dataclass
class SvSignal():
    t: str
    name: str

    def to_inst_declaration(self, comma: bool) -> str:
        return f"{self.t} {self.name}" + ("," if comma else "")

@dataclass
class SvSignalArray(SvSignal):
    width: int

    def to_inst_declaration(self, comma: bool) -> str:
        return super().to_inst_declaration(False) + f"[{self.width}-1:0]" + ("," if comma else "")