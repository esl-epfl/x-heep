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
import math

class Pad:

  def remove_comma_io_interface(self):
    try:
        self.x_heep_system_interface = self.x_heep_system_interface.rstrip(self.x_heep_system_interface[-1])
    except IndexError:
        pass
        ### bypass kind of PADs do not have any comma to be removed as they do not define an interface

  def create_pad_ring(self):

    # Mapping dictionary from string to integer
    mapping_dict = {
        'top' : 'core_v_mini_mcu_pkg::TOP',
        'right' : 'core_v_mini_mcu_pkg::RIGHT',
        'bottom' : 'core_v_mini_mcu_pkg::BOTTOM',
        'left' : 'core_v_mini_mcu_pkg::LEFT'
    }

    mapping = ''
    if self.pad_mapping is not None:
        mapping = ', .SIDE(' + mapping_dict[self.pad_mapping] + ')'

    self.interface = '    inout wire ' + self.name + '_io,\n'

    if self.pad_type == 'input':
        self.pad_ring_io_interface = '    inout wire ' + self.io_interface + ','
        self.pad_ring_ctrl_interface += '    output logic ' + self.signal_name + 'o,'
        self.pad_ring_instance = \
            'pad_cell_input #(.PADATTR('+ str(self.attribute_bits) +')' + mapping + ') ' + self.cell_name + ' ( \n' + \
            '   .pad_in_i(1\'b0),\n' + \
            '   .pad_oe_i(1\'b0),\n' + \
            '   .pad_out_o(' + self.signal_name + 'o),\n' + \
            '   .pad_io(' + self.signal_name + 'io),\n'
    if self.pad_type == 'output':
        self.pad_ring_io_interface = '    inout wire ' + self.io_interface + ','
        self.pad_ring_ctrl_interface += '    input logic ' + self.signal_name + 'i,'
        self.pad_ring_instance = \
            'pad_cell_output #(.PADATTR('+ str(self.attribute_bits) +')' + mapping + ') ' + self.cell_name + ' ( \n' + \
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
            'pad_cell_inout #(.PADATTR('+ str(self.attribute_bits) +')' + mapping + ') ' + self.cell_name + ' ( \n' + \
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

  def __init__(self, name, cell_name, pad_type, pad_mapping, index, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, has_attribute, attribute_bits, pad_layout_index, pad_layout_orient, pad_layout_cell, pad_layout_bondpad, pad_layout_offset, pad_layout_skip):

    self.name = name
    self.cell_name = cell_name
    self.index = index
    self.localparam = 'PAD_' + name.upper()
    self.pad_type = pad_type
    self.pad_mapping = pad_mapping
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

    self.layout_index = pad_layout_index
    self.layout_orient = pad_layout_orient
    self.layout_cell = pad_layout_cell
    self.layout_bondpad = pad_layout_bondpad
    self.layout_offset = pad_layout_offset
    self.layout_skip = pad_layout_skip

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

def prepare_pads_for_layout(total_pad_list, physical_attributes):
    """
    Separate pads into pad lists for the top, bottom, left, and right pads and order them according to their layout_index attribute, and set their positions on the floorplan.
    """

    # Separate pads according to side
    top_pad_list = []
    bottom_pad_list = []
    right_pad_list = []
    left_pad_list = []
    for pad in total_pad_list:
        if (pad.pad_mapping == "top"):
            top_pad_list.append(pad)
        elif (pad.pad_mapping == "bottom"):
            bottom_pad_list.append(pad)
        elif (pad.pad_mapping == "right"):
            right_pad_list.append(pad)
        elif (pad.pad_mapping == "left"):
            left_pad_list.append(pad)
    
    # Order pads according to layout index
    top_pad_list.sort(key=lambda x: x.layout_index)
    bottom_pad_list.sort(key=lambda x: x.layout_index)
    left_pad_list.sort(key=lambda x: x.layout_index)
    right_pad_list.sort(key=lambda x: x.layout_index)

    # Calculate pad offsets and check whether requested pad configuration fits in the floorplan
    top_pad_list, bondpad_offset_top = set_pad_positions(top_pad_list, physical_attributes)
    bottom_pad_list, bondpad_offset_bottom = set_pad_positions(bottom_pad_list, physical_attributes)
    left_pad_list, bondpad_offset_left = set_pad_positions(left_pad_list, physical_attributes)
    right_pad_list, bondpad_offset_right = set_pad_positions(right_pad_list, physical_attributes)

    bondpad_offsets = {
        "top": bondpad_offset_top,
        "bottom": bondpad_offset_bottom,
        "left": bondpad_offset_left,
        "right": bondpad_offset_right
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
        print("ERROR: Please set all of the mandatory fields of the physical_attributes in the pad config file.")
        return

    # Determine which dimension we are dealing with
    side = pad_list[0].pad_mapping
    if ((side == "top") | (side == "bottom")):
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
                print("ERROR: Width not defined for bondpad cell {0} of pad {1}".format(bp_cell, pad.cell_name))
                return
        else:
            print("ERROR: A bondpad cell is not defined for pad {1}".format(pad.cell_name))
            return
        bp_space += bp_width
    bp_space += bp_spacing*(len(pad_list)-1)

    # Check if the bondpads are able to fit on the side
    extra_space = side_length - bp_space - 2*edge_to_bp
    if (extra_space<0):
        print("ERROR: Bondpads cannot fit on side {0}. Either reduce bondpad spacing or move some pads to another side".format(side))

    # Calculate distance from edge to first bondpad (i.e. bondpad offset) to center the pads
    bp_offset = extra_space/2
    
    # Calculate skip parameter between one pad and the next to center the pads
    for i, pad in enumerate(pad_list):

        # Get bondpad width from physical attributes
        bp_cell = pad.layout_bondpad
        bp_width = float(pad_dims[bp_cell]["width"])

        if (i>0):
            last_bp_cell = pad_list[i-1].layout_bondpad
            last_bp_width = float(pad_dims[last_bp_cell]["width"])

        # Get pad width from physical attributes
        pad_cell = pad.layout_cell
        if pad_cell is not None:
            try: 
                pad_width = float(pad_dims[pad_cell]["width"])
            except KeyError:
                print("ERROR: Width not defined for pad cell {0} of pad {1}".format(pad_cell, pad.cell_name))
                return
        else:
            print("ERROR: A pad cell is not defined for pad {1}".format(pad.cell_name))
            return

        if (i>0):
            last_pad_cell = pad_list[i-1].layout_cell
            if pad_cell is not None:
                try: 
                    last_pad_width = float(pad_dims[last_pad_cell]["width"])
                except KeyError:
                    print("ERROR: Width not defined for pad cell {0} of pad {1}".format(last_pad_cell, pad_list[i-1].cell_name))
                    return
            else:
                print("ERROR: A pad cell is not defined for pad {1}".format(pad_list[i-1].cell_name))
                return
        if (i==0)&(pad.layout_offset is None)& (pad.layout_skip is None):
            pad.layout_offset = bp_offset - (edge_to_pad - edge_to_bp) + (bp_width/2) - (pad_width/2)
        
        # If the layout/skip of the pads is not predefined, calculate automatically
        if (pad.layout_offset is None) & (pad.layout_skip is None):
            pad.layout_skip = (last_bp_width + bp_width)/2 + bp_spacing - (last_pad_width + pad_width)/2
    
    return pad_list, bp_offset


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

    if 'cve2_rv32e' in obj:
        cve2_rv32e = obj['cve2_rv32e']
    else:
        cve2_rv32e = None
    
    if 'cve2_rv32m' in obj:
        cve2_rv32m = obj['cve2_rv32m']
    else:
        cve2_rv32m = None

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
    dma_ch_count = ao_peripherals["dma"]["num_channels"]

    if int(dma_ch_count, 16) > int('256', 16) or int(dma_ch_count, 16) == 0:
        exit("Number of DMA channels has to be between 0 and 256, excluded")

    dma_ch_size = ao_peripherals["dma"]["ch_length"]

    # Number of master ports for the dma subsystem
    num_dma_master_ports = ao_peripherals["dma"]["num_master_ports"]
    if int(num_dma_master_ports, 16) > int(dma_ch_count, 16) or int(num_dma_master_ports, 16) == 0:
        exit("Number of DMA master ports has to be between 0 and " + str(dma_ch_count) + ", 0 excluded")

    # Number of masters for each slave of the DMA NtoM xbar
    num_dma_xbar_channels_per_master_port = ao_peripherals["dma"]["num_channels_per_master_port"]
    if (int(num_dma_xbar_channels_per_master_port, 16) > int(dma_ch_count, 16) and int(dma_ch_count, 16) != 1) or int(num_dma_xbar_channels_per_master_port, 16) == 0:
        exit("Number of DMA channels per system bus master ports has to be between 0 and " + str(dma_ch_count) + ", excluded")

    if (int(num_dma_master_ports) > 1):
        # Computation of full_masters_xbars
        temp_full_masters_xbars = math.floor(int(dma_ch_count) / int(num_dma_xbar_channels_per_master_port))
        if temp_full_masters_xbars < int(num_dma_master_ports) and temp_full_masters_xbars * int(num_dma_xbar_channels_per_master_port) == int(dma_ch_count):
            full_masters_xbars = temp_full_masters_xbars - 1
        else:
            full_masters_xbars = temp_full_masters_xbars
        last = int(num_dma_xbar_channels_per_master_port) * full_masters_xbars

        # Array initialization
        array_xbar_gen = [0] * int(num_dma_master_ports)

        # Computation of the number of xbar channels for each master port
        for i in range(int(num_dma_master_ports)):
            if i < full_masters_xbars:
                array_xbar_gen[i] = int(num_dma_xbar_channels_per_master_port)
            else:
                array_xbar_gen[i] = min(int(dma_ch_count) - last, int(dma_ch_count) - last - (int(num_dma_master_ports) - i - 1))
                last = last + array_xbar_gen[i]

        if (sum(array_xbar_gen) != int(dma_ch_count) or 0 in array_xbar_gen):
            exit("Error in the DMA xbar generation: wrong parameters")
        
        dma_xbar_array = ", ".join(map(str, array_xbar_gen))
    else:
        if (int(num_dma_xbar_channels_per_master_port) != int(dma_ch_count)):
            exit("With 1 master port, the number of DMA channels per master port has to be equal to the number of DMA channels")
        dma_xbar_array = "default: 1"

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
        pad_type = pads[key]['type'].strip(',')

        try:
            pad_offset = int(pads[key]['num_offset'])
        except KeyError:
            pad_offset = 0

        try:
            pad_active = pads[key]['active']
        except KeyError:
            pad_active = 'high'
        
        try:
            pad_mapping = pads[key]['mapping'].strip(',')
        except KeyError:
            pad_mapping = None

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

        try:
            pad_layout_orient = pads[key]['layout_attributes']['orient']
        except KeyError:
            pad_layout_orient = None
        
        try:
            pad_layout_cell = pads[key]['layout_attributes']['cell']
        except KeyError:
            pad_layout_cell = None
        
        try:
            pad_layout_bondpad = pads[key]['layout_attributes']['bondpad']
        except KeyError:
            pad_layout_bondpad = None

        try:
            pad_layout_offset = pads[key]['layout_attributes']['offset']
        except KeyError:
            pad_layout_offset = None

        try:
            pad_layout_skip = pads[key]['layout_attributes']['skip']
        except KeyError:
            pad_layout_skip = None

        try:
            pad_layout_index = pads[key]['layout_attributes']['index']
        except KeyError:
            pad_layout_index = None

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

            p = Pad(pad_mux, '', pads[key]['mux'][pad_mux]['type'], pad_mapping, 0, pad_active_mux, pad_driven_manually_mux, pad_skip_declaration_mux, [], pads_attributes!=None, pads_attributes_bits, pad_layout_index, pad_layout_orient, pad_layout_cell, pad_layout_bondpad, pad_layout_offset, pad_layout_skip)
            pad_mux_list.append(p)

        if pad_num > 1:
            for p in range(pad_num):
                pad_cell_name = "pad_" + key + "_" + str(p+pad_offset) + "_i"
                pad_obj = Pad(pad_name + "_" + str(p+pad_offset), pad_cell_name, pad_type, pad_mapping, pad_index_counter, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits, pad_layout_index, pad_layout_orient, pad_layout_cell, pad_layout_bondpad, pad_layout_offset, pad_layout_skip)
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
            pad_obj = Pad(pad_name, pad_cell_name, pad_type, pad_mapping, pad_index_counter, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits, pad_layout_index, pad_layout_orient, pad_layout_cell, pad_layout_bondpad, pad_layout_offset, pad_layout_skip)
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
                pad_mapping = external_pads[key]['mapping']
            except KeyError:
                pad_mapping = None

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

            try:
                pad_layout_orient = external_pads[key]['layout_attributes']['orient']
            except KeyError:
                pad_layout_orient = None
            
            try:
                pad_layout_cell = external_pads[key]['layout_attributes']['cell']
            except KeyError:
                pad_layout_cell = None

            try:
                pad_layout_bondpad = external_pads[key]['layout_attributes']['bondpad']
            except KeyError:
                pad_layout_bondpad = None
            
            try:
                pad_layout_offset = external_pads[key]['layout_attributes']['offset']
            except KeyError:
                pad_layout_offset = None

            try:
                pad_layout_skip = external_pads[key]['layout_attributes']['skip']
            except KeyError:
                pad_layout_skip = None
            
            try:
                pad_layout_index = external_pads[key]['layout_attributes']['index']
            except KeyError:
                pad_layout_index = None
            
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

                p = Pad(pad_mux, '', external_pads[key]['mux'][pad_mux]['type'], pad_mapping, 0, pad_active_mux, pad_driven_manually_mux, pad_skip_declaration_mux, [], pads_attributes!=None, pads_attributes_bits, pad_layout_index, pad_layout_orient, pad_layout_cell, pad_layout_bondpad, pad_layout_offset, pad_layout_skip)
                pad_mux_list.append(p)

            if pad_num > 1:
                for p in range(pad_num):
                    pad_cell_name = "pad_" + key + "_" + str(p+pad_offset) + "_i"
                    pad_obj = Pad(pad_name + "_" + str(p+pad_offset), pad_cell_name, pad_type, pad_mapping, external_pad_index, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits, pad_layout_index, pad_layout_orient, pad_layout_cell, pad_layout_bondpad, pad_layout_offset, pad_layout_skip)
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
                pad_obj = Pad(pad_name, pad_cell_name, pad_type, pad_mapping, external_pad_index, pad_active, pad_driven_manually, pad_skip_declaration, pad_mux_list, pads_attributes!=None, pads_attributes_bits, pad_layout_index, pad_layout_orient, pad_layout_cell, pad_layout_bondpad, pad_layout_offset, pad_layout_skip)
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

    # If layout parameters exist in the pad config file, compute the pad offset/skip parameters and order the pads on each side
    try:
        physical_attributes = obj_pad['physical_attributes']
        top_pad_list, bottom_pad_list, left_pad_list, right_pad_list, bondpad_offsets = prepare_pads_for_layout(total_pad_list, physical_attributes)
    except KeyError:
        physical_attributes = None
        top_pad_list = None
        bottom_pad_list = None
        left_pad_list = None
        right_pad_list = None
        bondpad_offsets = None

    kwargs = {
        "xheep"                            : xheep,
        "cpu_type"                         : cpu_type,
        "cve2_rv32e"                       : cve2_rv32e,
        "cve2_rv32m"                       : cve2_rv32m,
        "external_domains"                 : external_domains,
        "debug_start_address"              : debug_start_address,
        "debug_size_address"               : debug_size_address,
        "ao_peripheral_start_address"      : ao_peripheral_start_address,
        "ao_peripheral_size_address"       : ao_peripheral_size_address,
        "ao_peripherals"                   : ao_peripherals,
        "ao_peripherals_count"             : ao_peripherals_count,
        "dma_ch_count"                     : dma_ch_count,
        "dma_ch_size"                      : dma_ch_size,
        "num_dma_master_ports"             : num_dma_master_ports,
        "num_dma_xbar_channels_per_master_port" : num_dma_xbar_channels_per_master_port,
        "dma_xbar_masters_array"           : dma_xbar_array,
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
        "pad_list"                         : pad_list,
        "external_pad_list"                : external_pad_list,
        "total_pad_list"                   : total_pad_list,
        "total_pad"                        : total_pad,
        "right_pad_list"                   : right_pad_list,
        "left_pad_list"                    : left_pad_list,
        "top_pad_list"                     : top_pad_list,
        "bottom_pad_list"                  : bottom_pad_list,
        "physical_attributes"              : physical_attributes,
        "bondpad_offsets"                  : bondpad_offsets,
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
