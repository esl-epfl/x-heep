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


# Compile a regex to trim trailing whitespaces on lines.
re_trailws = re.compile(r'[ \t\r]+$', re.MULTILINE)

def string2int(hex_json_string):
    return (hex_json_string.split('x')[1]).split(',')[0]


def write_template(tpl_path, outdir, **kwargs):
    if tpl_path:
        tpl_path = pathlib.Path(tpl_path).absolute()
        if tpl_path.exists():
            tpl = Template(filename=str(tpl_path))
            with open(outdir / tpl_path.with_suffix("").name, "w") as file:
                code = tpl.render_unicode(**kwargs)
                code = re_trailws.sub("", code)
                file.write(code)
        else:
            raise FileNotFoundError


def main():
    parser = argparse.ArgumentParser(prog="mcugen")
    parser.add_argument("--cfg",
                        "-c",
                        metavar="file",
                        type=argparse.FileType('r'),
                        required=True,
                        help="A configuration file")

    parser.add_argument("--outdir",
                        "-o",
                        type=pathlib.Path,
                        required=True,
                        help="Target directory.")
    # Parse arguments.

    parser.add_argument("--pkg-sv",
                        metavar="PKG_SV",
                        help="Name of top-level package file (output)")

    parser.add_argument("--header-c",
                        metavar="HEADER_C",
                        help="Name of header file (output)")

    parser.add_argument("-v",
                        "--verbose",
                        help="increase output verbosity",
                        action="store_true")

    args = parser.parse_args()

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)

    # Read HJSON description of System.
    with args.cfg as file:
        try:
            srcfull = file.read()
            obj = hjson.loads(srcfull, use_decimal=True)
            obj = JsonRef.replace_refs(obj)
        except ValueError:
            raise SystemExit(sys.exc_info()[1])

    if not args.outdir.is_dir():
            exit("Out directory is not a valid path.")

    outdir = args.outdir
    outdir.mkdir(parents=True, exist_ok=True)


    cpu_type = obj['cpu_type']

    bus_type = obj['bus_type']

    debug_start_address = string2int(obj['debug']['address'])
    if int(debug_start_address, 16) < int('10000', 16):
        exit("debug start address must be greater than 0x10000")

    debug_size_address = string2int(obj['debug']['length'])

    ext_slave_start_address = string2int(obj['ext_slaves']['address'])
    ext_slave_size_address = string2int(obj['ext_slaves']['length'])

    peripheral_start_address = string2int(obj['peripherals']['address'])
    if int(peripheral_start_address, 16) < int('10000', 16):
        exit("peripheral start address must be greater than 0x10000")

    peripheral_size_address = string2int(obj['peripherals']['length'])

    uart_start_offset  = string2int(obj['peripherals']['uart']['offset'])
    uart_size_address  = string2int(obj['peripherals']['uart']['length'])

    soc_ctrl_start_offset  = string2int(obj['peripherals']['soc_ctrl']['offset'])
    soc_ctrl_size_address  = string2int(obj['peripherals']['soc_ctrl']['length'])

    ext_periph_start_offset  = string2int(obj['peripherals']['ext_periph']['offset'])
    ext_periph_size_address  = string2int(obj['peripherals']['ext_periph']['length'])

    plic_start_offset  = string2int(obj['peripherals']['plic']['offset'])
    plic_size_address  = string2int(obj['peripherals']['plic']['length'])

    null_intr = obj['interrupts']['null_intr']
    uart_intr_tx_watermark = obj['interrupts']['uart_intr_tx_watermark']
    uart_intr_rx_watermark = obj['interrupts']['uart_intr_rx_watermark']
    uart_intr_tx_empty = obj['interrupts']['uart_intr_tx_empty']
    uart_intr_rx_overflow = obj['interrupts']['uart_intr_rx_overflow']
    uart_intr_rx_frame_err = obj['interrupts']['uart_intr_rx_frame_err']
    uart_intr_rx_break_err = obj['interrupts']['uart_intr_rx_break_err']
    uart_intr_rx_timeout = obj['interrupts']['uart_intr_rx_timeout']
    uart_intr_rx_parity_err = obj['interrupts']['uart_intr_rx_parity_err']
    memcopy_intr_done = obj['interrupts']['memcopy_intr_done']

    rv_timer_start_offset  = string2int(obj['peripherals']['rv_timer']['offset'])
    rv_timer_size_address  = string2int(obj['peripherals']['rv_timer']['length'])

    kwargs = {
        "cpu_type"                 : cpu_type,
        "bus_type"                 : bus_type,
        "debug_start_address"      : debug_start_address,
        "debug_size_address"       : debug_size_address,
        "peripheral_start_address" : peripheral_start_address,
        "peripheral_size_address"  : peripheral_size_address,
        "ext_slave_start_address"  : ext_slave_start_address,
        "ext_slave_size_address"   : ext_slave_size_address,
        "uart_start_offset"        : uart_start_offset,
        "uart_size_address"        : uart_size_address,
        "soc_ctrl_start_offset"    : soc_ctrl_start_offset,
        "soc_ctrl_size_address"    : soc_ctrl_size_address,
        "ext_periph_start_offset"  : ext_periph_start_offset,
        "ext_periph_size_address"  : ext_periph_size_address,
        "plic_start_offset"        : plic_start_offset,
        "plic_size_address"        : plic_size_address,
        "null_intr"                : null_intr,
        "uart_intr_tx_watermark"   : uart_intr_tx_watermark,
        "uart_intr_rx_watermark"   : uart_intr_rx_watermark,
        "uart_intr_tx_empty"       : uart_intr_tx_empty,
        "uart_intr_rx_overflow"    : uart_intr_rx_overflow,
        "uart_intr_rx_frame_err"   : uart_intr_rx_frame_err,
        "uart_intr_rx_break_err"   : uart_intr_rx_break_err,
        "uart_intr_rx_timeout"     : uart_intr_rx_timeout,
        "uart_intr_rx_parity_err"  : uart_intr_rx_parity_err,
        "memcopy_intr_done"        : memcopy_intr_done,
        "rv_timer_start_offset"    : rv_timer_start_offset,
        "rv_timer_size_address"    : rv_timer_size_address,
    }

    ###########
    # Package #
    ###########
    if args.pkg_sv != None:
        write_template(args.pkg_sv, outdir, **kwargs)

    if args.header_c != None:
        write_template(args.header_c, outdir, **kwargs)


if __name__ == "__main__":
    main()
