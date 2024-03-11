#!/usr/bin/env python3

# Copyright 2020 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

#Simplified version of occamygen.py https://github.com/pulp-platform/snitch/blob/master/util/occamygen.py

import argparse
import hjson
import pathlib
import sys
import re
import logging
from subprocess import run
import csv
from jsonref import JsonRef
from mako.template import Template
import collections
from math import log2
import x_heep_gen.load_config
from x_heep_gen.system import BusType

# Compile a regex to trim trailing whitespaces on lines.
re_trailws = re.compile(r'[ \t\r]+$', re.MULTILINE)

def string2int(hex_json_string):
    return (hex_json_string.split('x')[1]).split(',')[0]

def write_template(tpl_path, outdir, outfile, **kwargs):
    if tpl_path:
        tpl_path = pathlib.Path(tpl_path).absolute()
        if tpl_path.exists():
            tpl = Template(filename=str(tpl_path))
            if outfile == None:
                filename = outdir / tpl_path.with_suffix("").name
            else:
                filename = outfile
            with open(filename, "w") as file:
                code = tpl.render_unicode(**kwargs)
                code = re_trailws.sub("", code)
                file.write(code)
        else:
            raise FileNotFoundError

def main():
    parser = argparse.ArgumentParser(prog="mcugen")
    parser.add_argument("--cfg_peripherals",
                        "-c",
                        metavar="file",
                        type=argparse.FileType('r'),
                        required=True,
                        help="A configuration file")
    
    parser.add_argument("--config",
                        metavar="file",
                        type=str,
                        required=True,
                        help="X-Heep general configuration")

    parser.add_argument("--pads_cfg",
                        "-pc",
                        metavar="file",
                        type=argparse.FileType('r'),
                        required=True,
                        help="A pad configuration file")

    parser.add_argument("--outdir",
                        "-of",
                        type=pathlib.Path,
                        required=True,
                        help="Target directory.")

    parser.add_argument("--outfile",
                        "-o",
                        type=pathlib.Path,
                        required=False,
                        help="Target filename, if omitted the template basename is taken.")

    # Parse arguments.

    parser.add_argument("--cpu",
                        metavar="cv32e20,cv32e40p,cv32e40x,cv32e40px",
                        nargs='?',
                        default="",
                        help="CPU type (default value from cfg file)")

    parser.add_argument("--bus",
                        metavar="onetoM,NtoM",
                        nargs='?',
                        default="",
                        help="Bus type (default value from cfg file)")

    parser.add_argument("--memorybanks",
                        metavar="from 2 to 16",
                        nargs='?',
                        default="",
                        help="Number of 32KB Banks (default value from cfg file)")

    parser.add_argument("--memorybanks_il",
                        metavar="0, 2, 4 or 8",
                        nargs='?',
                        default="",
                        help="Number of interleaved memory banks (default value from cfg file)")

    parser.add_argument("--external_domains",
                        metavar="from 0 to 32",
                        nargs='?',
                        default="1",
                        help="Number of external domains")

    parser.add_argument("--pkg-sv",
                        metavar="PKG_SV",
                        help="Name of top-level package file (output)")

    parser.add_argument("--tpl-sv",
                        metavar="TPL_SV",
                        help="Name of SystemVerilog template for your module (output)")


    parser.add_argument("--header-c",
                        metavar="HEADER_C",
                        help="Name of header file (output)")

    parser.add_argument("--linker_script",
                        metavar="LINKER_SCRIPT",
                        help="Name of the linker script (output)")

    parser.add_argument("--external_pads",
                        metavar="file",
                        type=argparse.FileType('r'),
                        required=False,
                        nargs='?',
                        default=None,
                        const=None,
                        help="Name of the hjson file contaiting extra pads")

    parser.add_argument("-v",
                        "--verbose",
                        help="increase output verbosity",
                        action="store_true")

    args = parser.parse_args()

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)

    # Read HJSON description of System.
    with args.cfg_peripherals as file:
        try:
            srcfull = file.read()
            obj = hjson.loads(srcfull, use_decimal=True)
            obj = JsonRef.replace_refs(obj)
        except ValueError:
            raise SystemExit(sys.exc_info()[1])

    with args.pads_cfg as file:
        try:
            srcfull = file.read()
            obj_pad = hjson.loads(srcfull, use_decimal=True)
            obj_pad = JsonRef.replace_refs(obj_pad)
        except ValueError:
            raise SystemExit(sys.exc_info()[1])

    if not args.outdir.is_dir():
            exit("Out directory is not a valid path.")

    outdir = args.outdir
    outdir.mkdir(parents=True, exist_ok=True)

    outfile = args.outfile

    config_override = x_heep_gen.system.Override(None, None, None)

    if args.cpu != None and args.cpu != '':
        cpu_type = args.cpu
    else:
        cpu_type = obj['cpu_type']

    if args.bus != None and args.bus != '':
        config_override.bus_type = BusType(args.bus)

    if args.memorybanks != None and args.memorybanks != '':
        config_override.numbanks = int(args.memorybanks)

    if args.memorybanks_il != None and args.memorybanks_il != '':
        config_override.numbanks_il = int(args.memorybanks_il)

    if args.external_domains != None and args.external_domains != '':
        external_domains = int(args.external_domains)
    else:
        external_domains = 0

    if  external_domains > 32:
        exit("external_domains must be less than 32 instead of " + str(external_domains))



    xheep = x_heep_gen.load_config.load_cfg_file(pathlib.PurePath(str(args.config)), config_override)



    debug_start_address = string2int(obj['debug']['address'])
    if int(debug_start_address, 16) < int('10000', 16):
        exit("debug start address must be greater than 0x10000")

    debug_size_address = string2int(obj['debug']['length'])

    ao_peripheral_start_address = string2int(obj['ao_peripherals']['address'])
    if int(ao_peripheral_start_address, 16) < int('10000', 16):
        exit("always on peripheral start address must be greater than 0x10000")

    ao_peripheral_size_address = string2int(obj['ao_peripherals']['length'])


    def extract_peripherals(peripherals):
        result = {}
        for name, info in peripherals.items():
            if isinstance(info, dict):
                new_info = {}
                for k, v in info.items():
                    if k not in ("is_included"):
                        new_info[k] = string2int(v)
                    else:
                        new_info[k] = v
                result[name] = new_info

        return result


    def discard_path(peripherals):
        new = {}
        for k,v in peripherals.items():
            if isinstance(v, dict):
                new[k] = {key:val for key,val in v.items() if key not in ("path")}
            else:
                new[k] = v
        return new

    def len_extracted_peripherals(peripherals):
        len_ep = 0
        for name, info in peripherals.items():
            if isinstance(info, dict):
                for k, v in info.items():
                   if k in ("is_included"):
                    if v in ("yes"):
                        len_ep += 1
        return len_ep

    ao_peripherals = extract_peripherals(discard_path(obj['ao_peripherals']))
    ao_peripherals_count = len(ao_peripherals)


    peripheral_start_address = string2int(obj['peripherals']['address'])
    if int(peripheral_start_address, 16) < int('10000', 16):
        exit("peripheral start address must be greater than 0x10000")

    peripheral_size_address = string2int(obj['peripherals']['length'])
    peripherals = extract_peripherals(discard_path(obj['peripherals']))
    peripherals_count = len(peripherals)

    ext_slave_start_address = string2int(obj['ext_slaves']['address'])
    ext_slave_size_address = string2int(obj['ext_slaves']['length'])

    flash_mem_start_address  = string2int(obj['flash_mem']['address'])
    flash_mem_size_address  = string2int(obj['flash_mem']['length'])

    stack_size  = string2int(obj['linker_script']['stack_size'])
    heap_size  = string2int(obj['linker_script']['heap_size'])


    if ((int(stack_size,16) + int(heap_size,16)) > xheep.ram_size_address()):
        exit("The stack and heap section must fit in the RAM size, instead they takes " + str(stack_size + heap_size))


    plic_used_n_interrupts = len(obj['interrupts']['list'])
    plit_n_interrupts = obj['interrupts']['number']
    ext_int_list = { f"EXT_INTR_{k}": v for k, v in enumerate(range(plic_used_n_interrupts, plit_n_interrupts)) }

    interrupts = {
        **obj['interrupts']['list'],
        **ext_int_list
    }

    kwargs = {
        "xheep"                            : xheep,
        "cpu_type"                         : cpu_type,
        "external_domains"                 : external_domains,
        "debug_start_address"              : debug_start_address,
        "debug_size_address"               : debug_size_address,
        "ao_peripheral_start_address"      : ao_peripheral_start_address,
        "ao_peripheral_size_address"       : ao_peripheral_size_address,
        "ao_peripherals"                   : ao_peripherals,
        "ao_peripherals_count"             : ao_peripherals_count,
        "peripheral_start_address"         : peripheral_start_address,
        "peripheral_size_address"          : peripheral_size_address,
        "peripherals"                      : peripherals,
        "peripherals_count"                : peripherals_count,
        "ext_slave_start_address"          : ext_slave_start_address,
        "ext_slave_size_address"           : ext_slave_size_address,
        "flash_mem_start_address"          : flash_mem_start_address,
        "flash_mem_size_address"           : flash_mem_size_address,
        "stack_size"                       : stack_size,
        "heap_size"                        : heap_size,
        "plic_used_n_interrupts"           : plic_used_n_interrupts,
        "plit_n_interrupts"                : plit_n_interrupts,
        "interrupts"                       : interrupts,
    }

    ###########
    # Package #
    ###########
    if args.pkg_sv != None:
        write_template(args.pkg_sv, outdir, outfile, **kwargs)

    if args.header_c != None:
        write_template(args.header_c, outdir, outfile, **kwargs)

    if args.tpl_sv != None:
        write_template(args.tpl_sv, outdir, outfile, **kwargs)

    if args.linker_script != None:
        write_template(args.linker_script, outdir, outfile, **kwargs)

if __name__ == "__main__":
    main()
