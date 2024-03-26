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

class Pad:

  def remove_comma_io_interface(self):
    try:
        self.x_heep_system_interface = self.x_heep_system_interface.rstrip(self.x_heep_system_interface[-1])
    except IndexError:
        pass
        ### bypass kind of PADs do not have any comma to be removed as they do not define an interface

  def create_pad_ring(self):

    self.interface = '    inout wire ' + self.name + '_io,\n'

    if self.pad_type == 'input':
        self.pad_ring_io_interface = '    inout wire ' + self.io_interface + ','
        self.pad_ring_ctrl_interface += '    output logic ' + self.signal_name + 'o,'
        self.pad_ring_instance = \
            'pad_cell_input #(.PADATTR('+ str(self.attribute_bits) +')) ' + self.cell_name + ' ( \n' + \
            '   .pad_in_i(1\'b0),\n' + \
            '   .pad_oe_i(1\'b0),\n' + \
            '   .pad_out_o(' + self.signal_name + 'o),\n' + \
            '   .pad_io(' + self.signal_name + 'io),\n'
    if self.pad_type == 'output':
        self.pad_ring_io_interface = '    inout wire ' + self.io_interface + ','
        self.pad_ring_ctrl_interface += '    input logic ' + self.signal_name + 'i,'
        self.pad_ring_instance = \
            'pad_cell_output #(.PADATTR('+ str(self.attribute_bits) +')) ' + self.cell_name + ' ( \n' + \
            '   .pad_in_i(' + self.signal_name + 'i),\n' + \
            '   .pad_oe_i(1\'b1),\n' + \
            '   .pad_out_o(),\n' + \
            '   .pad_io(' + self.signal_name + 'io),\n'
    if self.pad_type == 'inout':
        self.pad_ring_io_interface = '    inout wire ' + self.io_interface + ','
        self.pad_ring_ctrl_interface += '    input logic ' + self.signal_name + 'i,\n'
        self.pad_ring_ctrl_interface += '    output logic ' + self.signal_name + 'o,\n'
        self.pad_ring_ctrl_interface += '    input logic ' + self.signal_name + 'oe_i,'
        self.pad_ring_instance = \
            'pad_cell_inout #(.PADATTR('+ str(self.attribute_bits) +')) ' + self.cell_name + ' ( \n' + \
            '   .pad_in_i(' + self.signal_name + 'i),\n' + \
            '   .pad_oe_i(' + self.signal_name + 'oe_i),\n' + \
            '   .pad_out_o(' + self.signal_name + 'o),\n' + \
            '   .pad_io(' + self.signal_name + 'io),\n'

    if self.pad_type == 'input' or self.pad_type == 'output' or self.pad_type == 'inout':
        if self.has_attribute:
            self.pad_ring_instance += \
                '   .pad_attributes_i(pad_attributes_i[core_v_mini_mcu_pkg::' + self.localparam + '])\n' + \
                ');\n\n'
        else:
            self.pad_ring_instance += \
                '   .pad_attributes_i(\'0)' + \
                ');\n\n'


  def create_core_v_mini_mcu_ctrl(self):

    cnt = len(self.pad_type_drive)

    for i in range(cnt):
        if self.driven_manually[i] == False:
            if self.pad_type_drive[i] == 'input' or self.pad_type_drive[i] == 'bypass_input':
                self.core_v_mini_mcu_interface += '    input logic ' + self.signal_name_drive[i] + 'i,\n'
            if self.pad_type_drive[i] == 'output' or self.pad_type_drive[i] == 'bypass_output':
                self.core_v_mini_mcu_interface += '    output logic ' + self.signal_name_drive[i] + 'o,\n'
            if self.pad_type_drive[i] == 'inout' or self.pad_type_drive[i] == 'bypass_inout':
                self.core_v_mini_mcu_interface += '    output logic ' + self.signal_name_drive[i] + 'o,\n'
                self.core_v_mini_mcu_interface += '    input logic ' + self.signal_name_drive[i] + 'i,\n'
                self.core_v_mini_mcu_interface += '    output logic ' + self.signal_name_drive[i] + 'oe_o,\n'

  def create_internal_signals(self):
    cnt = len(self.pad_type_drive)

    for i in range(cnt):

        self.in_internal_signals.append(self.signal_name_drive[i] + 'in_x')
        self.out_internal_signals.append(self.signal_name_drive[i] + 'out_x')
        self.oe_internal_signals.append(self.signal_name_drive[i] + 'oe_x')

        if (self.skip_declaration[i] == False):
            self.internal_signals += '  logic ' + self.in_internal_signals[i] + ',' \
                                     + self.out_internal_signals[i] + ',' \
                                     + self.oe_internal_signals[i] + ';\n'

  def create_multiplexers(self):
    cnt = len(self.pad_type_drive)

    if cnt > 1:
        ###muxing
        pad_in_internal_signals = self.signal_name + 'in_x_muxed'
        pad_out_internal_signals = self.signal_name + 'out_x_muxed'
        pad_oe_internal_signals = self.signal_name + 'oe_x_muxed'

        self.internal_signals += '  logic ' + pad_in_internal_signals + ',' \
                                 + pad_out_internal_signals + ',' \
                                 + pad_oe_internal_signals + ';\n'

        self.mux_process += '  always_comb\n' + \
                            '  begin\n'

        for i in range(cnt):
            self.mux_process += '   ' + self.in_internal_signals[i] + '=1\'b0;\n'


        self.mux_process += '   unique case(pad_muxes[core_v_mini_mcu_pkg::' + self.localparam + '])\n'

        for i in range(cnt):
            self.mux_process += '    ' + str(i) + ': begin\n' + \
                                '      ' + pad_out_internal_signals + ' = ' + self.out_internal_signals[i] + ';\n' + \
                                '      ' + pad_oe_internal_signals + ' = ' + self.oe_internal_signals[i] + ';\n' + \
                                '      ' + self.in_internal_signals[i] + ' = ' + pad_in_internal_signals + ';\n' + \
                                '    end\n'

        self.mux_process += '    default: begin\n' + \
                            '      ' + pad_out_internal_signals + ' = ' + self.out_internal_signals[0] + ';\n' + \
                            '      ' + pad_oe_internal_signals + ' = ' + self.oe_internal_signals[0] + ';\n' + \
                            '      ' + self.in_internal_signals[0] + ' = ' + pad_in_internal_signals + ';\n' + \
                            '    end\n'

        self.mux_process += '   endcase\n' + \
                            '  end\n'

  def create_constant_driver_assign(self):
    cnt = len(self.pad_type_drive)

    for i in range(cnt):

        if (self.skip_declaration[i] == False):
            if self.pad_type_drive[i] == 'input' or self.pad_type_drive[i] == 'bypass_input':
                self.constant_driver_assign += '  assign ' + self.out_internal_signals[i] + ' = 1\'b0;\n'
                self.constant_driver_assign += '  assign ' + self.oe_internal_signals[i] + ' = 1\'b0;\n'
            if self.pad_type_drive[i] == 'output' or self.pad_type_drive[i] == 'bypass_output':
                self.constant_driver_assign += '  assign ' + self.oe_internal_signals[i] + ' = 1\'b1;\n'

  def create_core_v_mini_mcu_bonding(self):

    cnt = len(self.pad_type_drive)

    for i in range(cnt):

        if self.driven_manually[i] == False:
            if self.pad_type_drive[i] == 'input' or self.pad_type_drive[i] == 'bypass_input':
                self.core_v_mini_mcu_bonding += '    .' + self.signal_name_drive[i] + 'i(' + self.in_internal_signals[i] + '),\n'
            if self.pad_type_drive[i] == 'output' or self.pad_type_drive[i] == 'bypass_output':
                self.core_v_mini_mcu_bonding += '    .' + self.signal_name_drive[i] + 'o(' + self.out_internal_signals[i] + '),\n'
            if self.pad_type_drive[i] == 'inout' or self.pad_type_drive[i] == 'bypass_inout':
                self.core_v_mini_mcu_bonding += '    .' + self.signal_name_drive[i] + 'i(' + self.in_internal_signals[i] + '),\n'
                self.core_v_mini_mcu_bonding += '    .' + self.signal_name_drive[i] + 'o(' + self.out_internal_signals[i] + '),\n'
                self.core_v_mini_mcu_bonding += '    .' + self.signal_name_drive[i] + 'oe_o(' + self.oe_internal_signals[i] + '),\n'

  def create_pad_ring_bonding(self):

    if(self.is_muxed):
        append_name = '_muxed'
    else:
        append_name = ''

    if self.pad_type == 'input':
        in_internal_signals = self.signal_name + 'in_x' + append_name
        self.pad_ring_bonding_bonding = '    .' + self.io_interface + '(' + self.signal_name + 'i),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'o(' + in_internal_signals + '),'
        self.x_heep_system_interface += '    inout wire ' + self.signal_name + 'i,'
    if self.pad_type == 'output':
        out_internal_signals = self.signal_name + 'out_x' + append_name
        self.pad_ring_bonding_bonding = '    .' + self.io_interface + '(' + self.signal_name + 'o),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'i(' + out_internal_signals + '),'
        self.x_heep_system_interface += '    inout wire ' + self.signal_name + 'o,'
    if self.pad_type == 'inout':
        in_internal_signals = self.signal_name + 'in_x' + append_name
        out_internal_signals = self.signal_name + 'out_x' + append_name
        oe_internal_signals = self.signal_name + 'oe_x' + append_name
        self.pad_ring_bonding_bonding = '    .' + self.io_interface + '(' + self.signal_name + 'io),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'o(' + in_internal_signals + '),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'i(' + out_internal_signals + '),\n'
        self.pad_ring_bonding_bonding += '    .' + self.signal_name + 'oe_i(' + oe_internal_signals + '),'
        self.x_heep_system_interface += '    inout wire ' + self.signal_name + 'io,'

  def __init__(self, name, cell_name, pad_type, index, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, has_attribute, attribute_bits):

    self.name = name
    self.cell_name = cell_name
    self.index = index
    self.localparam = 'PAD_' + name.upper()
    self.pad_type = pad_type
    self.pad_mux_list = pad_mux_list

    if('low' in pad_active):
        name_active = 'n'
    else:
        name_active = ''

    self.signal_name = self.name + '_' + name_active

    self.has_attribute = has_attribute
    self.attribute_bits = int(attribute_bits.split(":")[0]) - int(attribute_bits.split(":")[1]) + 1

    self.signal_name_drive = []
    self.pad_type_drive    = []
    self.driven_manually   = []
    self.skip_declaration  = []
    self.keep_internal     = []

    self.is_muxed = False

    self.is_driven_manually = pad_driven_manually
    self.do_skip_declaration = pad_skip_declaration

    if(len(pad_mux_list) == 0):
        self.signal_name_drive.append(self.signal_name)
        self.pad_type_drive.append(pad_type)
        self.driven_manually.append(pad_driven_manually)
        self.skip_declaration.append(pad_skip_declaration)
    else:
        for pad_mux in pad_mux_list:
            self.signal_name_drive.append(pad_mux.signal_name)
            self.pad_type_drive.append(pad_mux.pad_type)
            self.driven_manually.append(pad_mux.is_driven_manually)
            self.skip_declaration.append(pad_mux.do_skip_declaration)

        self.is_muxed = True

    self.in_internal_signals = []
    self.out_internal_signals = []
    self.oe_internal_signals = []

    self.io_interface = self.signal_name + 'io'

    ### Pad Ring ###
    self.pad_ring_io_interface = ''
    self.pad_ring_ctrl_interface = ''
    self.pad_ring_instance = ''

    ### core v mini mcu ###
    self.core_v_mini_mcu_interface = ''
    self.constant_driver_assign = ''
    self.mux_process = ''

    ### heep systems ###
    self.internal_signals = ''
    self.core_v_mini_mcu_bonding = ''
    self.pad_ring_bonding_bonding = ''
    self.x_heep_system_interface = ''

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
    parser.add_argument("--cfg",
                        "-c",
                        metavar="file",
                        type=argparse.FileType('r'),
                        required=True,
                        help="A configuration file")

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
    with args.cfg as file:
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

    if args.cpu != None and args.cpu != '':
        cpu_type = args.cpu
    else:
        cpu_type = obj['cpu_type']

    if args.bus != None and args.bus != '':
        bus_type = args.bus
    else:
        bus_type = obj['bus_type']

    if args.memorybanks != None and args.memorybanks != '':
        ram_numbanks_cont = int(args.memorybanks)
    else:
        ram_numbanks_cont = int(obj['ram']['numbanks'])

    if args.memorybanks_il != None and args.memorybanks_il != '':
        ram_numbanks_il = int(args.memorybanks_il)
    else:
        ram_numbanks_il = int(obj['ram']['numbanks_interleaved'])

    if ram_numbanks_il != 0:
        log_ram_numbanks_il = int(log2(ram_numbanks_il))

        if not log2(ram_numbanks_il).is_integer():
            exit("ram interleaved numbanks must be a power of 2 instead of " + str(ram_numbanks_il))
    else:
        log_ram_numbanks_il = 0

    if ram_numbanks_il != 0 and bus_type == 'onetoM':
        exit("bus type must be 'NtoM' instead 'onetoM' to access the interleaved memory banks in parallel" + str(args.bus))

    if ram_numbanks_cont + ram_numbanks_il < 2 and ram_numbanks_cont + ram_numbanks_il > 16:
        exit("ram numbanks must be between 2 and 16 instead of " + str(ram_numbanks_cont + ram_numbanks_il))
    else:
        ram_numbanks = ram_numbanks_cont + ram_numbanks_il

    ram_start_address = string2int(obj['ram']['address'])
    if int(ram_start_address,16) != 0:
        exit("ram start address must be 0 instead of " + str(ram_start_address))

    ram_size_address = '{:08X}'.format(ram_numbanks*32*1024)

    if args.external_domains != None and args.external_domains != '':
        external_domains = int(args.external_domains)
    else:
        external_domains = 0

    if  external_domains > 32:
        exit("external_domains must be less than 32 instead of " + str(external_domains))

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

    linker_onchip_code_start_address  = string2int(obj['linker_script']['onchip_ls']['code']['address'])
    linker_onchip_code_size_address  = string2int(obj['linker_script']['onchip_ls']['code']['lenght'])

    if int(linker_onchip_code_size_address,16) < 32*1024:
        exit("The code section must be at least 32KB, instead it is " + str(linker_onchip_code_size_address))

    linker_onchip_data_start_address  = string2int(obj['linker_script']['onchip_ls']['data']['address'])
    if (obj['linker_script']['onchip_ls']['data']['lenght'].split()[0].split(",")[0] == "whatisleft"):
        if ram_numbanks_il == 0 or (ram_numbanks_cont == 1 and ram_numbanks_il > 0):
            linker_onchip_data_size_address  = str('{:08X}'.format(int(ram_size_address,16) - int(linker_onchip_code_size_address,16)))
        else:
            linker_onchip_data_size_address  = str('{:08X}'.format(int(ram_size_address,16) - int(linker_onchip_code_size_address,16) - ram_numbanks_il*32*1024))
    else:
        if ram_numbanks_il == 0 or (ram_numbanks_cont == 1 and ram_numbanks_il > 0):
            linker_onchip_data_size_address  = string2int(obj['linker_script']['onchip_ls']['data']['lenght'])
        else:
            linker_onchip_data_size_address  = str('{:08X}'.format(int(string2int(obj['linker_script']['onchip_ls']['data']['lenght']),16) - ram_numbanks_il*32*1024))

    linker_onchip_il_start_address = str('{:08X}'.format(int(linker_onchip_data_start_address,16) + int(linker_onchip_data_size_address,16)))
    linker_onchip_il_size_address = str('{:08X}'.format(ram_numbanks_il*32*1024))

    stack_size  = string2int(obj['linker_script']['stack_size'])
    heap_size  = string2int(obj['linker_script']['heap_size'])


    linker_flash_code_start_address  = str('{:08X}'.format(int(linker_onchip_code_start_address,16) + int(flash_mem_start_address,16)))
    linker_flash_data_start_address  = str('{:08X}'.format(int(linker_onchip_data_start_address,16) + int(flash_mem_start_address,16)))
    linker_flash_il_start_address    = str('{:08X}'.format(int(linker_onchip_il_start_address,16)   + int(flash_mem_start_address,16)))

    if ram_numbanks_il == 0 or (ram_numbanks_cont == 1 and ram_numbanks_il > 0):
        linker_flash_left_start_address   = str('{:08X}'.format(int(linker_flash_data_start_address,16) + int(linker_onchip_data_size_address,16)))
        linker_flash_left_size_address    = str('{:08X}'.format(int(flash_mem_size_address,16) - int(linker_onchip_code_size_address,16) - int(linker_onchip_data_size_address,16)))
    else:
        linker_flash_left_start_address   = str('{:08X}'.format(int(linker_flash_il_start_address,16) + int(linker_onchip_il_size_address,16)))
        linker_flash_left_size_address    = str('{:08X}'.format(int(flash_mem_size_address,16) - int(linker_onchip_code_size_address,16) - int(linker_onchip_data_size_address,16) - int(linker_onchip_il_size_address,16)))


    if ((int(linker_onchip_data_size_address,16) + int(linker_onchip_code_size_address,16)) > int(ram_size_address,16)):
        exit("The code and data section must fit in the RAM size, instead they takes " + str(linker_onchip_data_size_address + linker_onchip_code_size_address))
    
    if ((int(stack_size,16) + int(heap_size,16)) > int(ram_size_address,16)):
        exit("The stack and heap section must fit in the RAM size, instead they takes " + str(stack_size + heap_size))


    plic_used_n_interrupts = len(obj['interrupts']['list'])
    plit_n_interrupts = obj['interrupts']['number']
    ext_int_list = { f"EXT_INTR_{k}": v for k, v in enumerate(range(plic_used_n_interrupts, plit_n_interrupts)) }

    interrupts = {
        **obj['interrupts']['list'],
        **ext_int_list
    }


    pads = obj_pad['pads']

    try:
        pads_attributes = obj_pad['attributes']
        pads_attributes_bits = pads_attributes['bits']
    except KeyError:
        pads_attributes = None
        pads_attributes_bits = "-1:0"

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

    pad_constant_driver_assign=''
    pad_mux_process=''

    pad_muxed_list = []

    for key in pads:

        pad_name = key
        pad_num  = pads[key]['num']
        pad_type = pads[key]['type']

        try:
            pad_offset = int(pads[key]['num_offset'])
        except KeyError:
            pad_offset = 0

        try:
            pad_active = pads[key]['active']
        except KeyError:
            pad_active = 'high'

        try:
            pad_mux_list_hjson = pads[key]['mux']
        except KeyError:
            pad_mux_list_hjson = []

        try:
            if ('True' in pads[key]['driven_manually']):
                pad_driven_manually = True
            else:
                pad_driven_manually = False
        except KeyError:
            pad_driven_manually = False

        try:
            if ('True' in pads[key]['skip_declaration']):
                pad_skip_declaration = True
            else:
                pad_skip_declaration = False
        except KeyError:
            pad_skip_declaration = False

        try:
            if ('True' in pads[key]['keep_internal']):
                pad_keep_internal = True
            else:
                pad_keep_internal = False
        except KeyError:
            pad_keep_internal = False

        pad_mux_list = []

        for pad_mux in pad_mux_list_hjson:

            try:
                pad_active_mux = pads[key]['mux'][pad_mux]['active']
            except KeyError:
                pad_active_mux = 'high'

            try:
                if ('True' in pads[key]['mux'][pad_mux]['driven_manually']):
                    pad_driven_manually_mux = True
                else:
                    pad_driven_manually_mux = False
            except KeyError:
                pad_driven_manually_mux = False

            try:
                if ('True' in pads[key]['mux'][pad_mux]['skip_declaration']):
                    pad_skip_declaration_mux = True
                else:
                    pad_skip_declaration_mux = False
            except KeyError:
                pad_skip_declaration_mux = False

            p = Pad(pad_mux, '', pads[key]['mux'][pad_mux]['type'], 0, pad_active_mux, pad_driven_manually_mux, pad_skip_declaration_mux, [], pads_attributes!=None, pads_attributes_bits)
            pad_mux_list.append(p)

        if pad_num > 1:
            for p in range(pad_num):
                pad_cell_name = "pad_" + key + "_" + str(p+pad_offset) + "_i"
                pad_obj = Pad(pad_name + "_" + str(p+pad_offset), pad_cell_name, pad_type, pad_index_counter, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits)
                if not pad_keep_internal:
                    pad_obj.create_pad_ring()
                pad_obj.create_core_v_mini_mcu_ctrl()
                if not pad_keep_internal:
                    pad_obj.create_pad_ring_bonding()
                pad_obj.create_internal_signals()
                pad_obj.create_constant_driver_assign()
                pad_obj.create_multiplexers()
                pad_obj.create_core_v_mini_mcu_bonding()
                pad_index_counter = pad_index_counter + 1
                pad_list.append(pad_obj)
                pad_constant_driver_assign+= pad_obj.constant_driver_assign
                pad_mux_process+=pad_obj.mux_process
                if (pad_obj.is_muxed):
                    pad_muxed_list.append(pad_obj)

        else:
            pad_cell_name = "pad_" + key + "_i"
            pad_obj = Pad(pad_name, pad_cell_name, pad_type, pad_index_counter, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits)
            if not pad_keep_internal:
                pad_obj.create_pad_ring()
            pad_obj.create_core_v_mini_mcu_ctrl()
            if not pad_keep_internal:
                pad_obj.create_pad_ring_bonding()
            pad_obj.create_internal_signals()
            pad_obj.create_constant_driver_assign()
            pad_obj.create_multiplexers()
            pad_obj.create_core_v_mini_mcu_bonding()
            pad_index_counter = pad_index_counter + 1
            pad_list.append(pad_obj)
            pad_constant_driver_assign+= pad_obj.constant_driver_assign
            pad_mux_process+=pad_obj.mux_process
            if (pad_obj.is_muxed):
                pad_muxed_list.append(pad_obj)

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

            try:
                pad_mux_list_hjson = external_pads[key]['mux']
            except KeyError:
                pad_mux_list_hjson = []

            try:
                if ('True' in external_pads[key]['driven_manually']):
                    pad_driven_manually = True
                else:
                    pad_driven_manually = False
            except KeyError:
                pad_driven_manually = False

            try:
                if ('True' in external_pads[key]['skip_declaration']):
                    pad_skip_declaration = True
                else:
                    pad_skip_declaration = False
            except KeyError:
                pad_skip_declaration = False

            pad_mux_list = []

            for pad_mux in pad_mux_list_hjson:

                try:
                    pad_active_mux = external_pads[key]['mux'][pad_mux]['active']
                except KeyError:
                    pad_active_mux = 'high'

                try:
                    if ('True' in external_pads[key]['mux'][pad_mux]['driven_manually']):
                        pad_driven_manually_mux = True
                    else:
                        pad_driven_manually_mux = False
                except KeyError:
                    pad_driven_manually_mux = False

                try:
                    if ('True' in external_pads[key]['mux'][pad_mux]['skip_declaration']):
                        pad_skip_declaration_mux = True
                    else:
                        pad_skip_declaration_mux = False
                except KeyError:
                    pad_skip_declaration_mux = False

                p = Pad(pad_mux, '', external_pads[key]['mux'][pad_mux]['type'], 0, pad_active_mux, pad_driven_manually_mux, pad_skip_declaration_mux, [], pads_attributes!=None, pads_attributes_bits)
                pad_mux_list.append(p)

            if pad_num > 1:
                for p in range(pad_num):
                    pad_cell_name = "pad_" + key + "_" + str(p+pad_offset) + "_i"
                    pad_obj = Pad(pad_name + "_" + str(p+pad_offset), pad_cell_name, pad_type, external_pad_index, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits)
                    pad_obj.create_pad_ring()
                    pad_obj.create_pad_ring_bonding()
                    pad_obj.create_internal_signals()
                    pad_obj.create_constant_driver_assign()
                    pad_obj.create_multiplexers()
                    external_pad_index_counter = external_pad_index_counter + 1
                    external_pad_index = external_pad_index + 1
                    external_pad_list.append(pad_obj)
                    pad_constant_driver_assign+= pad_obj.constant_driver_assign
                    pad_mux_process+=pad_obj.mux_process
                    if (pad_obj.is_muxed):
                        pad_muxed_list.append(pad_obj)

            else:
                pad_cell_name = "pad_" + key + "_i"
                pad_obj = Pad(pad_name, pad_cell_name, pad_type, external_pad_index, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits)
                pad_obj.create_pad_ring()
                pad_obj.create_pad_ring_bonding()
                pad_obj.create_internal_signals()
                pad_obj.create_constant_driver_assign()
                pad_obj.create_multiplexers()
                external_pad_index_counter = external_pad_index_counter + 1
                external_pad_index = external_pad_index + 1
                external_pad_list.append(pad_obj)
                pad_constant_driver_assign+= pad_obj.constant_driver_assign
                pad_mux_process+=pad_obj.mux_process
                if (pad_obj.is_muxed):
                    pad_muxed_list.append(pad_obj)

    total_pad_list = []

    total_pad_list = pad_list + external_pad_list

    max_total_pad_mux_bitlengh = -1
    for pad in pad_muxed_list:
        if (len(pad.pad_mux_list)-1).bit_length() > max_total_pad_mux_bitlengh:
          max_total_pad_mux_bitlengh = (len(pad.pad_mux_list)-1).bit_length()


    total_pad = pad_index_counter + external_pad_index_counter

    total_pad_muxed = len(pad_muxed_list)

    ##remove comma from last PAD io_interface
    last_pad = total_pad_list.pop()
    last_pad.remove_comma_io_interface()
    total_pad_list.append(last_pad)

    kwargs = {
        "cpu_type"                         : cpu_type,
        "bus_type"                         : bus_type,
        "ram_start_address"                : ram_start_address,
        "ram_numbanks"                     : ram_numbanks,
        "ram_numbanks_cont"                : ram_numbanks_cont,
        "ram_numbanks_il"                  : ram_numbanks_il,
        "log_ram_numbanks_il"              : log_ram_numbanks_il,
        "external_domains"                 : external_domains,
        "ram_size_address"                 : ram_size_address,
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
        "linker_flash_code_start_address"  : linker_flash_code_start_address,
        "linker_flash_data_start_address"  : linker_flash_data_start_address,
        "linker_flash_il_start_address"    : linker_flash_il_start_address,
        "linker_flash_left_start_address"  : linker_flash_left_start_address,
        "linker_flash_left_size_address"   : linker_flash_left_size_address,
        "linker_onchip_code_start_address" : linker_onchip_code_start_address,
        "linker_onchip_code_size_address"  : linker_onchip_code_size_address,
        "linker_onchip_data_start_address" : linker_onchip_data_start_address,
        "linker_onchip_data_size_address"  : linker_onchip_data_size_address,
        "linker_onchip_il_start_address"   : linker_onchip_il_start_address,
        "linker_onchip_il_size_address"    : linker_onchip_il_size_address,
        "stack_size"                       : stack_size,
        "heap_size"                        : heap_size,
        "plic_used_n_interrupts"           : plic_used_n_interrupts,
        "plit_n_interrupts"                : plit_n_interrupts,
        "interrupts"                       : interrupts,
        "pad_list"                         : pad_list,
        "external_pad_list"                : external_pad_list,
        "total_pad_list"                   : total_pad_list,
        "total_pad"                        : total_pad,
        "pad_constant_driver_assign"       : pad_constant_driver_assign,
        "pad_mux_process"                  : pad_mux_process,
        "pad_muxed_list"                   : pad_muxed_list,
        "total_pad_muxed"                  : total_pad_muxed,
        "max_total_pad_mux_bitlengh"       : max_total_pad_mux_bitlengh,
        "pads_attributes"                  : pads_attributes,
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
