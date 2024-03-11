from typing import Optional


def to_int(input) -> Optional[int]:
    if type(input) is int:
        return input
    
    if type(input) is str:
        base = 10
        if len(input) >= 2:
            if input[0:2].upper() == "0X":
                base = 16
                input = input[2:]
            elif input[0:2] == "0o":
                base = 8
                input = input[2:]
            elif input[0:2].upper() == "0b":
                base = 2
                input = input[2:]

        return int(input, base)
    return None

def to_bool(input) -> Optional[bool]:
    if type(input) is int:
        return input != 0
    if type(input) is bool:
        return input
    if type(input) is str:
        if input.lower() in ["yes", "true"]:
            return True
        if input.lower() in ["no", "false"]:
            return False

        return to_bool(to_int(input))
        
    return None