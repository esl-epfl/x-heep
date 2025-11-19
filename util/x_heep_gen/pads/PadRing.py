from .Pad import Pad, PadMapping


def as_bool(v, default: bool = False) -> bool:
    if isinstance(v, bool):
        return v
    if isinstance(v, str):
        s = v.strip().lower()
        if s in {"true", "1", "yes", "y"}:
            return True
        if s in {"false", "0", "no", "n"}:
            return False
    return default


def get_nested(d, path, default=None):
    cur = d
    for k in path:
        if not isinstance(cur, dict) or k not in cur:
            return default
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


class PadRing:
    def __init__(self, pad_cfg):

        pads = pad_cfg["pads"]

        try:
            pads_attributes = pad_cfg["attributes"]
            pads_attributes_bits = pads_attributes["bits"]
        except KeyError:
            pads_attributes = None
            pads_attributes_bits = "-1:0"

        # Read HJSON description of External Pads

        pad_list = []
        external_pad_list = []

        pad_constant_driver_assign = ""
        pad_mux_process = ""

        pad_muxed_list = []

        # in    ternal pads
        (
            pad_list,
            pad_muxed_internal,
            next_index,
            pad_constant_driver_assign,
            pad_mux_process,
        ) = build_pads_from_block(
            pads_block=pads,
            start_index=0,
            pads_attributes_present=(pads_attributes is not None),
            pads_attributes_bits=pads_attributes_bits,
            default_constant_attribute=False,
            always_emit_ring=False,  # respect keep_internal for internal pads
        )

        # external pads (continue indexing, always emit ring)
        external_pad_list = []
        pad_muxed_external = []

        # merge, totals
        total_pad_list = pad_list + external_pad_list
        pad_muxed_list = pad_muxed_internal + pad_muxed_external
        total_pad = len(total_pad_list)
        total_pad_muxed = len(pad_muxed_list)

        # max mux selector width (0 if none)
        max_total_pad_mux_bitlengh = 0
        if pad_muxed_list:
            max_total_pad_mux_bitlengh = max(
                (len(p.pad_mux_list) - 1).bit_length() for p in pad_muxed_list
            )

        # remove trailing comma from last PAD io_interface (kept to preserve behavior)
        if total_pad_list:
            last_pad = total_pad_list.pop()
            last_pad.remove_comma_io_interface()
            total_pad_list.append(last_pad)

        physical_attributes = None
        top_pad_list = None
        bottom_pad_list = None
        left_pad_list = None
        right_pad_list = None
        bondpad_offsets = None

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
            pass

        self.pad_list = pad_list
        self.total_pad_list = total_pad_list
        self.pad_muxed_list = pad_muxed_list
        self.total_pad = total_pad
        self.total_pad_muxed = total_pad_muxed
        self.max_total_pad_mux_bitlengh = max_total_pad_mux_bitlengh
        self.top_pad_list = top_pad_list
        self.bottom_pad_list = bottom_pad_list
        self.left_pad_list = left_pad_list
        self.right_pad_list = right_pad_list
        self.bondpad_offsets = bondpad_offsets
        self.physical_attributes = physical_attributes
        self.external_pad_list = external_pad_list
        self.pad_constant_driver_assign = pad_constant_driver_assign
        self.pad_mux_process = pad_mux_process
        self.pads_attributes = pads_attributes


def prepare_pads_for_layout(total_pad_list, physical_attributes):
    """
    Separate pads into pad lists for the top, bottom, left, and right pads and order them according to their layout_index attribute, and set their positions on the floorplan.
    """

    # Separate pads according to side
    top_pad_list = []
    bottom_pad_list = []
    right_pad_list = []
    left_pad_list = []
    pad_lists = {
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
                "ERROR: Pad {0} has an invalid mapping {1}. Please set mapping to top, bottom, left, or right.".format(
                    pad.cell_name, getattr(pad, "pad_mapping", None)
                )
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


def build_mux_list(
    block,
    pad_mapping,
    pads_attributes_present: bool,
    pads_attributes_bits: str,
    pad_constant_attribute: bool,
    pad_layout_index,
    pad_layout_orient,
    pad_layout_cell,
    pad_layout_bondpad,
    pad_layout_offset,
    pad_layout_skip,
):
    mux_list = []
    for mux_name, entry in (block.get("mux") or {}).items():
        mux = Pad(
            mux_name,
            "",
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
            pad_layout_index,
            pad_layout_orient,
            pad_layout_cell,
            pad_layout_bondpad,
            pad_layout_offset,
            pad_layout_skip,
        )
        mux_list.append(mux)
    return mux_list


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
    if (side == PadMapping.TOP) or (side == PadMapping.BOTTOM):
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


def build_pads_from_block(
    pads_block,
    start_index: int,
    pads_attributes_present: bool,
    pads_attributes_bits: str,
    default_constant_attribute: bool,
    always_emit_ring: bool,
):
    pad_list = []
    pad_muxed_list = []
    const_assign_parts = []
    mux_process_parts = []
    next_index = start_index

    for key, block in pads_block.items():
        base_name = key
        pad_num = int(block["num"])
        pad_type = (
            block["type"].strip(",")
            if isinstance(block["type"], str)
            else block["type"]
        )
        pad_offset = int(block.get("num_offset", 0))
        pad_active = block.get("active", "high")
        pad_mapping = coerce_enum(PadMapping, block.get("mapping"), None)

        pad_driven_manually = as_bool(block.get("driven_manually"), False)
        pad_skip_declaration = as_bool(block.get("skip_declaration"), False)
        pad_keep_internal = as_bool(block.get("keep_internal"), False)
        pad_constant_attribute = as_bool(
            block.get("constant_attribute"), default_constant_attribute
        )

        # layout (optional)
        pad_layout_orient = get_nested(block, ["layout_attributes", "orient"])
        pad_layout_cell = get_nested(block, ["layout_attributes", "cell"])
        pad_layout_bondpad = get_nested(block, ["layout_attributes", "bondpad"])
        pad_layout_offset = get_nested(block, ["layout_attributes", "offset"])
        pad_layout_skip = get_nested(block, ["layout_attributes", "skip"])
        pad_layout_index = get_nested(block, ["layout_attributes", "index"])

        # mux list
        pad_mux_list = build_mux_list(
            block,
            pad_mapping,
            pads_attributes_present,
            pads_attributes_bits,
            pad_constant_attribute,
            pad_layout_index,
            pad_layout_orient,
            pad_layout_cell,
            pad_layout_bondpad,
            pad_layout_offset,
            pad_layout_skip,
        )

        # unified loop (single/multi)
        for i in range(pad_num):
            idx = next_index + i
            suf = f"_{i + pad_offset}" if pad_num > 1 else ""
            pad_name = f"{base_name}{suf}"
            pad_cell_name = f"pad_{base_name}{suf}_i"

            pad_obj = Pad(
                pad_name,
                pad_cell_name,
                pad_type,
                pad_mapping,
                idx,
                pad_active,
                pad_driven_manually,
                pad_skip_declaration,
                pad_mux_list,
                pads_attributes_present,
                pads_attributes_bits,
                pad_constant_attribute,
                pad_layout_index,
                pad_layout_orient,
                pad_layout_cell,
                pad_layout_bondpad,
                pad_layout_offset,
                pad_layout_skip,
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

    return (
        pad_list,
        pad_muxed_list,
        next_index,
        "".join(const_assign_parts),
        "".join(mux_process_parts),
    )
