#!/usr/bin/env python3

# Copyright 2020 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

#Simplified version of occamygen.py https://github.com/pulp-platform/snitch/blob/master/util/occamygen.py

import argparse
import pickle
import hjson
import pathlib
import sys
import logging
from jsonref import JsonRef
import x_heep_gen.load_config
from x_heep_gen.system import BusType


def string2int(hex_json_string):
    return (hex_json_string.split('x')[1]).split(',')[0]


def main():
    parser = argparse.ArgumentParser(prog="mk_cfg")
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

    parser.add_argument("--outfile",
                        "-o",
                        type=pathlib.Path,
                        required=True,
                        help="config object file")

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

    ext_slave_start_address = string2int(obj['ext_slaves']['address'])
    ext_slave_size_address = string2int(obj['ext_slaves']['length'])

    flash_mem_start_address  = string2int(obj['flash_mem']['address'])
    flash_mem_size_address  = string2int(obj['flash_mem']['length'])

    stack_size  = string2int(obj['linker_script']['stack_size'])
    heap_size  = string2int(obj['linker_script']['heap_size'])


    if ((int(stack_size,16) + int(heap_size,16)) > xheep.ram_size_address()):
        exit("The stack and heap section must fit in the RAM size, instead they takes " + str(stack_size + heap_size))


    kwargs = {
        "xheep"                            : xheep,
        "cpu_type"                         : cpu_type,
        "external_domains"                 : external_domains,
        "debug_start_address"              : debug_start_address,
        "debug_size_address"               : debug_size_address,
        "ext_slave_start_address"          : ext_slave_start_address,
        "ext_slave_size_address"           : ext_slave_size_address,
        "flash_mem_start_address"          : flash_mem_start_address,
        "flash_mem_size_address"           : flash_mem_size_address,
        "stack_size"                       : stack_size,
        "heap_size"                        : heap_size,
    }

    with open(outfile, "wb") as f:
        pickle.dump(kwargs, f)

if __name__ == "__main__":
    main()
