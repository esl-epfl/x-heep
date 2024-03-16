import importlib
from pathlib import PurePath
from typing import List
import hjson

from .system import BusType, XHeep

def ram_list(l: "List[int]", entry):
    """
    Parses the hjson ram bank configuration in continuous mode.

    :param List[int] l: the list where bank sizes in kiB should be added
    :enrtry: the entry to be parsed. It should be a list an integer or an continuous dictionary
    :raise RuntimeError: when an invalid configuration is processed.
    """
    if type(l) is not list:
        raise TypeError("l should be of type list")
    
    if type(entry) is int:
        l.append(entry)
        return
    
    if type(entry) is list:
        for i in entry:
            ram_list(l, i)
        return
    
    if type(entry) is hjson.OrderedDict:
        num = 1
        if "num" in entry:
            if type(entry["num"]) is not int:
                raise RuntimeError("if the num field is present in ram configuration it should be an integer")
            num = entry["num"]
        
        if "sizes" in entry:
            for _ in range(num):
                ram_list(l, entry["sizes"])
            return
        else:
            raise RuntimeError("dictionaries in continuous ram configuration sections should at least have a sizes entry")
    
    raise RuntimeError("entries in ram configuration should either be integer, lists, or dictionaries")



def load_ram_configuration(system: XHeep, mem: hjson.OrderedDict):
    """
    Reads the whole ram configuration.

    :param XHeep system: the system object where the ram should be added.
    :param hjson.OrderedDict mem: The configuration part with the ram informations.
    :raise TypeError: when arguments do not have the right type
    :raise RuntimeError: when an invlaid configuration is procesed.
    """
    if not isinstance(system, XHeep):
        raise TypeError("system should be an instance of XHeep object")
    if type(mem) is not hjson.OrderedDict:
        raise TypeError("mem should be of type hjson.OrderedDict")

    for key, value in mem.items():
        if type(value) is not hjson.OrderedDict:
            raise RuntimeError("Ram configuration entries should be dictionaries")
        
        t = "continuous"
        if "type" in value:
            t = value["type"]
            if type(t) is not str:
                raise RuntimeError("ram type should be a string")
            if t != "continuous" and t != "interleaved":
                raise RuntimeError(f"ram type should be continuous or interleaved not {t}")
        
        if t == "interleaved":
            if "num" not in value or type(value["num"]) is not int:
                raise RuntimeError("The num field is required for interleaved ram section and should be an integer")
            
            if "size" not in value or type(value["size"]) is not int:
                raise RuntimeError("The size field is required for interleaved ram section and should be an integer")
            
            system.add_ram_banks_il(int(value["num"]), int(value["size"]), key)

        elif t == "continuous":
            banks: List[int] = []
            ram_list(banks, value)
            system.add_ram_banks(banks, key)



def load_cfg_hjson(src: str) -> XHeep:
    """
    Loads the configuration passed as a hjson string and creates an object representing the mcu.

    :param str src: configuration content
    :return: the object representing the mcu configuration
    :rtype: XHeep
    :raise RuntimeError: when and invalid configuration is passed or when the sanity checks failed
    """
    config = hjson.loads(src, parse_int=int, object_pairs_hook=hjson.OrderedDict)
    mem_config = None
    bus_config = None

    for key, value in config.items():
        if key == "ram_banks":
            mem_config = value
        elif key == "bus_type":
            bus_config = value

    if mem_config is None:
        raise RuntimeError("No memory configuration found")
    if bus_config is None:
        raise RuntimeError("No bus type configuration found")
    
    system = XHeep(BusType(bus_config))
    load_ram_configuration(system, mem_config)

    if not system.validate():
        raise RuntimeError("Could not validate system configuration")
    return system



def _chk_purep(f):
    """
    Helper to check the type is `PurePath`

    :param f: object to check
    :raise TypeError: when object is of wrong type.
    """
    if not isinstance(f, PurePath):
        raise TypeError("parameter should be of type PurePath")



def load_cfg_hjson_file(f: PurePath) -> XHeep:
    """
    Loads the configuration passed in the path as hjson and creates an object representing the mcu.

    :param PurePath f: path of the configuration
    :return: the object representing the mcu configuration
    :rtype: XHeep
    :raise RuntimeError: when and invalid configuration is passed or when the sanity checks failed
    """
    _chk_purep(f)

    with open(f, "r") as file:
         return load_cfg_hjson(file.read())
    


def load_cfg_script_file(f: PurePath) -> XHeep:
    """
    Executes the python file passed as argument to cinfigure the system.

    This file should have a function config that takes no parameters and returns an instance (or subclass) of the XHeep type.
    The script can import modules from the util directory.
    The script should not have side effects as it is called multiple time in the current makefile.

    :param PurePath f: path of the configuration
    :return: the object representing the mcu configuration
    :rtype: XHeep
    :raise RuntimeError: when and invalid configuration is passed or when the sanity checks failed
    """
    _chk_purep(f)

    spec = importlib.util.spec_from_file_location("configs._config", f)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    return mod.config()
    


def load_cfg_file(f: PurePath) -> XHeep:
    """
    Load the Configuration by extension type. It currently supports .hjson and .py

    :param PurePath f: path of the configuration
    :return: the object representing the mcu configuration
    :rtype: XHeep
    :raise RuntimeError: when and invalid configuration is passed or when the sanity checks failed
    """
    _chk_purep(f)

    if f.suffix == ".hjson":
        return load_cfg_hjson_file(f)
    
    if f.suffix == ".py":
        return load_cfg_script_file(f)
    
    raise RuntimeError(f"unsupported file extension {f.suffix}")