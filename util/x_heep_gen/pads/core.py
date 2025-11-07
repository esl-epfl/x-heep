# x_heep_gen/pads/core.py
from __future__ import annotations
from dataclasses import dataclass, field, asdict
from typing import Dict, List, Optional, Callable

VALID_TYPES = {"input", "output", "inout", "bypass_input", "bypass_output", "bypass_inout"}

@dataclass(frozen=True)
class MuxAlt:
    signal: str
    type: str

@dataclass(frozen=True)
class PadDef:
    """
    Single-pad definition (what users author in separate files).
    """
    name: str                   # e.g. "uart_tx", "gpio_7"
    type: str                   # "input" | "output" | "inout" | bypass_*
    active: Optional[str] = None            # "low" | "high" | None
    driven_manually: bool = False
    mux: List[MuxAlt] = field(default_factory=list)
    # optional cosmetics/placement:
    mapping: Optional[str] = None              # "top|right|bottom|left" (free-form for now)
    index_hint: Optional[int] = None        # optional suggested order

    def to_cfg_entry(self) -> Dict:
        # Validate types early
        if self.type not in VALID_TYPES:
            raise ValueError(f"{self.name}: invalid type {self.type}")
        for m in self.mux:
            if m.type not in VALID_TYPES:
                raise ValueError(f"{self.name}.mux[{m.signal}]: invalid type {m.type}")
        entry = {"type": self.type}
        if self.active: entry["active"] = self.active
        if self.driven_manually: entry["driven_manually"] = True
        if self.mux:
            entry["mux"] = {m.signal: {"type": m.type} for m in self.mux}
        return entry

# Global in-memory registry (simple and explicit)
_REGISTRY: Dict[str, PadDef] = {}

def register(pad: PadDef) -> None:
    if pad.name in _REGISTRY:
        raise ValueError(f"Pad '{pad.name}' already registered")
    _REGISTRY[pad.name] = pad

def all_pads() -> Dict[str, PadDef]:
    return dict(_REGISTRY)

def build_pad_cfg(selected: List[str], *, extras: Dict[str, Dict]=None) -> Dict:
    """
    Build HJSON-like {"pads": {...}} dict from a list of pad names to include.
    `extras` lets selectors override fields (e.g., {'rst': {'driven_manually': False}}).
    """
    extras = extras or {}
    pads_cfg: Dict[str, Dict] = {}
    for name in selected:
        if name not in _REGISTRY:
            raise KeyError(f"'{name}' not found. Known: {sorted(_REGISTRY)}")
        pad = _REGISTRY[name]
        entry = pad.to_cfg_entry()
        if name in extras:
            # shallow override on purpose (keep it simple)
            entry.update(extras[name])
        pads_cfg[name] = entry
    return {"pads": pads_cfg}
