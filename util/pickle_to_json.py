import pickle
from mako.template import Template
from pathlib import Path
import json
import os
import x_heep_gen
from x_heep_gen.pads import Pad
import hjson
from jsonref import JsonRef
import sys



from enum import Enum
from pathlib import Path


def _enum_to_str(x):
    return x.value if isinstance(x, Enum) else x

def parse_pad(pad: Pad):
    """Return a compact, JSON-safe dict with only the fields that matter."""
    return {
        "name": pad.name,
        "cell_name": pad.cell_name,
        "index": pad.index,
        "pad_type": pad.pad_type,  # "input" | "output" | "inout"
        "pad_mapping": _enum_to_str(pad.pad_mapping),  # "top" | "right" | "bottom" | "left"
        "is_muxed": bool(pad.is_muxed),
        "has_attribute": bool(pad.has_attribute),
        "attribute_bits": int(pad.attribute_bits),
        "constant_attribute": pad.constant_attribute,

        # Summarise the mux list into stable, comparable info:
        "pad_mux_list": [
            {
                "signal_name": m.signal_name,
                "pad_type": m.pad_type,
                "is_driven_manually": bool(m.is_driven_manually),
                "do_skip_declaration": bool(m.do_skip_declaration),
            }
            for m in (pad.pad_mux_list or [])
        ],

        # Layout info if you need placement comparisons:
        "layout": {
            "index": pad.layout_index,
            "orient": pad.layout_orient,
            "cell": pad.layout_cell,
            "bondpad": pad.layout_bondpad,
            "offset": pad.layout_offset,
            "skip": pad.layout_skip,
        },
    }


def convert(path:str,template_path:str,out_path:str):
    #open pickle
    data = pickle.load(open(os.path.join(os.path.dirname(__file__),path), "rb"))
    
    parser_map = {"pads": parse_pad}
    
    
    kwargs = {
        "xheep": data.get("xheep"),
        "cpu_type": data.get("cpu_type"),
        "cve2_rv32e": data.get("cve2_rv32e"),
        "cve2_rv32m": data.get("cve2_rv32m"),
        "external_domains": data.get("external_domains"),
        "debug_start_address": data.get("debug_start_address"),
        "debug_size_address": data.get("debug_size_address"),
        "has_spi_slave": data.get("has_spi_slave"),
        "ext_slave_start_address": data.get("ext_slave_start_address"),
        "ext_slave_size_address": data.get("ext_slave_size_address"),
        "flash_mem_start_address": data.get("flash_mem_start_address"),
        "flash_mem_size_address": data.get("flash_mem_size_address"),
        "stack_size": data.get("stack_size"),
        "heap_size": data.get("heap_size"),
        "plic_used_n_interrupts": data.get("plic_used_n_interrupts"),
        "plit_n_interrupts": data.get("plit_n_interrupts"),
        "interrupts": data.get("interrupts"),
        "pad_list": data.get("pad_list"),
        "external_pad_list": data.get("external_pad_list"),
        "total_pad_list": data.get("total_pad_list"),
        "total_pad": data.get("total_pad"),
        "right_pad_list": data.get("right_pad_list"),
        "left_pad_list": data.get("left_pad_list"),
        "top_pad_list": data.get("top_pad_list"),
        "bottom_pad_list": data.get("bottom_pad_list"),
        "physical_attributes": data.get("physical_attributes"),
        "bondpad_offsets": data.get("bondpad_offsets"),
        "pad_constant_driver_assign": data.get("pad_constant_driver_assign"),
        "pad_mux_process": data.get("pad_mux_process"),
        "pad_muxed_list": data.get("pad_muxed_list"),
        "total_pad_muxed": data.get("total_pad_muxed"),
        "max_total_pad_mux_bitlengh": data.get("max_total_pad_mux_bitlengh"),
        "pads_attributes": data.get("pads_attributes"),
    }
    # --- Convert all pad lists to JSON-safe dicts ---
    pad_keys = [
    "pad_list",
    #"external_pad_list",
    #"total_pad_list",
    #"right_pad_list",
    #"left_pad_list",
    #"top_pad_list",
    #"bottom_pad_list",
    ]

    pad_dicts = {}
    for key in pad_keys:
        pads = kwargs.get(key)
        if pads is not None:
            pad_dicts[key] = [parse_pad(pad) for pad in pads]
        else:
            pad_dicts[key] = []
        
    # --- Render template ---
    pad_groups = {k: [parse_pad(p) for p in (kwargs.get(k) or [])] for k in pad_keys}
    meta = {k: v for k, v in kwargs.items() if k not in pad_keys}

    template = Template(filename=os.path.join(template_path, "pads.json.tpl"))
    rendered_json = template.render(pad_groups=pad_groups, meta=meta)
    #write to out_path
    with open(os.path.join(out_path,"pads.json"), "w") as f:
        f.write(rendered_json)
        
        
if __name__ == "__main__":
    # get path from sys.argv
    if len(sys.argv) != 3:
        print("Usage: python pickle_to_json.py <pickle_path> <prefix>")
        sys.exit(1)
    pickle_path = sys.argv[1]
    prefix = sys.argv[2].lower()
    
    template_path = os.path.join(os.path.dirname(__file__), "testing_templates")
    output_path = os.path.join(os.path.dirname(__file__), prefix+"_jsons")
    convert(pickle_path,template_path,output_path)
    
    
    
    
    
    
    
