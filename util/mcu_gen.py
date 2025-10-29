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
from x_heep_gen.pad_ring import PadRing
import x_heep_gen.peripherals.base_peripherals
import x_heep_gen.peripherals.user_peripherals
import x_heep_gen.peripherals.abstractions
from x_heep_gen.pads import Pad,PadMapping
import math
import os
from x_heep_gen.arg_utils import as_bool, get_nested, coerce_enum



# Compile a regex to trim trailing whitespaces on lines.
re_trailws = re.compile(r"[ \t\r]+$", re.MULTILINE)


def string2int(hex_json_string):
    return (hex_json_string.split("x")[1]).split(",")[0]


def write_template(tpl_path, outfile, **kwargs):
    if tpl_path:
        tpl_path = pathlib.Path(tpl_path).absolute()
        if tpl_path.exists():
            tpl = Template(filename=str(tpl_path))
            print("Generating file from template: {0}".format(tpl_path))
            print("Template inside =", tpl.source)

            if outfile:
                filename = outfile
            else:
                filename = tpl_path.with_suffix("")

            with open(filename, "w") as file:
                code = tpl.render_unicode(**kwargs,strict_undefined=True)
                code = re_trailws.sub("", code)
                file.write(code)
        else:
            raise FileNotFoundError("Template file not found: {0}".format(tpl_path))
    else:
        raise FileNotFoundError("Template file not provided")




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

    padring = PadRing(pad_cfg,config)
    


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
        "pad_list": padring.pad_list,
        "external_pad_list": padring.external_pad_list,
        "total_pad_list": padring.total_pad_list,
        "total_pad": padring.total_pad,
        "right_pad_list": padring.right_pad_list,
        "bottom_pad_list": padring.bottom_pad_list,
        "physical_attributes": padring.physical_attributes,
        "bondpad_offsets": padring.bondpad_offsets,
        "pad_constant_driver_assign": padring.pad_constant_driver_assign,
        "pad_mux_process": padring.pad_mux_process,
        "pad_muxed_list": padring.pad_muxed_list,
        "total_pad_muxed": padring.total_pad_muxed,
        "max_total_pad_mux_bitlengh": padring.max_total_pad_mux_bitlengh,
        "pads_attributes": padring.pads_attributes,
        "padring": padring,
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
