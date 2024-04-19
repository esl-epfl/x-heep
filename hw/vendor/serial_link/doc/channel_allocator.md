## Channel Allocator

The *Channel Allocator* is a module of the Serial Link between the Data Link Layer and the Physical Layer that handles faulty channels. It is able to redistribute packets assigned to faulty channels to working channel. Further, it also takes care of the synchronization of the RX channels.

### Calibration
The *Channel Allocator* needs to be configured with the active/working RX and TX channels. To this end, the Serial Link needs to be calibrated initially. The calibration is done by configuring the Serial Link in the [*Raw Mode*](https://github.com/pulp-platform/serial_link/blob/doc/doc/data_link_layer.md#raw-mode) where a mask of working resp. faulty channels is extracted and which is used to configure the Channel Allocator

### Operating modes
The *Channel Allocator* can be bypassed by SW configuration e.g. this is required during the *Raw Mode*.

### RX Synchronization
The Data Link layer splits up a AXI Stream payload to smaller packets that are sent over different channels. On the receiving side those packets need to be assembled again to a AXI Stream payload. Since different channels might have different delays, the packets of the channels need to be synchronized, which is done by the *Channel Allocator*