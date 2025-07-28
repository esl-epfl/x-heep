/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : i2c.h                                                        **
** version  : 1.0                                                          **
** date     : 03/06/2023                                                   **
**                                                                         **
*****************************************************************************
**                                                                         **
**                                                                         **
*****************************************************************************

*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   i2c.h
* @date   16/05/2023
* @brief  This is the main header file of the HAL for I2C peripheral
*
* In this files there are definitions of low level HAL functions to interact
* with the registers of the I2C peripheral.
* The functionalities implemented allow to configure the peripheral with
* parameters for the FMT and RX fifos, enable or disable interrupts and write
* and read bytes.
*/

#ifndef _I2C_H_
#define _I2C_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include "inttypes.h"
#include "stddef.h"
#include "stdbool.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

/**
 * Maximum bandwidth supported expressed as [nano seconds * Baud]
*/
#define NANO_SEC_PER_KBAUD    1000000   // One million

/**
 * Possible speed (expressed in kBaud) for the I2C devices
*/
#define STANDARD_MODE_SPEED   100   
#define FAST_MODE_SPEED       400
#define FAST_MODE_PLUS_SPEED  1000


/**
 * Speed constants for the Standard-speed mode 
*/
#define T_HIGH_SCL_STANDARD       4000
#define T_LOW_SCL_STANDARD        4700
#define T_START_SET_UP_STANDARD   4700
#define T_START_HOLD_STANDARD     4000
#define T_DATA_SET_UP_STANDARD    250
#define T_SIGNAL_HOLD_STANDARD    0
#define T_STOP_SET_UP_STANDARD    4000
#define T_STOP_HOLD_STANDARD     4700


/**
 * Speed constants for the Fast-speed mode 
*/
#define T_HIGH_SCL_FAST           600
#define T_LOW_SCL_FAST            1300
#define T_START_SET_UP_FAST       600
#define T_START_HOLD_FAST         600
#define T_DATA_SET_UP_FAST        100
#define T_SIGNAL_HOLD_FAST        0
#define T_STOP_SET_UP_FAST        600
#define T_STOP_HOLD_FAST          1300


/**
 * Speed constants for the Fast-plus-speed mode 
*/
#define T_HIGH_SCL_FAST_PLUS      260
#define T_LOW_SCL_FAST_PLUS       500
#define T_START_SET_UP_FAST_PLUS  260
#define T_START_HOLD_FAST_PLUS    260
#define T_DATA_SET_UP_FAST_PLUS   50
#define T_SIGNAL_HOLD_FAST_PLUS   0
#define T_STOP_SET_UP_FAST_PLUS   260
#define T_STOP_HOLD_FAST_PLUS     500

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
 * Pointer to a generic handler function for the I2C.
 * This pointer is used to sequentially store different
 * handlers for different I2C interrupt cases.
*/
typedef void (*handler_func_i2c_t)(void);


/**
 * A toggle state: enabled ior disabled.
*/
typedef enum i2c_toggle {
  /**
   * The "disabled" state.
   */
  kI2cToggleDisabled,
  /*
   * The "enabled" state.
   */
  kI2cToggleEnabled
} i2c_toggle_t;


/**
 * The result of a I2C operation.
 */
typedef enum i2c_result {
  /**
   * Indicates that the operation succeeded.
   */
  kI2cOk,
  /**
   * Indicates some unspecified failure.
   */
  kI2cError,
  /**
   * Indicates that some parameter passed into a function failed a
   * precondition.
   *
   * When this value is returned, no hardware operations occurred.
   */
  kI2cBadArg,
} i2c_result_t;


/**
 * Represents a speed setting for an I2C component: standard, fast, and
 * fast plus, corresponding to 100 kbaud, 400 kbaud, and 1 Mbaud,
 * respectively.
 */
typedef enum i2c_speed {
  /**
   * Standard speed, 100 kilobaud.
   */
  kI2cSpeedStandard,
  /**
   * Fast speed, 400 kilobaud.
   */
  kI2cSpeedFast,
  /**
   * Fast plus speed, 1 megabaud.
   */
  kI2cSpeedFastPlus,
} i2c_speed_t;


/**
 * Timing configuration parameters for I2C.
 *
 * While the I2C device requires ten parameters to describe its timing
 * configuration, the degrees of freedom of those parameters is constrained to
 * the ones in this struct.
 *
 * See `i2c_compute_timing()`
 */
typedef struct i2c_timing_config {
  /**
   * The lowest speed at which an I2C target connected to this host will
   * operate.
   *
   * In other words, this is the maximum speed at which the host can operate
   * without going over what the target devices can handle.
   */
  i2c_speed_t lowest_target_device_speed;
  /**
   * The period of the clock driving this device, in nanoseconds.
   *
   * This value should not be zero, since it is used as a divisor for
   * division.
   */
  uint32_t clock_period_nanos;
  /**
   * The expected time it takes for the I2C bus signal to rise, in nanoseconds.
   *
   * This value is dependent on properties of the hardware's interconnect, and
   * not under actual firmware control.
   */
  uint32_t sda_rise_nanos;
  /**
   * The expected time for the bus signal to fall, similar to `sda_rise_nanos`.
   */
  uint32_t sda_fall_nanos;
  /**
   * The desired period of the SCL line, in nanoseconds.
   *
   * Normally, this should just be `1'000'000 / lowest_target_device_speed`,
   * but the period may be larger if desired.
   *
   * Setting this value to zero will result in the minimum period being used.
   */
  uint32_t scl_period_nanos;
} i2c_timing_config_t;


/**
 * Runtime configuration for I2C.
 *
 * This struct describes runtime timing parameters. Computing these values is
 * somewhat complicated, so these fields should be initialized using the
 * `i2c_compute_timing()` function. A caller is, however, free to compute
 * these values themselves if they prefer, so long as the I2C spec is
 * respected.
 *
 * These values are given in units of input clock cycles.
 */
typedef struct i2c_config {
  uint16_t scl_time_high_cycles;
  uint16_t scl_time_low_cycles;
  uint16_t rise_cycles;
  uint16_t fall_cycles;
  uint16_t start_signal_setup_cycles;
  uint16_t start_signal_hold_cycles;
  uint16_t data_signal_setup_cycles;
  uint16_t data_signal_hold_cycles;
  uint16_t stop_signal_setup_cycles;
  uint16_t stop_signal_hold_cycles;
} i2c_config_t;


/**
 * A snapshot of the entablement state of the interrupts for I2C.
 *
 * This is to be used with the `i2c_irq_disable_all()` and
 * `i2c_irq_restore_all()` functions.
 */
typedef uint32_t i2c_irq_snapshot_t;


/**
 * Represents a valid watermark level for one of the I2C FIFOs.
 */
typedef enum i2c_watermark_level {
  /**
   * A one-byte watermark.
   */
  kI2cLevel1Byte = 0,
  /**
   * A four-byte watermark.
   */
  kI2cLevel4Byte,
  /**
   * An eight-byte watermark.
   */
  kI2cLevel8Byte,
  /**
   * A sixteen-byte watermark.
   */
  kI2cLevel16Byte,
  /**
   * A thirty-byte watermark.
   *
   * Note that this watermark is only supported for RX, and not for FMT.
   */
  kI2cLevel30Byte,
} i2c_level_t;


/**
 * Flags for a formatted I2C byte, used by the `i2c_write_byte_raw()`
 * function.
 */
typedef struct i2c_fmt_flags {
  /**
   * Causes a start signal to be sent before the byte.
   *
   * If a start has been issued during the current transaction, this will issue
   * a repeated start.
   */
  bool start;
  /**
   * Causes a stop signal to be sent after the byte.
   *
   * This flag cannot be set when both `read` and `read_cont` are set.
   */
  bool stop;
  /**
   * Causes the byte to be interpreted as an unsigned number of bytes to read
   * from the target; 0 is interpreted as 256.
   */
  bool read;
  /**
   * Requires `read` to be set; if so, once the final byte associated with this
   * read is received, it will be acknowledged, allowing the read operation to
   * continue.
   */
  bool read_cont;
  /**
   * By default, the hardware expects an ACK after every byte sent, and raises
   * an exception. This flag disables that behavior.
   *
   * This flag cannot be set along with `read` or `read_cont`.
   */
  bool suppress_nak_irq;
} i2c_fmt_flags_t;


/**
 * Available formatting codes for `i2c_write_byte_raw()`.
 *
 * Each code describes how to interpret the `byte` parameter, referred to below
 * as "the byte".
 */
typedef enum i2c_fmt {
  /**
   * Start a transaction. This sends a START signal followed by the byte.
   * The byte sent will form (potentially part of) the target address for the
   * transaction.
   *
   * May be followed by any format code.
   */
  kI2cFmtStart,
  /**
   * Transmit byte. This simply sends the byte. It may need to be used in
   * conjunction with `Start` to send a multi-byte target address.
   *
   * May be followed by any format code.
   */
  kI2cFmtTx,
  /**
   * Transmit byte and stop. This sends the byte, and then sends a stop
   * signal, completing a transaction.
   *
   * Only `Start` may follow this code.
   */
  kI2cFmtTxStop,
  /**
   * Request `n` bytes, where `n` is the byte interpreted as an unsigned
   * integer; a byte value of `0` will be interpreted as requesting `256`
   * bytes. This will NAK the last byte.
   *
   * Only `Start` may follow this code (this code does not stop a transaction;
   * see `RxStop`).
   */
  kI2cFmtRx,
  /**
   * Request `n` bytes, same as `Rx`, but ACK the last byte so that more data
   * can be requested.
   *
   * May be followed by `RxContinue`, `Rx`, or `RxStop`.
   */
  kI2cFmtRxContinue,
  /**
   * Request `n` bytes, same as `Rx`, but, after NAKing the last byte, send a
   * stop signal to end the transaction.
   *
   * Only `Start` may follow this code.
   */
  kI2cFmtRxStop,
} i2c_fmt_t;


/**
 * Represents an I2C-related interrupt type.
 */
typedef enum i2c_irq {
  /**
   * Fired when the FMT FIFO underflows its watermark.
   */
  kI2cIrqFmtWatermarkUnderflow = 0,
  /**
   * Fired when the RX FIFO overflows its watermark.
   */
  kI2cIrqRxWatermarkOverflow,
  /**
   * Fired when the FMT FIFO overflows.
   */
  kI2cIrqFmtFifoOverflow,
  /**
   * Fired when the RX FIFO overflows.
   */
  kI2cIrqRxFifoOverflow,
  /**
   * Fired when there is no ACK in response to an address or data write.
   */
  kI2cIrqNak,
  /**
   * Fired when the SCL line seems to have interference.
   */
  kI2cIrqSclInterference,
  /**
   * Fired when the SDA line seems to have interference.
   */
  kI2cIrqSdaInterference,
  /**
   * Fired when the target stretches the clock beyond the allowed period.
   */
  kI2cIrqClockStretchTimeout,
  /**
   * Fired when the target does not maintain a stable SDA line.
   */
  kI2cIrqSdaUnstable,
  /**
   * Fires when host issues a repeated START or terminates the transaction
   * with a STOP.
  */
  kI2cIrqTransComplete,
  /**
   * Fires when the target stretches the clock for a read command (TX FIFO empty)
  */
  kI2cIrqTxEmpty,
  /**
   * Fires when the target stretches the clock for a read command (ACQ FIFO non empty)
  */
  kI2cIrqTxNonEmpty,
  /**
   * Fires when the TX FIFO overflows
  */
  kI2cIrqTxOverflow,
  /**
   * Fires when the ACQ FIFO is full
  */
  kI2cIrqAcqOverflow,
  /**
   * Fires when a STOP is received without a preceding NACK (target mode)
  */
  kI2cIrqAckStop,
  /**
   * Fires when the host stop sending the clock during a transaction (target mode)
  */
  kI2cIrqHostTimeout,

  /**
   * Helper variable used to keep track of the amount of different I2C-related
   * interrupt requests there can be.
   * Whenever a new IRQ is added after the last one, this variable should 
   * be changed as well in order to keep track of the correct number.
  */
  kI2cNIrqTypes = kI2cIrqHostTimeout
} i2c_irq_t;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * IRQ handler for when the FMT FIFO depth goes under the watermark
*/
void handler_irq_i2c_fmtWatermarkUnderflow(void);

/**
 * IRQ handler for when the RX FIFO depth goes over the watermark
*/
void handler_irq_i2c_rxWatermarkOverflow(void);

/**
 * IRQ handler for when the FMT FIFO overflows
*/
void handler_irq_i2c_fmtOverflow(void);

/**
 * IRQ handler for when the RX FIFO overflows
*/
void handler_irq_i2c_rxOverflow(void);

/**
 * IRQ handler for when there is no ACK response
*/
void handler_irq_i2c_nak(void);

/**
 * IRQ handler for when the SCL line appears to have interference
*/
void handler_irq_i2c_sclInterference(void);

/**
 * IRQ handler for when the SDA line appears to have interference
*/
void handler_irq_i2c_sdaInteference(void);

/**
 * IRQ handler for when the target stretches the clock beyond the allower period
*/
void handler_irq_i2c_clockStretchTimeout(void);

/**
 * IRQ handler for when the target doesn't keep the SDA line stable
*/
void handler_irq_i2c_sdaUnstable(void);

/**
 * IRQ handler for when the host issues a repeated START or terminates the transaction
 * with a STOP.
*/
void handler_irq_i2c_transComplete(void);

/**
 * IRQ handler for when the target stretches the clock for a read command (TX FIFO empty)
*/
void handler_irq_i2c_txEmpty(void);

/**
 * IRQ handler for when the target stretches the clock for a read command (ACQ FIFO non empty)
*/
void handler_irq_i2c_txNonEmpty(void);

/**
 * IRQ handler for when the TX FIFO overflows
*/
void handler_irq_i2c_txOverflow(void);

/**
 * IRQ handler for when the ACQ FIFO is full
*/
void handler_irq_i2c_acqOverflow(void);

/**
 * IRQ handler for when a STOP is received without a preceding NACK (target mode)
*/
void handler_irq_i2c_ackStop(void);

/**
 * IRQ handler for when the host stop sending the clock during a transaction (target mode)
*/
void handler_irq_i2c_hostTimeout(void);

/**
 * Generic hanlder for the I2C interupts.
 * Whenever the I2C generates an interrupt, this function will be called
 * by the interrupt controller and the proper handler will be called, basing on the 
 * interrupt ID.
 *
 * @param id An interrupt source ID
*/
void handler_i2c(uint32_t id);


/**
 * Computes timing parameters for an I2C device and store them in `config`.
 * 
 * @param timing_config Configuration values for producing timing parameters.
 * @param[out] config I2C configuration to which to apply the computed parameters.
 * @return The result of the operation.
*/
i2c_result_t i2c_compute_timing(i2c_timing_config_t timing_config, 
                                i2c_config_t *config);


/**
 * Configures I2C with runtime information.
 *
 * @param config Runtime configuration parameters.
 * @return The result of the operation.
 */
i2c_result_t i2c_configure(i2c_config_t config);


/**
 * Returns whether a particular interrupt is currently pending.
 *
 * @param irq An interrupt type.
 * @param[out] is_pending Out-param for whether the interrupt is pending.
 * @return The result of the operation.
 */
i2c_result_t i2c_irq_is_pending(i2c_irq_t irq, bool *is_pending);


/**
 * Acknowledges a particular interrupt, indicating to the hardware that it has
 * been successfully serviced.
 *
 * @param irq An interrupt type.
 * @return The result of the operation.
 */
i2c_result_t i2c_irq_acknowledge(i2c_irq_t irq);


/**
 * Checks whether a particular interrupt is currently enabled or disabled.
 *
 * @param irq An interrupt type.
 * @param[out] state State of the interrupt.
 * @return The result of the operation.
 */
i2c_result_t i2c_irq_get_enabled(i2c_irq_t irq, i2c_toggle_t *state);


/**
 * Sets whether a particular interrupt is currently enabled or disabled.
 *
 * @param irq An interrupt type.
 * @param state The new toggle state for the interrupt.
 * @return The result of the operation.
 */
i2c_result_t i2c_irq_set_enabled(i2c_irq_t irq, i2c_toggle_t state);


/**
 * Forces a particular interrupt, causing it to be serviced as if hardware had
 * asserted it.
 *
 * @param irq An interrupt type.
 * @return The result of the operation.
 */
i2c_result_t i2c_irq_force(i2c_irq_t irq);


/**
 * Disables all interrupts, optionally snapshotting all toggle state for later
 * restoration.
 *
 * @param[out] snapshot Out-param for the snapshot.
 * @return The result of the operation.
 */
i2c_result_t i2c_irq_disable_all(i2c_irq_snapshot_t *snapshot);


/**
 * Restores interrupts from the given snapshot.
 *
 * This function can be used with `i2c_irq_disable_all()` to temporary
 * interrupt save-and-restore.
 *
 * @param snapshot A snapshot to restore from.
 * @return The result of the operation.
 */
i2c_result_t i2c_irq_restore_all(const i2c_irq_snapshot_t *snapshot);


/**
 * Resets the state of the RX FIFO, essentially dropping all received bytes.
 *
 * @return The result of the operation.
 */
i2c_result_t i2c_reset_rx_fifo();


/**
 * Resets the state of the FMT FIFO, essentially dropping all scheduled
 * operations.
 *
 * @return The result of the operation.
 */
i2c_result_t i2c_reset_fmt_fifo();


/**
 * Sets watermarks for for the RX and FMT FIFOs, which will trigger the respective
 * interrupts when each fifo exceeds, or falls below, the set level.
 *
 * Note that the 30-byte level is only supported for the RX FIFO: trying to use
 * it with the FMT FIFO is an error.
 * 
 * @param rx_level The desired watermark level for the RX FIFO.
 * @param fmt_level The desired watermark level for the FMT FIFO.
 * @return The result of the operation.
 */
i2c_result_t i2c_set_watermarks(i2c_level_t rx_level, i2c_level_t fmt_level);


/**
 * Enables or disables the "Host I2C" functionality, effectively turning the
 * I2C device on or off. This function should be called to enable the device
 * once timings, interrupts, and watermarks are all configured.
 *
 * @param state The new toggle state for the host functionality.
 * @return The result of the operation.
 */
i2c_result_t i2c_host_set_enabled(i2c_toggle_t state);

/**
 * Enables or disables the "override mode". In override mode, software is able
 * to directly control the driven values of the SCL and SDA lines using
 * `i2c_override_drive_pins()`.
 *
 * @param state The new toggle state for override mode.'
 * @return The result of the operation.
 */
i2c_result_t i2c_override_set_enabled(i2c_toggle_t state);


/**
 * Drives the SCL and SDA pins to the given values when "override mode" is
 * enabled.
 *
 * @param scl The value to drive SCL to.
 * @param sda The value to drive SDA to.
 * @return The result of the operation.
 */
i2c_result_t i2c_override_drive_pins(bool scl, bool sda);


/**
 * Returns oversampling of the last 16 values of the SCL and SDA pins, with the
 * zeroth bit being the most recent.
 *
 * @param[out] scl_samples SCL sample bits.
 * @param[out] sda_samples SDA sample bits.
 * @return The result of the operation.
 */
i2c_result_t i2c_override_sample_pins(uint16_t *scl_samples, 
                                      uint16_t *sda_samples);


/**
 * Returns the current levels, i.e., number of entries, in the FMT and RX FIFOs.
 * These values represent the number of entries pending for send by hardware,
 * and entries pending for read by software, respectively.
 *
 * @param[out] fmt_fifo_level The number of unsent FMT bytes.
 * @param[out] rx_fifo_level The number of unread RX bytes.
 * @return The result of the operation.
 */
i2c_result_t i2c_get_fifo_levels(uint8_t *fmt_fifo_level,
                                  uint8_t *rx_fifo_level);


/**
 * Pops an entry (a byte) off of the RX FIFO.
 *
 * @param[out] byte The popped byte.
 * @return The result of the operation.
 */
i2c_result_t i2c_read_byte(uint8_t *byte);


/**
 * Pushes a raw write entry onto the FMT FIFO, consisting of a byte and format
 * flags. This function can be called in sequence to enqueue an I2C
 * transmission.
 *
 * This function has to be preferred for testing and debugging.
 * 
 * @param byte The value to push onto the FIFO.
 * @param flags The flags to use for this write.
 * @return The result of the operation.
 */
i2c_result_t i2c_write_byte_raw(uint8_t byte, i2c_fmt_flags_t flags);


/**
 * Pushes a write entry onto the FMT FIFO, consisting of a byte and a format
 * code. This function can be called in sequence to enqueue an I2C
 * transmission.
 *
 * @param byte The value to push onto the FIFO.
 * @param code The code to use for this write.
 * @param suppress_nak_irq Whether to supress the NAK IRQ for this one byte.
 *        May not be used in combination with `Rx` codes.
 * @return The result of the operation.
 */
i2c_result_t i2c_write_byte(uint8_t byte, i2c_fmt_t code, bool suppress_nak_irq);

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/

#endif
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
