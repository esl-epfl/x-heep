
#ifndef PADFRAME_H
#define PADFRAME_H
#include <stdint.h>

#ifndef PADFRAME_BASE_ADDRESS
#error "PADFRAME_BASE_ADDRESS is not defined. Set this token to the configuration base address of your padframe before you include this header file."
#endif



/**
 * Sets the chip2pad pad signal for the pad: pad_io_30
 *
 * @param value The value to program into the pad configuration register. A value smaller than 1.
 */
void padframe_xheep_pad_io_30_cfg_chip2pad_set(uint8_t value);

/**
 * Get the currently configured chip2pad value for the pad: pad_io_30
 *
 * @return The value of the chip2pad field
 */
uint8_t padframe_xheep_pad_io_30_cfg_chip2pad_get();

/**
 * Sets the output_en pad signal for the pad: pad_io_30
 *
 * @param value The value to program into the pad configuration register. A value smaller than 1.
 */
void padframe_xheep_pad_io_30_cfg_output_en_set(uint8_t value);

/**
 * Get the currently configured output_en value for the pad: pad_io_30
 *
 * @return The value of the output_en field
 */
uint8_t padframe_xheep_pad_io_30_cfg_output_en_get();

typedef enum {
  PADFRAME_XHEEP_PAD_IO_30_REGISTER = 0,
  PADFRAME_XHEEP_PAD_IO_30_group_GPIO_port_GPIO_30 = 1,
  PADFRAME_XHEEP_PAD_IO_30_group_I2C_port_I2C_SDA = 2,
} padframe_xheep_pad_io_30_mux_sel_t;

/**
   * Choose the entity (a port or the dedicated configuration register) that controls pad_io_30.
   *
   * @param mux_sel Port or configuration register to connect to the pad.
 */
void padframe_xheep_pad_io_30_mux_set(padframe_xheep_pad_io_30_mux_sel_t mux_sel);

/**
 * Read the current multiplexer select value configured for pad_io_30.
 *
 * @return Port or configuration register currently connected to the pad.
 */
 padframe_xheep_pad_io_30_mux_sel_t padframe_xheep_pad_io_30_mux_get();



/**
 * Sets the chip2pad pad signal for the pad: pad_io_31
 *
 * @param value The value to program into the pad configuration register. A value smaller than 1.
 */
void padframe_xheep_pad_io_31_cfg_chip2pad_set(uint8_t value);

/**
 * Get the currently configured chip2pad value for the pad: pad_io_31
 *
 * @return The value of the chip2pad field
 */
uint8_t padframe_xheep_pad_io_31_cfg_chip2pad_get();

/**
 * Sets the output_en pad signal for the pad: pad_io_31
 *
 * @param value The value to program into the pad configuration register. A value smaller than 1.
 */
void padframe_xheep_pad_io_31_cfg_output_en_set(uint8_t value);

/**
 * Get the currently configured output_en value for the pad: pad_io_31
 *
 * @return The value of the output_en field
 */
uint8_t padframe_xheep_pad_io_31_cfg_output_en_get();

typedef enum {
  PADFRAME_XHEEP_PAD_IO_31_REGISTER = 0,
  PADFRAME_XHEEP_PAD_IO_31_group_GPIO_port_GPIO_31 = 1,
  PADFRAME_XHEEP_PAD_IO_31_group_I2C_port_I2C_SCL = 2,
} padframe_xheep_pad_io_31_mux_sel_t;

/**
   * Choose the entity (a port or the dedicated configuration register) that controls pad_io_31.
   *
   * @param mux_sel Port or configuration register to connect to the pad.
 */
void padframe_xheep_pad_io_31_mux_set(padframe_xheep_pad_io_31_mux_sel_t mux_sel);

/**
 * Read the current multiplexer select value configured for pad_io_31.
 *
 * @return Port or configuration register currently connected to the pad.
 */
 padframe_xheep_pad_io_31_mux_sel_t padframe_xheep_pad_io_31_mux_get();




#endif /*  PADFRAME_H */
