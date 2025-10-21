#!/usr/bin/env python3

# Copyright 2020 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

# Simplified version of occamygen.py https://github.com/pulp-platform/snitch/blob/master/util/occamygen.py

import argparse
import hjson
import pathlib
import sys
import re
import logging
import pickle
from subprocess import run
import csv
from jsonref import JsonRef
from mako.template import Template
import collections
from math import log2
import x_heep_gen.load_config
from x_heep_gen.load_config import load_peripherals_config
from x_heep_gen.system import BusType
import x_heep_gen.peripherals.base_peripherals
import x_heep_gen.peripherals.user_peripherals
import x_heep_gen.peripherals.abstractions
from x_heep_gen.pads import Pad,PadMapping
import math
import os



# Compile a regex to trim trailing whitespaces on lines.
re_trailws = re.compile(r"[ \t\r]+$", re.MULTILINE)


def string2int(hex_json_string):
    return (hex_json_string.split("x")[1]).split(",")[0]


def write_template(tpl_path, outfile, **kwargs):
    if tpl_path:
        tpl_path = pathlib.Path(tpl_path).absolute()
        if tpl_path.exists():
            tpl = Template(filename=str(tpl_path))
            if outfile:
                filename = outfile
            else:
                filename = tpl_path.with_suffix("")

            with open(filename, "w") as file:
                code = tpl.render_unicode(**kwargs)
                code = re_trailws.sub("", code)
                file.write(code)
        else:
            raise FileNotFoundError("Template file not found: {0}".format(tpl_path))
    else:
        raise FileNotFoundError("Template file not provided")


def prepare_pads_for_layout(total_pad_list, physical_attributes):
    """
    Separate pads into pad lists for the top, bottom, left, and right pads and order them according to their layout_index attribute, and set their positions on the floorplan.
    """

    # Separate pads according to side
    top_pad_list = []
    bottom_pad_list = []
    right_pad_list = []
    left_pad_list = []
    pad_lists ={
        PadMapping.TOP: top_pad_list,
        PadMapping.BOTTOM: bottom_pad_list,
        PadMapping.RIGHT: right_pad_list,
        PadMapping.LEFT: left_pad_list,
    }
    for pad in total_pad_list:
        if pad.pad_mapping in pad_lists:
            pad_lists[pad.pad_mapping].append(pad)
        else:
            print(
                "ERROR: Pad {0} has an invalid mapping {1}. Please set mapping to top, bottom, left, or right."
                .format(pad.cell_name, getattr(pad, "pad_mapping", None))
            )
            return

    # Order pads according to layout index
    top_pad_list.sort(key=lambda x: x.layout_index)
    bottom_pad_list.sort(key=lambda x: x.layout_index)
    left_pad_list.sort(key=lambda x: x.layout_index)
    right_pad_list.sort(key=lambda x: x.layout_index)

    # Calculate pad offsets and check whether requested pad configuration fits in the floorplan
    top_pad_list, bondpad_offset_top = set_pad_positions(
        top_pad_list, physical_attributes
    )
    bottom_pad_list, bondpad_offset_bottom = set_pad_positions(
        bottom_pad_list, physical_attributes
    )
    left_pad_list, bondpad_offset_left = set_pad_positions(
        left_pad_list, physical_attributes
    )
    right_pad_list, bondpad_offset_right = set_pad_positions(
        right_pad_list, physical_attributes
    )

    bondpad_offsets = {
        "top": bondpad_offset_top,
        "bottom": bondpad_offset_bottom,
        "left": bondpad_offset_left,
        "right": bondpad_offset_right,
    }

    return top_pad_list, bottom_pad_list, left_pad_list, right_pad_list, bondpad_offsets


def set_pad_positions(pad_list, physical_attributes):
    """Calculate the `offset` and `skip` attributes of the pads such that the bondpads are centered on each side and the pads are aligned with their respective bondpads.
    Perform checks to make sure the pads can all fit on the requested side without violating design constraints or exceeding layout margins.
    """

    # Ensure the physical attributes were properly set in the pad config file
    try:
        fp_width = float(physical_attributes["floorplan_dimensions"]["width"])
        fp_length = float(physical_attributes["floorplan_dimensions"]["length"])
        edge_to_bp = float(physical_attributes["edge_offset"]["bondpad"])
        edge_to_pad = float(physical_attributes["edge_offset"]["pad"])
        bp_spacing = float(physical_attributes["spacing"]["bondpad"])
        pad_dims = physical_attributes["dimensions"]
    except KeyError:
        print(
            "ERROR: Please set all of the mandatory fields of the physical_attributes in the pad config file."
        )
        return

    # Determine which dimension we are dealing with
    side = pad_list[0].pad_mapping
    if (side == "top") | (side == "bottom"):
        side_length = fp_width
    else:
        side_length = fp_length

    # Calculate space occupied by bondpads on the designated side of the chip
    bp_space = 0
    for pad in pad_list:
        bp_cell = pad.layout_bondpad
        if bp_cell is not None:
            # Get bondpad width from physical attributes
            try:
                bp_width = float(pad_dims[bp_cell]["width"])
            except KeyError:
                print(
                    "ERROR: Width not defined for bondpad cell {0} of pad {1}".format(
                        bp_cell, pad.cell_name
                    )
                )
                return
        else:
            print(
                "ERROR: A bondpad cell is not defined for pad {1}".format(pad.cell_name)
            )
            return
        bp_space += bp_width
    bp_space += bp_spacing * (len(pad_list) - 1)

    # Check if the bondpads are able to fit on the side
    extra_space = side_length - bp_space - 2 * edge_to_bp
    if extra_space < 0:
        print(
            "ERROR: Bondpads cannot fit on side {0}. Either reduce bondpad spacing or move some pads to another side".format(
                side
            )
        )

    # Calculate distance from edge to first bondpad (i.e. bondpad offset) to center the pads
    bp_offset = extra_space / 2

    # Calculate skip parameter between one pad and the next to center the pads
    for i, pad in enumerate(pad_list):

        # Get bondpad width from physical attributes
        bp_cell = pad.layout_bondpad
        bp_width = float(pad_dims[bp_cell]["width"])

        if i > 0:
            last_bp_cell = pad_list[i - 1].layout_bondpad
            last_bp_width = float(pad_dims[last_bp_cell]["width"])

        # Get pad width from physical attributes
        pad_cell = pad.layout_cell
        if pad_cell is not None:
            try:
                pad_width = float(pad_dims[pad_cell]["width"])
            except KeyError:
                print(
                    "ERROR: Width not defined for pad cell {0} of pad {1}".format(
                        pad_cell, pad.cell_name
                    )
                )
                return
        else:
            print("ERROR: A pad cell is not defined for pad {1}".format(pad.cell_name))
            return

        if i > 0:
            last_pad_cell = pad_list[i - 1].layout_cell
            if pad_cell is not None:
                try:
                    last_pad_width = float(pad_dims[last_pad_cell]["width"])
                except KeyError:
                    print(
                        "ERROR: Width not defined for pad cell {0} of pad {1}".format(
                            last_pad_cell, pad_list[i - 1].cell_name
                        )
                    )
                    return
            else:
                print(
                    "ERROR: A pad cell is not defined for pad {1}".format(
                        pad_list[i - 1].cell_name
                    )
                )
                return
        if (i == 0) & (pad.layout_offset is None) & (pad.layout_skip is None):
            pad.layout_offset = (
                bp_offset
                - (edge_to_pad - edge_to_bp)
                + (bp_width / 2)
                - (pad_width / 2)
            )

        # If the layout/skip of the pads is not predefined, calculate automatically
        if (pad.layout_offset is None) & (pad.layout_skip is None):
            pad.layout_skip = (
                (last_bp_width + bp_width) / 2
                + bp_spacing
                - (last_pad_width + pad_width) / 2
            )

    return pad_list, bp_offset




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

def build_mux_list(block,
                   pad_mapping,
                   pads_attributes_present: bool,
                   pads_attributes_bits: str,
                   pad_constant_attribute: bool,
                   pad_layout_index, pad_layout_orient, pad_layout_cell,
                   pad_layout_bondpad, pad_layout_offset, pad_layout_skip) :
    mux_list = []
    for mux_name, entry in (block.get("mux") or {}).items():
        mux = Pad(
            mux_name, "",
            entry["type"],
            pad_mapping,
            0,
            entry.get("active", "high"),
            as_bool(entry.get("driven_manually"), False),
            as_bool(entry.get("skip_declaration"), False),
            [],
            pads_attributes_present,
            pads_attributes_bits,
            pad_constant_attribute,
            pad_layout_index, pad_layout_orient, pad_layout_cell,
            pad_layout_bondpad, pad_layout_offset, pad_layout_skip,
        )
        mux_list.append(mux)
    return mux_list

def build_pads_from_block(pads_block,
                          start_index: int,
                          pads_attributes_present: bool,
                          pads_attributes_bits: str,
                          default_constant_attribute: bool,
                          always_emit_ring: bool) :
    pad_list = []
    pad_muxed_list = []
    const_assign_parts = []
    mux_process_parts = []
    next_index = start_index

    for key, block in pads_block.items():
        base_name = key
        pad_num   = int(block["num"])
        pad_type  = (block["type"].strip(",") if isinstance(block["type"], str)
                     else block["type"])
        pad_offset = int(block.get("num_offset", 0))
        pad_active = block.get("active", "high")
        pad_mapping = coerce_enum(PadMapping, block.get("mapping"), None)

        pad_driven_manually = as_bool(block.get("driven_manually"), False)
        pad_skip_declaration = as_bool(block.get("skip_declaration"), False)
        pad_keep_internal   = as_bool(block.get("keep_internal"), False)
        pad_constant_attribute = as_bool(block.get("constant_attribute"),
                                         default_constant_attribute)

        # layout (optional)
        pad_layout_orient = get_nested(block, ["layout_attributes","orient"])
        pad_layout_cell   = get_nested(block, ["layout_attributes","cell"])
        pad_layout_bondpad= get_nested(block, ["layout_attributes","bondpad"])
        pad_layout_offset = get_nested(block, ["layout_attributes","offset"])
        pad_layout_skip   = get_nested(block, ["layout_attributes","skip"])
        pad_layout_index  = get_nested(block, ["layout_attributes","index"])

        # mux list
        pad_mux_list = build_mux_list(block, pad_mapping,
                                      pads_attributes_present, pads_attributes_bits,
                                      pad_constant_attribute,
                                      pad_layout_index, pad_layout_orient, pad_layout_cell,
                                      pad_layout_bondpad, pad_layout_offset, pad_layout_skip)

        # unified loop (single/multi)
        for i in range(pad_num):
            idx = next_index + i
            suf = f"_{i + pad_offset}" if pad_num > 1 else ""
            pad_name = f"{base_name}{suf}"
            pad_cell_name = f"pad_{base_name}{suf}_i"

            pad_obj = Pad(
                pad_name, pad_cell_name, pad_type, pad_mapping, idx,
                pad_active, pad_driven_manually, pad_skip_declaration,
                pad_mux_list, pads_attributes_present, pads_attributes_bits,
                pad_constant_attribute,
                pad_layout_index, pad_layout_orient, pad_layout_cell,
                pad_layout_bondpad, pad_layout_offset, pad_layout_skip,
            )

            # build sections (internal can skip ring; external always emits ring)
            emit_ring = always_emit_ring or not pad_keep_internal
            if emit_ring:
                pad_obj.create_pad_ring()
            pad_obj.create_core_v_mini_mcu_ctrl()
            if emit_ring:
                pad_obj.create_pad_ring_bonding()
            pad_obj.create_internal_signals()
            pad_obj.create_constant_driver_assign()
            pad_obj.create_multiplexers()
            pad_obj.create_core_v_mini_mcu_bonding()

            pad_list.append(pad_obj)
            const_assign_parts.append(pad_obj.constant_driver_assign)
            mux_process_parts.append(pad_obj.mux_process)
            if pad_obj.is_muxed:
                pad_muxed_list.append(pad_obj)

        next_index += pad_num

    return pad_list, pad_muxed_list, next_index, "".join(const_assign_parts), "".join(mux_process_parts)


"""
    Ideally, generate the xheep object with the configuration passed in args. After generating the xheep object, serialize it to a file and save it.

    Currently, generates xheep object with other parameters (and serialize everything)
"""


def generate_xheep(args):

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)

    with open(args.config, "r") as file:
        try:
            srcfull = file.read()
            config = hjson.loads(srcfull, use_decimal=True)
            config = JsonRef.replace_refs(config)
        except ValueError:
            raise SystemExit(sys.exc_info()[1])

    with open(args.pads_cfg, "r") as file:
        try:
            srcfull = file.read()
            pad_cfg = hjson.loads(srcfull, use_decimal=True)
            pad_cfg = JsonRef.replace_refs(pad_cfg)
        except ValueError:
            raise SystemExit(sys.exc_info()[1])

    config_override = x_heep_gen.system.Override(None, None, None)

    if args.cpu != None and args.cpu != "":
        cpu_type = args.cpu
    else:
        cpu_type = config["cpu_type"]

    if "cve2_rv32e" in config:
        cve2_rv32e = config["cve2_rv32e"]
    else:
        cve2_rv32e = None

    if "cve2_rv32m" in config:
        cve2_rv32m = config["cve2_rv32m"]
    else:
        cve2_rv32m = None

    if args.bus != None and args.bus != "":
        config_override.bus_type = BusType(args.bus)

    if args.memorybanks != None and args.memorybanks != "":
        config_override.numbanks = int(args.memorybanks)

    if args.memorybanks_il != None and args.memorybanks_il != "":
        config_override.numbanks_il = int(args.memorybanks_il)

    if args.external_domains != None and args.external_domains != "":
        external_domains = int(args.external_domains)
    else:
        external_domains = 0

    if external_domains > 32:
        exit(
            "external_domains must be less than 32 instead of " + str(external_domains)
        )

    try:
        has_spi_slave = 1 if config["debug"]["has_spi_slave"] == "yes" else 0
    except KeyError:
        has_spi_slave = 0

    if args.python_x_heep_cfg != None and args.python_x_heep_cfg != "":
        xheep = x_heep_gen.load_config.load_cfg_file(
            pathlib.PurePath(str(args.python_x_heep_cfg)), config_override
        )
    else:
        xheep = x_heep_gen.load_config.load_cfg_file(
            pathlib.PurePath(str(args.config)), config_override
        )

    # config is used as the base config for peripherals (if a domain is not defined in the config, it will be added to xheep using informations in config)
    load_peripherals_config(xheep, args.config)

    debug_start_address = string2int(config["debug"]["address"])
    if int(debug_start_address, 16) < int("10000", 16):
        exit("debug start address must be greater than 0x10000")

    debug_size_address = string2int(config["debug"]["length"])
    ext_slave_start_address = string2int(config["ext_slaves"]["address"])
    ext_slave_size_address = string2int(config["ext_slaves"]["length"])

    flash_mem_start_address = string2int(config["flash_mem"]["address"])
    flash_mem_size_address = string2int(config["flash_mem"]["length"])

    stack_size = string2int(config["linker_script"]["stack_size"])
    heap_size = string2int(config["linker_script"]["heap_size"])

    if (int(stack_size, 16) + int(heap_size, 16)) > xheep.ram_size_address():
        exit(
            "The stack and heap section must fit in the RAM size, instead they takes "
            + str(stack_size + heap_size)
        )

    plic_used_n_interrupts = len(config["interrupts"]["list"])
    plit_n_interrupts = config["interrupts"]["number"]
    ext_int_list = {
        f"EXT_INTR_{k}": v
        for k, v in enumerate(range(plic_used_n_interrupts, plit_n_interrupts))
    }

    interrupts = {**config["interrupts"]["list"], **ext_int_list}

    pads = pad_cfg["pads"]

    try:
        pads_attributes = pad_cfg["attributes"]
        pads_attributes_bits = pads_attributes["bits"]
    except KeyError:
        pads_attributes = None
        pads_attributes_bits = "-1:0"

    # Read HJSON description of External Pads
    if "external_pads" in config:
        external_pads = config["external_pads"]
    else:
        external_pads = None

    pad_list = []
    pad_index_counter = 0
    external_pad_list = []
    external_pad_index_counter = 0

    pad_constant_driver_assign = ""
    pad_mux_process = ""

    pad_muxed_list = []

# internal pads
    pad_list, pad_muxed_internal, next_index, pad_constant_driver_assign, pad_mux_process = \
        build_pads_from_block(
            pads_block=pads,
            start_index=0,
            pads_attributes_present=(pads_attributes is not None),
            pads_attributes_bits=pads_attributes_bits,
            default_constant_attribute=False,
            always_emit_ring=False,   # respect keep_internal for internal pads
        )

    # external pads (continue indexing, always emit ring)
    external_pad_list = []
    pad_muxed_external = []
    external_pad_index_counter = 0

    if external_pads:
        external_pad_list, pad_muxed_external, next_index, ext_const, ext_mux = \
            build_pads_from_block(
                pads_block=external_pads,
                start_index=next_index,
                pads_attributes_present=(pads_attributes is not None),
                pads_attributes_bits=pads_attributes_bits,
                default_constant_attribute=False,
                always_emit_ring=True,   # external pads always generate pad ring
            )
        pad_constant_driver_assign += ext_const
        pad_mux_process += ext_mux
        external_pad_index_counter = len(external_pad_list)

    # merge, totals
    total_pad_list = pad_list + external_pad_list
    pad_muxed_list = pad_muxed_internal + pad_muxed_external
    total_pad = len(total_pad_list)
    total_pad_muxed = len(pad_muxed_list)

    # max mux selector width (0 if none)
    max_total_pad_mux_bitlengh = 0
    if pad_muxed_list:
        max_total_pad_mux_bitlengh = max((len(p.pad_mux_list) - 1).bit_length() for p in pad_muxed_list)

    # remove trailing comma from last PAD io_interface (kept to preserve behavior)
    if total_pad_list:
        last_pad = total_pad_list.pop()
        last_pad.remove_comma_io_interface()
        total_pad_list.append(last_pad)

        # If layout parameters exist in the config, compute the pad offset/skip parameters and order the pads on each side
    try:
        physical_attributes = pad_cfg["physical_attributes"]
        (
            top_pad_list,
            bottom_pad_list,
            left_pad_list,
            right_pad_list,
            bondpad_offsets,
        ) = prepare_pads_for_layout(total_pad_list, physical_attributes)
    except KeyError:
        physical_attributes = None
        top_pad_list = None
        bottom_pad_list = None
        left_pad_list = None
        right_pad_list = None
        bondpad_offsets = None

    kwargs = {
        "xheep": xheep,
        "cpu_type": cpu_type,
        "cve2_rv32e": cve2_rv32e,
        "cve2_rv32m": cve2_rv32m,
        "external_domains": external_domains,
        "debug_start_address": debug_start_address,
        "debug_size_address": debug_size_address,
        "has_spi_slave": has_spi_slave,
        "ext_slave_start_address": ext_slave_start_address,
        "ext_slave_size_address": ext_slave_size_address,
        "flash_mem_start_address": flash_mem_start_address,
        "flash_mem_size_address": flash_mem_size_address,
        "stack_size": stack_size,
        "heap_size": heap_size,
        "plic_used_n_interrupts": plic_used_n_interrupts,
        "plit_n_interrupts": plit_n_interrupts,
        "interrupts": interrupts,
        "pad_list": pad_list,
        "external_pad_list": external_pad_list,
        "total_pad_list": total_pad_list,
        "total_pad": total_pad,
        "right_pad_list": right_pad_list,
        "left_pad_list": left_pad_list,
        "top_pad_list": top_pad_list,
        "bottom_pad_list": bottom_pad_list,
        "physical_attributes": physical_attributes,
        "bondpad_offsets": bondpad_offsets,
        "pad_constant_driver_assign": pad_constant_driver_assign,
        "pad_mux_process": pad_mux_process,
        "pad_muxed_list": pad_muxed_list,
        "total_pad_muxed": total_pad_muxed,
        "max_total_pad_mux_bitlengh": max_total_pad_mux_bitlengh,
        "pads_attributes": pads_attributes,
    }

    return kwargs


def main():
    parser = argparse.ArgumentParser(prog="mcugen")

    parser.add_argument(
        "--cached_path", "-cp", help="Path to the cached xheep file", required=True
    )

    parser.add_argument(
        "--cached",
        "-ca",
        help="If set, the script will not generate the xheep object, but will use the cached version instead",
        required=False,
        action="store_true",
    )

    args, _ = parser.parse_known_args()

    if args.cached:
        # Validate cached file exists
        if not os.path.exists(args.cached_path):
            parser.error(
                f"Cached file {args.cached_path} does not exist. Cannot use --cached flag."
            )

        # X-Heep object has been generated
        with open(args.cached_path, "rb") as f:
            kwargs = pickle.load(f)

        parser.add_argument(
            "--outfile",
            "-o",
            type=pathlib.Path,
            required=False,
            help="Target filename. If not provided, the template filename will be used as the output filename.",
        )

        parser.add_argument(
            "--outtpl",
            "-ot",
            type=pathlib.Path,
            required=True,
            help="Target template filename",
        )

        args = parser.parse_args()
        outtpl = args.outtpl
        outfile = args.outfile

        write_template(outtpl, outfile, **kwargs)

    else:
        # X-Heep object must be generated
        cached_path = args.cached_path

        parser.add_argument(
            "--config",
            metavar="file",
            type=str,
            required=True,
            help="X-Heep general configuration",
        )

        parser.add_argument(
            "--python_x_heep_cfg",
            metavar="file",
            type=str,
            required=False,
            nargs="?",
            default="",
            help="X-Heep custom configuration",
        )

        parser.add_argument(
            "--pads_cfg",
            "-pc",
            metavar="file",
            type=str,
            required=True,
            help="Pads configuration",
        )

        parser.add_argument(
            "--cpu",
            metavar="cv32e20,cv32e40p,cv32e40x,cv32e40px",
            nargs="?",
            default="",
            help="CPU type (default value from cfg file)",
        )

        parser.add_argument(
            "--bus",
            metavar="onetoM,NtoM",
            nargs="?",
            default="",
            help="Bus type (default value from cfg file)",
        )

        parser.add_argument(
            "--memorybanks",
            metavar="from 2 to 16",
            nargs="?",
            default="",
            help="Number of 32KB Banks (default value from cfg file)",
        )

        parser.add_argument(
            "--memorybanks_il",
            metavar="0, 2, 4 or 8",
            nargs="?",
            default="",
            help="Number of interleaved memory banks (default value from cfg file)",
        )

        parser.add_argument(
            "--external_domains",
            metavar="from 0 to 32",
            nargs="?",
            default="1",
            help="Number of external domains",
        )

        parser.add_argument(
            "-v", "--verbose", help="increase output verbosity", action="store_true"
        )

        args = parser.parse_args()
        kwargs = generate_xheep(args)

        # Create directory structure if it doesn't exist
        os.makedirs(os.path.dirname(cached_path), exist_ok=True)
        with open(cached_path, "wb") as f:
            pickle.dump(kwargs, f)


if __name__ == "__main__":
    main()
