from enum import Enum


class PadMapping(Enum):
    TOP = "top"
    RIGHT = "right"
    BOTTOM = "bottom"
    LEFT = "left"


class Pad:

    def remove_comma_io_interface(self):
        s = self.x_heep_system_interface.rstrip()
        if s.endswith(","):
            self.x_heep_system_interface = s[:-1]
        else:
            self.x_heep_system_interface = s

    def create_pad_ring(self):
        # Mapping dictionary (unchanged)
        mapping_dict = {
            PadMapping.TOP: "core_v_mini_mcu_pkg::TOP",
            PadMapping.RIGHT: "core_v_mini_mcu_pkg::RIGHT",
            PadMapping.BOTTOM: "core_v_mini_mcu_pkg::BOTTOM",
            PadMapping.LEFT: "core_v_mini_mcu_pkg::LEFT",
        }

        # Build ", .SIDE(...)" exactly like before
        mapping = (
            f", .SIDE({mapping_dict[self.pad_mapping]})" if self.pad_mapping else ""
        )

        # Top-level interface
        self.interface = f"    inout wire {self.name}_io,\n"

        # Parameter string (keeps same parenthesis position)
        param_str = f"#(.PADATTR({self.attribute_bits}){mapping})"
        sig = self.signal_name

        # --- Pad type logic ---
        if self.pad_type == "input":
            self.pad_ring_io_interface = f"    inout wire {self.io_interface},"
            self.pad_ring_ctrl_interface += f"    output logic {sig}o,"
            conns = (
                "\n".join(
                    [
                        "   .pad_in_i(1'b0),",
                        "   .pad_oe_i(1'b0),",
                        f"   .pad_out_o({sig}o),",
                        f"   .pad_io({sig}io),",
                    ]
                )
                + "\n"
            )
            cell = "pad_cell_input"

        elif self.pad_type == "output":
            self.pad_ring_io_interface = f"    inout wire {self.io_interface},"
            self.pad_ring_ctrl_interface += f"    input logic {sig}i,"
            conns = (
                "\n".join(
                    [
                        f"   .pad_in_i({sig}i),",
                        "   .pad_oe_i(1'b1),",
                        "   .pad_out_o(),",
                        f"   .pad_io({sig}io),",
                    ]
                )
                + "\n"
            )
            cell = "pad_cell_output"

        elif self.pad_type == "inout":
            self.pad_ring_io_interface = f"    inout wire {self.io_interface},"
            self.pad_ring_ctrl_interface += (
                f"    input logic {sig}i,\n"
                f"    output logic {sig}o,\n"
                f"    input logic {sig}oe_i,"
            )
            conns = (
                "\n".join(
                    [
                        f"   .pad_in_i({sig}i),",
                        f"   .pad_oe_i({sig}oe_i),",
                        f"   .pad_out_o({sig}o),",
                        f"   .pad_io({sig}io),",
                    ]
                )
                + "\n"
            )
            cell = "pad_cell_inout"

        # --- Instance construction (identical layout) ---
        if self.pad_type in ("input", "output", "inout"):
            header = f"{cell} {param_str} {self.cell_name} ( \n{conns}"
            if self.has_attribute:
                attr_line = (
                    f"   .pad_attributes_i(pad_attributes_i[core_v_mini_mcu_pkg::{self.localparam}])\n"
                    ");\n\n"
                )
            else:
                attr_line = "   .pad_attributes_i('0)" + ");\n\n"
            self.pad_ring_instance = header + attr_line

    def create_core_v_mini_mcu_ctrl(self):

        cnt = len(self.pad_type_drive)

        for i in range(cnt):
            if self.driven_manually[i] == False:
                if (
                    self.pad_type_drive[i] == "input"
                    or self.pad_type_drive[i] == "bypass_input"
                ):
                    self.core_v_mini_mcu_interface += (
                        "    input logic " + self.signal_name_drive[i] + "i,\n"
                    )
                if (
                    self.pad_type_drive[i] == "output"
                    or self.pad_type_drive[i] == "bypass_output"
                ):
                    self.core_v_mini_mcu_interface += (
                        "    output logic " + self.signal_name_drive[i] + "o,\n"
                    )
                if (
                    self.pad_type_drive[i] == "inout"
                    or self.pad_type_drive[i] == "bypass_inout"
                ):
                    self.core_v_mini_mcu_interface += (
                        "    output logic " + self.signal_name_drive[i] + "o,\n"
                    )
                    self.core_v_mini_mcu_interface += (
                        "    input logic " + self.signal_name_drive[i] + "i,\n"
                    )
                    self.core_v_mini_mcu_interface += (
                        "    output logic " + self.signal_name_drive[i] + "oe_o,\n"
                    )

    def create_internal_signals(self):
        cnt = len(self.pad_type_drive)

        for i in range(cnt):

            self.in_internal_signals.append(self.signal_name_drive[i] + "in_x")
            self.out_internal_signals.append(self.signal_name_drive[i] + "out_x")
            self.oe_internal_signals.append(self.signal_name_drive[i] + "oe_x")

            if self.skip_declaration[i] == False:
                self.internal_signals += (
                    "  logic "
                    + self.in_internal_signals[i]
                    + ","
                    + self.out_internal_signals[i]
                    + ","
                    + self.oe_internal_signals[i]
                    + ";\n"
                )

    def create_multiplexers(self):
        cnt = len(self.pad_type_drive)

        if cnt > 1:
            ###muxing
            pad_in_internal_signals = self.signal_name + "in_x_muxed"
            pad_out_internal_signals = self.signal_name + "out_x_muxed"
            pad_oe_internal_signals = self.signal_name + "oe_x_muxed"

            self.internal_signals += (
                "  logic "
                + pad_in_internal_signals
                + ","
                + pad_out_internal_signals
                + ","
                + pad_oe_internal_signals
                + ";\n"
            )

            self.mux_process += "  always_comb\n" + "  begin\n"

            for i in range(cnt):
                self.mux_process += "   " + self.in_internal_signals[i] + "=1'b0;\n"

            self.mux_process += (
                "   unique case(pad_muxes[core_v_mini_mcu_pkg::"
                + self.localparam
                + "])\n"
            )

            for i in range(cnt):
                self.mux_process += (
                    "    "
                    + str(i)
                    + ": begin\n"
                    + "      "
                    + pad_out_internal_signals
                    + " = "
                    + self.out_internal_signals[i]
                    + ";\n"
                    + "      "
                    + pad_oe_internal_signals
                    + " = "
                    + self.oe_internal_signals[i]
                    + ";\n"
                    + "      "
                    + self.in_internal_signals[i]
                    + " = "
                    + pad_in_internal_signals
                    + ";\n"
                    + "    end\n"
                )

            self.mux_process += (
                "    default: begin\n"
                + "      "
                + pad_out_internal_signals
                + " = "
                + self.out_internal_signals[0]
                + ";\n"
                + "      "
                + pad_oe_internal_signals
                + " = "
                + self.oe_internal_signals[0]
                + ";\n"
                + "      "
                + self.in_internal_signals[0]
                + " = "
                + pad_in_internal_signals
                + ";\n"
                + "    end\n"
            )

            self.mux_process += "   endcase\n" + "  end\n"

    def create_constant_driver_assign(self):
        cnt = len(self.pad_type_drive)

        for i in range(cnt):

            if self.skip_declaration[i] == False:
                if (
                    self.pad_type_drive[i] == "input"
                    or self.pad_type_drive[i] == "bypass_input"
                ):
                    self.constant_driver_assign += (
                        "  assign " + self.out_internal_signals[i] + " = 1'b0;\n"
                    )
                    self.constant_driver_assign += (
                        "  assign " + self.oe_internal_signals[i] + " = 1'b0;\n"
                    )
                if (
                    self.pad_type_drive[i] == "output"
                    or self.pad_type_drive[i] == "bypass_output"
                ):
                    self.constant_driver_assign += (
                        "  assign " + self.oe_internal_signals[i] + " = 1'b1;\n"
                    )

    def create_core_v_mini_mcu_bonding(self):

        cnt = len(self.pad_type_drive)

        for i in range(cnt):

            if self.driven_manually[i] == False:
                if (
                    self.pad_type_drive[i] == "input"
                    or self.pad_type_drive[i] == "bypass_input"
                ):
                    self.core_v_mini_mcu_bonding += (
                        "    ."
                        + self.signal_name_drive[i]
                        + "i("
                        + self.in_internal_signals[i]
                        + "),\n"
                    )
                if (
                    self.pad_type_drive[i] == "output"
                    or self.pad_type_drive[i] == "bypass_output"
                ):
                    self.core_v_mini_mcu_bonding += (
                        "    ."
                        + self.signal_name_drive[i]
                        + "o("
                        + self.out_internal_signals[i]
                        + "),\n"
                    )
                if (
                    self.pad_type_drive[i] == "inout"
                    or self.pad_type_drive[i] == "bypass_inout"
                ):
                    self.core_v_mini_mcu_bonding += (
                        "    ."
                        + self.signal_name_drive[i]
                        + "i("
                        + self.in_internal_signals[i]
                        + "),\n"
                    )
                    self.core_v_mini_mcu_bonding += (
                        "    ."
                        + self.signal_name_drive[i]
                        + "o("
                        + self.out_internal_signals[i]
                        + "),\n"
                    )
                    self.core_v_mini_mcu_bonding += (
                        "    ."
                        + self.signal_name_drive[i]
                        + "oe_o("
                        + self.oe_internal_signals[i]
                        + "),\n"
                    )

    def create_pad_ring_bonding(self):

        if self.is_muxed:
            append_name = "_muxed"
        else:
            append_name = ""

        if self.pad_type == "input":
            in_internal_signals = self.signal_name + "in_x" + append_name
            self.pad_ring_bonding_bonding = (
                "    ." + self.io_interface + "(" + self.signal_name + "i),\n"
            )
            self.pad_ring_bonding_bonding += (
                "    ." + self.signal_name + "o(" + in_internal_signals + "),"
            )
            self.x_heep_system_interface += "    inout wire " + self.signal_name + "i,"
        if self.pad_type == "output":
            out_internal_signals = self.signal_name + "out_x" + append_name
            self.pad_ring_bonding_bonding = (
                "    ." + self.io_interface + "(" + self.signal_name + "o),\n"
            )
            self.pad_ring_bonding_bonding += (
                "    ." + self.signal_name + "i(" + out_internal_signals + "),"
            )
            self.x_heep_system_interface += "    inout wire " + self.signal_name + "o,"
        if self.pad_type == "inout":
            in_internal_signals = self.signal_name + "in_x" + append_name
            out_internal_signals = self.signal_name + "out_x" + append_name
            oe_internal_signals = self.signal_name + "oe_x" + append_name
            self.pad_ring_bonding_bonding = (
                "    ." + self.io_interface + "(" + self.signal_name + "io),\n"
            )
            self.pad_ring_bonding_bonding += (
                "    ." + self.signal_name + "o(" + in_internal_signals + "),\n"
            )
            self.pad_ring_bonding_bonding += (
                "    ." + self.signal_name + "i(" + out_internal_signals + "),\n"
            )
            self.pad_ring_bonding_bonding += (
                "    ." + self.signal_name + "oe_i(" + oe_internal_signals + "),"
            )
            self.x_heep_system_interface += "    inout wire " + self.signal_name + "io,"

    def __init__(
        self,
        name,
        cell_name,
        pad_type,
        pad_mapping,
        index,
        pad_active,
        pad_driven_manually,
        pad_skip_declaration,
        pad_mux_list,
        has_attribute,
        attribute_bits,
        constant_attribute,
        pad_layout_index,
        pad_layout_orient,
        pad_layout_cell,
        pad_layout_bondpad,
        pad_layout_offset,
        pad_layout_skip,
    ):

        self.name = name
        self.cell_name = cell_name
        self.index = index
        self.localparam = "PAD_" + name.upper()
        self.pad_type = pad_type
        self.pad_mapping = pad_mapping
        self.pad_mux_list = pad_mux_list

        if pad_active == "low":
            name_active = "n"
        else:
            name_active = ""

        self.signal_name = self.name + "_" + name_active

        self.has_attribute = has_attribute
        self.attribute_bits = (
            int(attribute_bits.split(":")[0]) - int(attribute_bits.split(":")[1]) + 1
        )
        self.constant_attribute = constant_attribute

        self.signal_name_drive = []
        self.pad_type_drive = []
        self.driven_manually = []
        self.skip_declaration = []
        self.keep_internal = []

        self.is_muxed = False
        print(
            f"Creating Pad: {self.name} of type {self.pad_type} mapped to {self.pad_mapping} is driven manually: {pad_driven_manually} skip declaration: {pad_skip_declaration}"
        )
        self.is_driven_manually = pad_driven_manually
        self.do_skip_declaration = pad_skip_declaration

        self.layout_index = pad_layout_index
        self.layout_orient = pad_layout_orient
        self.layout_cell = pad_layout_cell
        self.layout_bondpad = pad_layout_bondpad
        self.layout_offset = pad_layout_offset
        self.layout_skip = pad_layout_skip

        if len(pad_mux_list) == 0:
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

        self.io_interface = self.signal_name + "io"

        ### Pad Ring ###
        self.pad_ring_io_interface = ""
        self.pad_ring_ctrl_interface = ""
        self.pad_ring_instance = ""

        ### core v mini mcu ###
        self.core_v_mini_mcu_interface = ""
        self.constant_driver_assign = ""
        self.mux_process = ""

        ### heep systems ###
        self.internal_signals = ""
        self.core_v_mini_mcu_bonding = ""
        self.pad_ring_bonding_bonding = ""
        self.x_heep_system_interface = ""
        if pad_mapping is None:
            self.pad_mapping = None
        elif isinstance(pad_mapping, PadMapping):
            self.pad_mapping = pad_mapping
        elif isinstance(pad_mapping, str):
            # accept "top", "TOP", etc.
            self.pad_mapping = PadMapping(pad_mapping.lower())
        else:
            raise TypeError(
                f"pad_mapping must be PadMapping | str | None, got {type(pad_mapping)}"
            )

    def __eq__(self, value):
        if not isinstance(value, Pad):
            return NotImplemented
        return vars(self) == vars(value)
