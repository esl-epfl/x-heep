
def as_bool(v, default: bool=False) -> bool:
    if isinstance(v, bool): return v
    if isinstance(v, str):
        s = v.strip().lower()
        if s in {"true","1","yes","y"}: return True
        if s in {"false","0","no","n"}: return False
    return default

def get_nested(d, path, default=None):
    cur = d
    for k in path:
        if not isinstance(cur, dict) or k not in cur: return default
        cur = cur[k]
    return cur

def coerce_enum(enum_cls, raw, default=None):
    if raw is None:
        return default
    if isinstance(raw, enum_cls):
        return raw
    try:
        if isinstance(raw, str):
            s = raw.strip(",").strip()
            # Try name match (TOP/Right/etc.) then value match ("top"/"right"/…)
            try:
                return enum_cls[s.upper()]
            except KeyError:
                return enum_cls(s.lower())
        return enum_cls(raw)
    except Exception:
        return default  # or raise if you prefer strictness