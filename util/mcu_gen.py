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

class Pad:


  def create_pad_ring(self):

    self.pad_ring_io_interface = '    inout logic ' + self.io_interface + ','
    self.pad_ring_ctrl_interface = ''
    if self.pad_type == 'input':
        self.pad_ring_ctrl_interface += '    output logic ' + self.signal_name + 'o,'
        self.pad_ring_instance = \
            'pad_cell_input #(.PADATTR(16)) ' + self.cell_name + ' ( \n' + \
            '   .pad_in_i(1\'b0),\n' + \
            '   .pad_oe_i(1\'b0),\n' + \
            '   .pad_out_o(' + self.signal_name + 'o),\n' + \
            '   .pad_io(' + self.signal_name + 'io),\n' + \
            '   .pad_attributes_i(pad_attributes_i[core_v_mini_mcu_pkg::' + self.localparam + '])\n' + \
            ');\n\n'
    if self.pad_type == 'output':
        self.pad_ring_ctrl_interface += '    input logic ' + self.signal_name + 'i,'
        self.pad_ring_instance = \
            'pad_cell_output #(.PADATTR(16)) ' + self.cell_name + ' ( \n' + \
            '   .pad_in_i(' + self.signal_name + 'i),\n' + \
            '   .pad_oe_i(1\'b1),\n' + \
            '   .pad_out_o(),\n' + \
            '   .pad_io(' + self.signal_name + 'io),\n' + \
            '   .pad_attributes_i(pad_attributes_i[core_v_mini_mcu_pkg::' + self.localparam + '])\n' + \
            ');\n\n'
    if self.pad_type == 'inout':
        self.pad_ring_ctrl_interface += '    input logic ' + self.signal_name + 'i,\n'
        self.pad_ring_ctrl_interface += '    output logic ' + self.signal_name + 'o,\n'
        self.pad_ring_ctrl_interface += '    input logic ' + self.signal_name + 'oe_i,'
        self.pad_ring_instance = \
            'pad_cell_inout #(.PADATTR(16)) ' + self.cell_name + ' ( \n' + \
            '   .pad_in_i(' + self.signal_name + 'i),\n' + \
            '   .pad_oe_i(' + self.signal_name + 'oe_i),\n' + \
            '   .pad_out_o(' + self.signal_name + 'o),\n' + \
            '   .pad_io(' + self.signal_name + 'io),\n' + \
            '   .pad_attributes_i(pad_attributes_i[core_v_mini_mcu_pkg::' + self.localparam + '])\n' + \
            ');\n\n'


  def create_core_v_mini_mcu_ctrl(self):

    if self.pad_type == 'input':
        self.core_v_mini_mcu_interface += '    input logic ' + self.signal_name + 'i,'
    if self.pad_type == 'output':
        self.core_v_mini_mcu_interface += '    output logic ' + self.signal_name + 'o,'
    if self.pad_type == 'inout':
        self.core_v_mini_mcu_interface += '    output logic ' + self.signal_name + 'o,'
        self.core_v_mini_mcu_interface += '    input logic ' + self.signal_name + 'i,'
        self.core_v_mini_mcu_interface += '    output logic ' + self.signal_name + 'oe_o,'
    if self.pad_type == 'bypass_output':
        self.core_v_mini_mcu_interface += '    output logic ' + self.signal_name + 'o,\n'
    if self.pad_type == 'bypass_input':
        self.core_v_mini_mcu_interface += '    input logic ' + self.signal_name + 'i,\n'

  def create_core_v_mini_mcu_bonding(self):
    if self.pad_type == 'input':
        in_internal_signals = self.signal_name + 'in_x'
        self.core_v_mini_mcu_bonding = '    .' + self.signal_name + 'i(' + in_internal_signals + '),'
    if self.pad_type == 'output':
        out_internal_signals = self.signal_name + 'out_x'
        self.core_v_mini_mcu_bonding = '    .' + self.signal_name + 'o(' + out_internal_signals + '),'
    if self.pad_type == 'inout':
        in_internal_signals = self.signal_name + 'in_x'
        out_internal_signals = self.signal_name + 'out_x'
        oe_internal_signals = self.signal_name + 'oe_x'
        self.core_v_mini_mcu_bonding = '    .' + self.signal_name + 'i(' + in_internal_signals + '),\n'
        self.core_v_mini_mcu_bonding += '    .' + self.signal_name + 'o(' + out_internal_signals + '),\n'
        self.core_v_mini_mcu_bonding += '    .' + self.signal_name + 'oe_o(' + oe_internal_signals + '),'
    if self.pad_type == 'bypass_output':
        out_internal_signals = self.signal_name + 'out_x'
        self.core_v_mini_mcu_bonding = '.' + self.signal_name + 'o(' + out_internal_signals + '),'
    if self.pad_type == 'bypass_input':
        in_internal_signals = self.signal_name + 'in_x'
        self.core_v_mini_mcu_bonding = '.' + self.signal_name + 'i(' + in_internal_signals + '),'

  def create_pad_ring_bonding(self):

    if self.pad_type == 'input':
        in_internal_signals = self.signal_name + 'in_x'
        self.internal_signals = '  logic ' + in_internal_signals + ';'
        self.pad_ring_bonding_bonding = '    .' + self.io_interface + '(' + self.signal_name + 'i),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'o(' + in_internal_signals + '),'
        self.x_heep_system_interface += '    inout logic ' + self.signal_name + 'i,'
    if self.pad_type == 'output':
        out_internal_signals = self.signal_name + 'out_x'
        self.internal_signals = '  logic ' + out_internal_signals + ';'
        self.pad_ring_bonding_bonding = '    .' + self.io_interface + '(' + self.signal_name + 'o),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'i(' + out_internal_signals + '),'
        self.x_heep_system_interface += '    inout logic ' + self.signal_name + 'o,'
    if self.pad_type == 'inout':
        in_internal_signals = self.signal_name + 'in_x'
        out_internal_signals = self.signal_name + 'out_x'
        oe_internal_signals = self.signal_name + 'oe_x'
        self.internal_signals += '  logic ' + in_internal_signals + ',' \
                                 + out_internal_signals + ',' \
                                 + oe_internal_signals + ';'

        self.pad_ring_bonding_bonding = '    .' + self.io_interface + '(' + self.signal_name + 'io),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'o(' + in_internal_signals + '),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'i(' + out_internal_signals + '),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'oe_i(' + oe_internal_signals + '),'
        self.x_heep_system_interface += '    inout logic ' + self.signal_name + 'io,'


  def __init__(self, name, cell_name, pad_type, index, pad_active):


    self.name = name
    self.cell_name = cell_name
    self.index = index
    self.localparam = 'PAD_' + name.upper()
    self.pad_type = pad_type

    if('low' in pad_active):
        self.name_active = 'n'
    else:
        self.name_active = ''

    self.signal_name = self.name + '_' + self.name_active

    self.io_interface = self.signal_name + 'io'

    ### Pad Ring ###
    self.pad_ring_io_interface = ''
    self.pad_ring_ctrl_interface = ''
    self.pad_ring_instance = ''

    ### core v mini mcu ###

    self.core_v_mini_mcu_interface = ''


    ### heep systems ###
    self.internal_signals = ''
    self.core_v_mini_mcu_bonding = ''
    self.pad_ring_bonding_bonding = ''
    self.x_heep_system_interface = ''


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

    parser.add_argument("--cpu",
                        metavar="cv32e20,cv32e40p",
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
                        help="Name of the hjson file contaiting extra pads")

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

    if args.cpu != None and args.cpu != '':
        cpu_type = args.cpu
    else:
        cpu_type = obj['cpu_type']

    if args.bus != None and args.bus != '':
        bus_type = args.bus
    else:
        bus_type = obj['bus_type']

    if args.memorybanks != None and args.memorybanks != '':
        ram_numbanks = int(args.memorybanks)
    else:
        ram_numbanks = int(obj['ram']['numbanks'])

    if ram_numbanks < 2 and ram_numbanks > 16:
        exit("ram numbanks must be between 2 and 16 instead of " + str(ram_numbanks))

    ram_start_address = string2int(obj['ram']['address'])
    if int(ram_start_address,16) != 0:
        exit("ram start address must be 0 instead of " + str(ram_start_address))

    ram_size_address = '{:08X}'.format(ram_numbanks*32*1024)

    debug_start_address = string2int(obj['debug']['address'])
    if int(debug_start_address, 16) < int('10000', 16):
        exit("debug start address must be greater than 0x10000")

    debug_size_address = string2int(obj['debug']['length'])

    ao_peripheral_start_address = string2int(obj['ao_peripherals']['address'])
    if int(ao_peripheral_start_address, 16) < int('10000', 16):
        exit("always on peripheral start address must be greater than 0x10000")

    ao_peripheral_size_address = string2int(obj['ao_peripherals']['length'])

    soc_ctrl_start_offset  = string2int(obj['ao_peripherals']['soc_ctrl']['offset'])
    soc_ctrl_size_address  = string2int(obj['ao_peripherals']['soc_ctrl']['length'])

    bootrom_start_offset  = string2int(obj['ao_peripherals']['bootrom']['offset'])
    bootrom_size_address  = string2int(obj['ao_peripherals']['bootrom']['length'])

    spi_flash_start_offset  = string2int(obj['ao_peripherals']['spi_flash']['offset'])
    spi_flash_size_address  = string2int(obj['ao_peripherals']['spi_flash']['length'])

    spi_memio_start_offset  = string2int(obj['ao_peripherals']['spi_memio']['offset'])
    spi_memio_size_address  = string2int(obj['ao_peripherals']['spi_memio']['length'])

    power_manager_start_offset  = string2int(obj['ao_peripherals']['power_manager']['offset'])
    power_manager_size_address  = string2int(obj['ao_peripherals']['power_manager']['length'])

    rv_timer_start_offset  = string2int(obj['ao_peripherals']['rv_timer']['offset'])
    rv_timer_size_address  = string2int(obj['ao_peripherals']['rv_timer']['length'])

    dma_start_offset  = string2int(obj['ao_peripherals']['dma']['offset'])
    dma_size_address  = string2int(obj['ao_peripherals']['dma']['length'])

    pad_attribute_start_offset  = string2int(obj['ao_peripherals']['pad']['offset'])
    pad_attribute_size_address  = string2int(obj['ao_peripherals']['pad']['length'])

    spi_start_offset  = string2int(obj['ao_peripherals']['spi']['offset'])
    spi_size_address  = string2int(obj['ao_peripherals']['spi']['length'])

    peripheral_start_address = string2int(obj['peripherals']['address'])
    if int(peripheral_start_address, 16) < int('10000', 16):
        exit("peripheral start address must be greater than 0x10000")

    peripheral_size_address = string2int(obj['peripherals']['length'])

    plic_start_offset  = string2int(obj['peripherals']['plic']['offset'])
    plic_size_address  = string2int(obj['peripherals']['plic']['length'])

    uart_start_offset  = string2int(obj['peripherals']['uart']['offset'])
    uart_size_address  = string2int(obj['peripherals']['uart']['length'])

    gpio_start_offset  = string2int(obj['peripherals']['gpio']['offset'])
    gpio_size_address  = string2int(obj['peripherals']['gpio']['length'])

    i2c_start_offset  = string2int(obj['peripherals']['i2c']['offset'])
    i2c_size_address  = string2int(obj['peripherals']['i2c']['length'])

    ext_periph_start_offset  = string2int(obj['peripherals']['ext_periph']['offset'])
    ext_periph_size_address  = string2int(obj['peripherals']['ext_periph']['length'])

    ext_slave_start_address = string2int(obj['ext_slaves']['address'])
    ext_slave_size_address = string2int(obj['ext_slaves']['length'])

    flash_mem_start_address  = string2int(obj['flash_mem']['address'])
    flash_mem_size_address  = string2int(obj['flash_mem']['length'])

    linker_onchip_code_start_address  = string2int(obj['linker_script']['onchip_ls']['code']['address'])
    linker_onchip_code_size_address  = string2int(obj['linker_script']['onchip_ls']['code']['lenght'])

    if int(linker_onchip_code_size_address,16) < 32*1024:
        exit("The code section must be at least 32KB, instead it is " + str(linker_onchip_code_size_address))

    linker_onchip_data_start_address  = string2int(obj['linker_script']['onchip_ls']['data']['address'])
    if (obj['linker_script']['onchip_ls']['data']['lenght'].split()[0].split(",")[0] == "whatisleft"):
        linker_onchip_data_size_address  = str('{:08X}'.format(int(ram_size_address,16) - int(linker_onchip_code_size_address,16)))
    else:
        linker_onchip_data_size_address  = string2int(obj['linker_script']['onchip_ls']['data']['lenght'])

    if ((int(linker_onchip_data_size_address,16) + int(linker_onchip_code_size_address,16)) > int(ram_size_address,16)):
        exit("The code and data section must fit in the RAM size, instead they takes " + str(linker_onchip_data_size_address + linker_onchip_code_size_address))


    null_intr = obj['interrupts']['null_intr']
    uart_intr_tx_watermark = obj['interrupts']['uart_intr_tx_watermark']
    uart_intr_rx_watermark = obj['interrupts']['uart_intr_rx_watermark']
    uart_intr_tx_empty = obj['interrupts']['uart_intr_tx_empty']
    uart_intr_rx_overflow = obj['interrupts']['uart_intr_rx_overflow']
    uart_intr_rx_frame_err = obj['interrupts']['uart_intr_rx_frame_err']
    uart_intr_rx_break_err = obj['interrupts']['uart_intr_rx_break_err']
    uart_intr_rx_timeout = obj['interrupts']['uart_intr_rx_timeout']
    uart_intr_rx_parity_err = obj['interrupts']['uart_intr_rx_parity_err']
    gpio_intr_0 = obj['interrupts']['gpio_intr_0']
    gpio_intr_1 = obj['interrupts']['gpio_intr_1']
    gpio_intr_2 = obj['interrupts']['gpio_intr_2']
    gpio_intr_3 = obj['interrupts']['gpio_intr_3']
    gpio_intr_4 = obj['interrupts']['gpio_intr_4']
    gpio_intr_5 = obj['interrupts']['gpio_intr_5']
    gpio_intr_6 = obj['interrupts']['gpio_intr_6']
    gpio_intr_7 = obj['interrupts']['gpio_intr_7']
    gpio_intr_8 = obj['interrupts']['gpio_intr_8']
    gpio_intr_9 = obj['interrupts']['gpio_intr_9']
    gpio_intr_10 = obj['interrupts']['gpio_intr_10']
    gpio_intr_11 = obj['interrupts']['gpio_intr_11']
    gpio_intr_12 = obj['interrupts']['gpio_intr_12']
    gpio_intr_13 = obj['interrupts']['gpio_intr_13']
    gpio_intr_14 = obj['interrupts']['gpio_intr_14']
    gpio_intr_15 = obj['interrupts']['gpio_intr_15']
    gpio_intr_16 = obj['interrupts']['gpio_intr_16']
    gpio_intr_17 = obj['interrupts']['gpio_intr_17']
    gpio_intr_18 = obj['interrupts']['gpio_intr_18']
    gpio_intr_19 = obj['interrupts']['gpio_intr_19']
    gpio_intr_20 = obj['interrupts']['gpio_intr_20']
    gpio_intr_21 = obj['interrupts']['gpio_intr_21']
    gpio_intr_22 = obj['interrupts']['gpio_intr_22']
    gpio_intr_23 = obj['interrupts']['gpio_intr_23']
    gpio_intr_24 = obj['interrupts']['gpio_intr_24']
    gpio_intr_25 = obj['interrupts']['gpio_intr_25']
    gpio_intr_26 = obj['interrupts']['gpio_intr_26']
    gpio_intr_27 = obj['interrupts']['gpio_intr_27']
    gpio_intr_28 = obj['interrupts']['gpio_intr_28']
    gpio_intr_29 = obj['interrupts']['gpio_intr_29']
    gpio_intr_30 = obj['interrupts']['gpio_intr_30']
    gpio_intr_31 = obj['interrupts']['gpio_intr_31']
    intr_fmt_watermark = obj['interrupts']['intr_fmt_watermark']
    intr_rx_watermark = obj['interrupts']['intr_rx_watermark']
    intr_fmt_overflow = obj['interrupts']['intr_fmt_overflow']
    intr_rx_overflow = obj['interrupts']['intr_rx_overflow']
    intr_nak = obj['interrupts']['intr_nak']
    intr_scl_interference = obj['interrupts']['intr_scl_interference']
    intr_sda_interference = obj['interrupts']['intr_sda_interference']
    intr_stretch_timeout = obj['interrupts']['intr_stretch_timeout']
    intr_sda_unstable = obj['interrupts']['intr_sda_unstable']
    intr_trans_complete = obj['interrupts']['intr_trans_complete']
    intr_tx_empty = obj['interrupts']['intr_tx_empty']
    intr_tx_nonempty = obj['interrupts']['intr_tx_nonempty']
    intr_tx_overflow = obj['interrupts']['intr_tx_overflow']
    intr_acq_overflow = obj['interrupts']['intr_acq_overflow']
    intr_ack_stop = obj['interrupts']['intr_ack_stop']
    intr_host_timeout = obj['interrupts']['intr_host_timeout']
    dma_intr_done = obj['interrupts']['dma_intr_done']
    spi_flash_intr_error = obj['interrupts']['spi_flash_intr_error']
    spi_flash_intr_event = obj['interrupts']['spi_flash_intr_event']
    spi_intr_error = obj['interrupts']['spi_intr_error']
    spi_intr_event = obj['interrupts']['spi_intr_event']
    # Interrupt lines available for external interrupt sources
    ext_intr_0 = obj['interrupts']['ext_intr_0']
    ext_intr_1 = obj['interrupts']['ext_intr_1']

    pads       = obj['pads']

    # Read HJSON description of External Pads
    if args.external_pads != None:
        with args.external_pads as file_external_pads:
            try:
                srcfull = file_external_pads.read()
                ext_pads_obj = hjson.loads(srcfull, use_decimal=True)
                ext_pads_obj = JsonRef.replace_refs(ext_pads_obj)
            except ValueError:
                raise SystemExit(sys.exc_info()[1])

        external_pads = ext_pads_obj['pads']
    else:
        external_pads = None


    pad_list   = []
    pad_index_counter = 0
    external_pad_list   = []
    external_pad_index_counter = 0

    for key in pads:

        pad_name = key
        pad_num = pads[key]['num']
        pad_type = pads[key]['type']
        try:
            pad_offset = int(pads[key]['num_offset'])
        except KeyError:
            pad_offset = 0
        try:
            pad_active = pads[key]['active']
        except KeyError:
            pad_active = 'high'



        if pad_num > 1:
            for p in range(pad_num):
                pad_cell_name = "pad_" + key + "_" + str(p+pad_offset) + "_i"
                pad_obj = Pad(pad_name + "_" + str(p+pad_offset), pad_cell_name, pad_type, pad_index_counter, pad_active)
                pad_obj.create_pad_ring()
                pad_obj.create_core_v_mini_mcu_ctrl()
                pad_obj.create_pad_ring_bonding()
                pad_obj.create_core_v_mini_mcu_bonding()
                pad_index_counter = pad_index_counter + 1
                pad_list.append(pad_obj)
        else:
            pad_cell_name = "pad_" + key + "_i"
            pad_obj = Pad(pad_name, pad_cell_name, pad_type, pad_index_counter, pad_active)
            pad_obj.create_pad_ring()
            pad_obj.create_core_v_mini_mcu_ctrl()
            pad_obj.create_pad_ring_bonding()
            pad_obj.create_core_v_mini_mcu_bonding()
            pad_index_counter = pad_index_counter + 1
            pad_list.append(pad_obj)

    if external_pads != None:
        external_pad_index_counter = 0
        external_pad_index = pad_index_counter
        for key in external_pads:
            pad_name = key
            pad_num = external_pads[key]['num']
            pad_type = external_pads[key]['type']
            try:
                pad_offset = int(external_pads[key]['num_offset'])
            except KeyError:
                pad_offset = 0
            try:
                pad_active = external_pads[key]['active']
            except KeyError:
                pad_active = 'high'



            if pad_num > 1:
                for p in range(pad_num):
                    pad_cell_name = "pad_" + key + "_" + str(p+pad_offset) + "_i"
                    pad_obj = Pad(pad_name + "_" + str(p+pad_offset), pad_cell_name, pad_type, external_pad_index, pad_active)
                    pad_obj.create_pad_ring()
                    pad_obj.create_pad_ring_bonding()
                    external_pad_index_counter = external_pad_index_counter + 1
                    external_pad_index = external_pad_index + 1
                    external_pad_list.append(pad_obj)
            else:
                pad_cell_name = "pad_" + key + "_i"
                pad_obj = Pad(pad_name, pad_cell_name, pad_type, external_pad_index, pad_active)
                pad_obj.create_pad_ring()
                pad_obj.create_pad_ring_bonding()
                external_pad_index_counter = external_pad_index_counter + 1
                external_pad_index = external_pad_index + 1
                external_pad_list.append(pad_obj)


    total_pad_list = []

    total_pad_list = pad_list + external_pad_list

    total_pad = pad_index_counter + external_pad_index_counter

    kwargs = {
        "cpu_type"                         : cpu_type,
        "bus_type"                         : bus_type,
        "ram_start_address"                : ram_start_address,
        "ram_numbanks"                     : ram_numbanks,
        "ram_size_address"                 : ram_size_address,
        "debug_start_address"              : debug_start_address,
        "debug_size_address"               : debug_size_address,
        "ao_peripheral_start_address"      : ao_peripheral_start_address,
        "ao_peripheral_size_address"       : ao_peripheral_size_address,
        "soc_ctrl_start_offset"            : soc_ctrl_start_offset,
        "soc_ctrl_size_address"            : soc_ctrl_size_address,
        "bootrom_start_offset"             : bootrom_start_offset,
        "bootrom_size_address"             : bootrom_size_address,
        "spi_flash_start_offset"           : spi_flash_start_offset,
        "spi_flash_size_address"           : spi_flash_size_address,
        "spi_memio_start_offset"           : spi_memio_start_offset,
        "spi_memio_size_address"           : spi_memio_size_address,
        "power_manager_start_offset"       : power_manager_start_offset,
        "power_manager_size_address"       : power_manager_size_address,
        "rv_timer_start_offset"            : rv_timer_start_offset,
        "rv_timer_size_address"            : rv_timer_size_address,
        "dma_start_offset"                 : dma_start_offset,
        "dma_size_address"                 : dma_size_address,
        "pad_attribute_start_offset"       : pad_attribute_start_offset,
        "pad_attribute_size_address"       : pad_attribute_size_address,
        "spi_start_offset"                 : spi_start_offset,
        "spi_size_address"                 : spi_size_address,
        "peripheral_start_address"         : peripheral_start_address,
        "peripheral_size_address"          : peripheral_size_address,
        "plic_start_offset"                : plic_start_offset,
        "plic_size_address"                : plic_size_address,
        "uart_start_offset"                : uart_start_offset,
        "uart_size_address"                : uart_size_address,
        "gpio_start_offset"                : gpio_start_offset,
        "gpio_size_address"                : gpio_size_address,
        "i2c_start_offset"                 : i2c_start_offset,
        "i2c_size_address"                 : i2c_size_address,
        "ext_periph_start_offset"          : ext_periph_start_offset,
        "ext_periph_size_address"          : ext_periph_size_address,
        "ext_slave_start_address"          : ext_slave_start_address,
        "ext_slave_size_address"           : ext_slave_size_address,
        "flash_mem_start_address"          : flash_mem_start_address,
        "flash_mem_size_address"           : flash_mem_size_address,
        "linker_onchip_code_start_address" : linker_onchip_code_start_address,
        "linker_onchip_code_size_address"  : linker_onchip_code_size_address,
        "linker_onchip_data_start_address" : linker_onchip_data_start_address,
        "linker_onchip_data_size_address"  : linker_onchip_data_size_address,
        "null_intr"                        : null_intr,
        "uart_intr_tx_watermark"           : uart_intr_tx_watermark,
        "uart_intr_rx_watermark"           : uart_intr_rx_watermark,
        "uart_intr_tx_empty"               : uart_intr_tx_empty,
        "uart_intr_rx_overflow"            : uart_intr_rx_overflow,
        "uart_intr_rx_frame_err"           : uart_intr_rx_frame_err,
        "uart_intr_rx_break_err"           : uart_intr_rx_break_err,
        "uart_intr_rx_timeout"             : uart_intr_rx_timeout,
        "uart_intr_rx_parity_err"          : uart_intr_rx_parity_err,
        "gpio_intr_0"                      : gpio_intr_0,
        "gpio_intr_1"                      : gpio_intr_1,
        "gpio_intr_2"                      : gpio_intr_2,
        "gpio_intr_3"                      : gpio_intr_3,
        "gpio_intr_4"                      : gpio_intr_4,
        "gpio_intr_5"                      : gpio_intr_5,
        "gpio_intr_6"                      : gpio_intr_6,
        "gpio_intr_7"                      : gpio_intr_7,
        "gpio_intr_8"                      : gpio_intr_8,
        "gpio_intr_9"                      : gpio_intr_9,
        "gpio_intr_10"                     : gpio_intr_10,
        "gpio_intr_11"                     : gpio_intr_11,
        "gpio_intr_12"                     : gpio_intr_12,
        "gpio_intr_13"                     : gpio_intr_13,
        "gpio_intr_14"                     : gpio_intr_14,
        "gpio_intr_15"                     : gpio_intr_15,
        "gpio_intr_16"                     : gpio_intr_16,
        "gpio_intr_17"                     : gpio_intr_17,
        "gpio_intr_18"                     : gpio_intr_18,
        "gpio_intr_19"                     : gpio_intr_19,
        "gpio_intr_20"                     : gpio_intr_20,
        "gpio_intr_21"                     : gpio_intr_21,
        "gpio_intr_22"                     : gpio_intr_22,
        "gpio_intr_23"                     : gpio_intr_23,
        "gpio_intr_24"                     : gpio_intr_24,
        "gpio_intr_25"                     : gpio_intr_25,
        "gpio_intr_26"                     : gpio_intr_26,
        "gpio_intr_27"                     : gpio_intr_27,
        "gpio_intr_28"                     : gpio_intr_28,
        "gpio_intr_29"                     : gpio_intr_29,
        "gpio_intr_30"                     : gpio_intr_30,
        "gpio_intr_31"                     : gpio_intr_31,
        "intr_fmt_watermark"               : intr_fmt_watermark,
        "intr_rx_watermark"                : intr_rx_watermark,
        "intr_fmt_overflow"                : intr_fmt_overflow,
        "intr_rx_overflow"                 : intr_rx_overflow,
        "intr_nak"                         : intr_nak,
        "intr_scl_interference"            : intr_scl_interference,
        "intr_sda_interference"            : intr_sda_interference,
        "intr_stretch_timeout"             : intr_stretch_timeout,
        "intr_sda_unstable"                : intr_sda_unstable,
        "intr_trans_complete"              : intr_trans_complete,
        "intr_tx_empty"                    : intr_tx_empty,
        "intr_tx_nonempty"                 : intr_tx_nonempty,
        "intr_tx_overflow"                 : intr_tx_overflow,
        "intr_acq_overflow"                : intr_acq_overflow,
        "intr_ack_stop"                    : intr_ack_stop,
        "intr_host_timeout"                : intr_host_timeout,
        "dma_intr_done"                    : dma_intr_done,
        "spi_flash_intr_error"             : spi_flash_intr_error,
        "spi_flash_intr_event"             : spi_flash_intr_event,
        "spi_intr_error"                   : spi_intr_error,
        "spi_intr_event"                   : spi_intr_event,
        "ext_intr_0"                       : ext_intr_0,
        "ext_intr_1"                       : ext_intr_1,
        "pad_list"                         : pad_list,
        "external_pad_list"                : external_pad_list,
        "total_pad_list"                   : total_pad_list,
        "total_pad"                        : total_pad,
    }

    ###########
    # Package #
    ###########
    if args.pkg_sv != None:
        write_template(args.pkg_sv, outdir, **kwargs)

    if args.header_c != None:
        write_template(args.header_c, outdir, **kwargs)

    if args.tpl_sv != None:
        write_template(args.tpl_sv, outdir, **kwargs)

    if args.linker_script != None:
        write_template(args.linker_script, outdir, **kwargs)


if __name__ == "__main__":
    main()
