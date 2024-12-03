from typing import Optional


def to_int(input) -> Optional[int]:
    """
    A helper to read integers from configuration files.

    It can parse integers as `int` or strings.
    It supports prefixes to strings for alternative bases.

    :param input: input
    :return: The integer, None if it could not parse.
    :rtype: Optional[int]
    """
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
    """
    A helper to read booleans from configuration files.

    It can parse integers as `bool` integers or strings.
    Integers are parsed with `ot_int()`, strings can be `yes`, `true`, `no` or `false` and are not case sensistive.
    
    :param input: input
    :return: The boolean, None if it could not parse.
    :rtype: Optional[bool]
    """
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