# Pad Configuration
The pads of the design can be configured using `.hjson` files, which are then read by `mcu-gen` and template files to generate the pad ring RTL and back-end pad IO of the design.
An example pad configuration file is `pad_cfg.hjson`.

## Basic pad configuration
Each pad of the design is defined as an object in the `pads` list. For example,
```
clk: {
        num: 1,
        type: input
    }
```
defines a single input pad named `clk`.

### Mandatory pad attributes 
Pad objects must contain the following mandatory attributes:
* `num`: The number of pads to be defined.
* `type`: The type of pad.

If the `num` attribute is greater than 1, then a group of pads is defined with names `<pad_name>_n`, where `n` is the sum of the index of the pad and the optional `offset` attribute.

The possible values of `type` are `input`, `output`, `inout`, `bypass_input`, `bypass_output`, `bypass_inout`, and `supply`. The `supply` type is reserved for power pads like VDD and VSS. The `bypass` type pads are meant to be internal signals that are not included in the pad I/O of the top-level design, but are needed internally. For example, these can be pads that are inputs to the Core-V-Mini MCU. This is essentially the same functionality as the `keep_internal` optional parameter described below.

### Optional pad attributes
Pad objects can contain the following optional attributes:
* `num_offset`: The offset of the naming of the first pad when `num`>1. Default: 0
* `mapping`: The side of the design on which the pad is located (i.e. `top`, `bottom`, `right`, or `left`). Default: `top`
* `active`: Whether the pad is active-high (`high`) or active-low (`low`). Default: `high`
* `driven_manually`: Whether pads are driven manually or not. Default: `false`
* `mux`: Multiplexing of pads between two signals.
* `skip_declaration`: Whether or not to skip the definition of the pad in the Core-V-Mini MCU interface. Default: `false`
* `keep_internal`: Whether or not to keep the pad internally (i.e. for the MCU) but exclude it from the top-level pad I/O.

The `driven_manually` attribute is the opposite of the `keep_internal` attribute and `bypass` pad type; it is meant for pads that are part of the top-level I/O interface but not part of the Core-V-Mini MCU interface.

Multiplexing is when two signals share the same pad, which can save space on the I/O ring of the design. Here is an example of a multiplexed pad design:
```
i2s_sck: {
    num: 1,
    type: inout
    mux: {
        i2s_sck: {
            type: inout
        },
        gpio_20: {
            type: inout
        }
    }
```
Please note that if any of the multiplexed signals is of type `inout`, then the global type should also be `inout`.

### Pad layout attributes
When doing the physical layout of the design, it is useful to define certain attributes to be able to place each pad along the I/O ring of the design. These attributes can be defined in the `layout_attributes` dictionary. They are not mandatory for generating the pad RTL, but can be used, for example, to populate a `.io` file template that is read into the placement and routing tool.

The `layout_attributes` dictionary of a given pad object contains the following mandatory attributes:
* `index`: The index of the pad on its side of the I/O ring.
* `cell`: Specific cell to use for the pad. You can get the value from the LEF file of the pads in the desired technology. 
* `bondpad`: Specific bondpad cell to use. 

Additionally, the following optional attributes can be defined:
* `orient`: Orientation of the pad on the physical layout. Options: `R0|R90|R180|R270|MX|MX90|MY|MY90`
* `offset`: offset from edge (in um). If this parameter is not defined, the offsets are calculated automatically using the physical_attributes (see the Pad spacing section below).
* `skip`: distance from neighboring pad (in um). If this parameter is not defined, the skips are calculated automatically using the physical_attributes (see the Pad spacing section below).


### Example pad definition
To put it all together, here is an example of a pad with several of the above attributes defined:
```
gpio_0: {
    num: 1,
    type: inout
    mapping: left
    layout_attributes: {
        index: 8
        cell: PAD1
        orient: mx90
        bondpad: BONDPAD1
    }
}
```


## Defining additional RTL inputs for all pad objects
Depending on the technology used, some pads cells require additional I/Os other than the basic input and output ports. The amount of additional information can be defined in the `bits` field of the `attributes` object as follows:
```
attributes: {
    bits: 7:0
}
```
Then, users can connect these additional attributes in RTL to the correct I/Os of the pad cell.

## Physical attributes
An optional dictionary called `physical_attributes` can be added in order to trigger an `mcu-gen` funcionality that automatically calculates the ideal locations of the pads on the I/O ring. In order to compute these locations, the following attributes must be set:

* `floorplan_dimensions`: Dimensions of the design floorplan (in um)
    * `width`: Width of chip (in um)
    * `length`: Length of the chip (in um)
* `edge_offset`: Offsets from the edge of the design to the specified objects (in um)
    * `bondpad`: Distance (in um) from design edge to bondpad
    * `pad`: Distance (in um) from design edge to pad
* `spacing`: Chosen spacing (in um) between objects 
    * `bondpad`: Spacing between bondpads. Make sure the minimum bondpad pitch of the packaging supplier is not violated.
* `dimensions`: Dimensions of bondpads and pads in the design. These should exactly match the names in the "cell" field of the "layout_attributes". Found in the respective LEF files of each object.
    * `insert_cell_name_here`: Not the actual name of the attribute, but a placeholder for a bondpad name (ex. PAD...)
        * `width`: Width in um

The names of the cells in the `dimensions` field should exactly match those of the `layout_attributes`:`cell` or `layout_attributes`:`bondpad` parameter of each cell. Similarly, each `cell` or `bondpad` name defined among all of the pads should have dimensions associated in order for the pad offset calculation to work correctly. The width of the cell can be found in the LEF file of th pads/bondpads. 

Here is an example of the `physical_attributes` definition:
```
 physical_attributes: {
        floorplan_dimensions: {
            width: 2000
            length: 1500
        },
        edge_offset: {
            bondpad: 20
            pad: 90
        },
        spacing: {
            bondpad: 25
        },
        dimensions: {
            # Bondpads
            BONDPAD1: {
                width: 50
            },
            BONDPAD2: {
                width: 60
            },
            # Pads
            PAD1: {
                width: 40
            },
            PAD2: {
                width: 45
            },
        }
    }
```

### Pad spacing
The following section describes how the spacing between pads is performed. When placing a pad object along the I/O boundary, the `.io` file needs to know either the offset from the pad to the core ring (i.e. the `offset` parameter of the `layout_attributes` of the `pad` object), or the distance from the pad to its neighbor in the clockwise direction (i.e. the `skip` parameter of the `layout_attributes` of the `pad` object). Therefore, either the `offset` or `skip` attribute needs to be defined for all pads to automatically populate the `.io` file. These parameters can be defined one of two ways:
1. Manually defining the `offset` or `skip` attribute for each pad in its `layout_attributes`.
2. Leaving these attributes empty and allowing `mcu-gen` to calculate them automatically using the global `physical_attributes`.
Please note that a combination of the two can be performed; If a given `skip` or `offset` parameter is defined for a pad, then the calculated values are overwritten. This gives the designer flexibility to modify the pad locations on only one side, for example.

![Physical attributes and layout attributes illustration](/images/pad_spacing.png)

The automatic pad placement calculates the pad `offset` and `skip` parameters such that:
1. Bondpads are evenly spaced and centered on their respecitve side
2. Pads are centered with their respective bondpad

Furthermore, `mcu-gen` displays an error message if the number of pads on a given side and the bondpad spacing cause the bondpads to overflow past the I/O boundary on a given side. A solution could be to move pads to a different side, or reduce the bondpad spacing parameter. However, make sure that the bondpad spacing does not exceed the minimum possible spacing defined by the packaging provider.
