/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************
**
** project  : X-HEEP
** filename : spi_sdk.h
** version  : 1
** date     : 18/04/24
**
***************************************************************************
**
** Copyright (c) EPFL contributors.
** All rights reserved.
**
***************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file   spi_sdk.h
* @date   18/04/24
* @author Llorenç Muela
* @brief  The Serial Peripheral Interface (SPI) SDK to set up and use the
* SPI peripheral
*/

#ifndef _SDK_SPI_H_
#define _SDK_SPI_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

// Chosen arbitrarily
#define SPI_CSN_TIMES_DEFAULT 15
// Default timeout for blocking transactions in milliseconds
#define SPI_TIMEOUT_DEFAULT   100

/**
 * @brief Macro to create a Slave SPI device with standard parameters.
 */
#define SPI_SLAVE(cs_id, freq_max) (spi_slave_t) { \
    .csid       = cs_id, \
    .data_mode  = SPI_DATA_MODE_0, \
    .full_cycle = false, \
    .csn_lead   = SPI_CSN_TIMES_DEFAULT, \
    .csn_trail  = SPI_CSN_TIMES_DEFAULT, \
    .csn_idle   = SPI_CSN_TIMES_DEFAULT, \
    .freq       = freq_max \
}

/**
 * @brief Macro to instantiate a Dummy Segment
 */
#define SPI_SEG_DUMMY(cycles) (spi_segment_t) { \
    .len  = cycles, \
    .mode = SPI_MODE_DUMMY \
}

/**
 * @brief Macro to instantiate a TX Segment
 */
#define SPI_SEG_TX(bytes) (spi_segment_t) { \
    .len  = bytes, \
    .mode = SPI_MODE_TX_STD \
}

/**
 * @brief Macro to instantiate a RX Segment
 */
#define SPI_SEG_RX(bytes) (spi_segment_t) { \
    .len  = bytes, \
    .mode = SPI_MODE_RX_STD \
}

/**
 * @brief Macro to instantiate a BIDIR Segment
 */
#define SPI_SEG_BIDIR(bytes) (spi_segment_t) { \
    .len  = bytes, \
    .mode = SPI_MODE_BIDIR \
}

/**
 * @brief Macro to instantiate a Dual Speed TX Segment
 */
#define SPI_SEG_TX_DUAL(bytes) (spi_segment_t) { \
    .len  = bytes, \
    .mode = SPI_MODE_TX_DUAL \
}

/**
 * @brief Macro to instantiate a Dual Speed RX Segment
 */
#define SPI_SEG_RX_DUAL(bytes) (spi_segment_t) { \
    .len  = bytes, \
    .mode = SPI_MODE_RX_DUAL \
}

/**
 * @brief Macro to instantiate a Quad Speed TX Segment
 */
#define SPI_SEG_TX_QUAD(bytes) (spi_segment_t) { \
    .len  = bytes, \
    .mode = SPI_MODE_TX_QUAD \
}

/**
 * @brief Macro to instantiate a Quad Speed RX Segment
 */
#define SPI_SEG_RX_QUAD(bytes) (spi_segment_t) { \
    .len  = bytes, \
    .mode = SPI_MODE_RX_QUAD \
}

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
* @brief SPI Peripheral IDX
*/
typedef enum {
    SPI_IDX_FLASH       = 0,  // Identifier for the SPI FLASH peripheral
    SPI_IDX_HOST        = 1,  // Identifier for the SPI HOST1 peripheral
    SPI_IDX_HOST_2      = 2   // Identifier for the SPI HOST2 peripheral
} spi_idx_e;

typedef enum {
    SPI_CODE_OK                 = 0x0000, // Everything Okay
    SPI_CODE_BASE_ERROR         = 0x0001, // HAL complained (should never happen)
    SPI_CODE_IDX_INVAL          = 0x0002, // The idx of the provided spi_t is not valid
    SPI_CODE_WM_EXCEEDS         = 0x0004, // The provided watermark is too big
    SPI_CODE_NOT_INIT           = 0x0008, // The provided spi_t was not initialized
    SPI_CODE_SLAVE_CSID_INVAL   = 0x0010, // The csid of the slave is not valid
    SPI_CODE_SLAVE_FREQ_INVAL   = 0x0020, // Max frequency of the slave is too low
    SPI_CODE_NOT_IDLE           = 0x0040, // The SPI device is busy (but not from SDK)
    SPI_CODE_SLAVE_INVAL        = 0x0080, // Something was wrong with the slave configuration
    SPI_CODE_SEGMENT_INVAL      = 0x0100, // The spi_mode_e of the segment was invalid
    SPI_CODE_IS_BUSY            = 0x0200, // The SPI device is busy
    SPI_CODE_TXN_LEN_INVAL      = 0x0400, // The transaction length is 0 or too long
    SPI_CODE_TIMEOUT_INVAL      = 0x0800  // The specified timeout is invalid
} spi_codes_e;

typedef enum {
    SPI_DATA_MODE_0 = 0,     // cpol = 0, cpha = 0
    SPI_DATA_MODE_1 = 1,     // cpol = 0, cpha = 1
    SPI_DATA_MODE_2 = 2,     // cpol = 1, cpha = 0
    SPI_DATA_MODE_3 = 3      // cpol = 1, cpha = 1
} spi_datamode_e;

typedef enum {
    SPI_MODE_DUMMY   = 0,   // Dummy SCK command segment
    SPI_MODE_RX_STD  = 1,   // Standard receive command segment
    SPI_MODE_TX_STD  = 2,   // Standard transmit command segment
    SPI_MODE_BIDIR   = 3,   // Standard bidirectional command segment
    // 4 is same than DUMMY
    SPI_MODE_RX_DUAL = 5,   // Dual speed receive command segment
    SPI_MODE_TX_DUAL = 6,   // Dual speed transmit command segment
    // 7 is invalid
    // 8 is same than DUMMY
    SPI_MODE_RX_QUAD = 9,   // Quad speed receive command segment
    SPI_MODE_TX_QUAD = 10   // Quad speed transmit command segment
    // everything > 10 is invalid
} spi_mode_e;

// State is useful to know if the peripheral is busy, but also very useful if a
// blocking transaction was made and we want to check the result of the blocking
// transation (i.e. error or success)
typedef enum {
    // Indicates SPI device was never initialized (should never happen!)
    SPI_STATE_NONE      = 0x00,
    // Indicates SPI device never executed a transaction
    SPI_STATE_INIT      = 0x01,
    // Indicates SPI device is currently processing a transaction
    SPI_STATE_BUSY      = 0x02,
    // Indicates SPI device has successfully executed a transaction
    SPI_STATE_DONE      = 0x04,
    // Indicates there was an error during transaction
    SPI_STATE_ERROR     = 0x08,
    // Indicates the transaction execution has timed-out
    SPI_STATE_TIMEOUT   = 0x10,
    // Indicates the argument passed to spi_get_state was not valid
    SPI_STATE_ARG_INVAL = 0x20
} spi_state_e;

/**
 * @brief Type defining the configuration of the slave device with whom to communicate
 */
typedef struct {
    // The Chip Select line where device connected
    uint8_t        csid       : 2;
    // The data sampling and transmitting mode (polarity and phase)
    spi_datamode_e data_mode  : 2;
    // If 1 data is sampled a full cycle after shifting data out, instead of half cycle
    bool           full_cycle : 1;
    // The minimum number of sck half-cycles to hold cs_n high between commands
    uint8_t        csn_idle   : 4;
    // The number of half sck cycles, CSNTRAIL+1, to leave between last edge of sck 
    // and the rising edge of cs_n
    uint8_t        csn_trail  : 4;
    // The number of half sck cycles, CSNLEAD+1, to leave between the falling edge 
    // of cs_n and the first edge of sck
    uint8_t        csn_lead   : 4;
    // The maximum frequency in hertz of the slave
    uint32_t       freq       : 32;
} spi_slave_t;

/**
 * @brief Type defining a command segment
 */
typedef struct {
    uint32_t   len  : 24;  // Length of data in bytes for the particular segment
    spi_mode_e mode : 4;   // Communication mode (TX, BIDIR, RX_QUAD, ...)
} spi_segment_t;

// Event / Error interrupt callback  (txbuffer, txlen, rxbuffer, rxlen)
typedef void (*spi_cb_t)(const uint32_t*, uint32_t, uint32_t*, uint32_t);

/**
 * @brief Structure holding all the callbacks for a transaction.
 *        Each callback may be NULL if you don't want it to be called.
 */
typedef struct {
    spi_cb_t done_cb;   // Called once transaction is done
    spi_cb_t txwm_cb;   // Called each time TX watermark event is triggered
    spi_cb_t rxwm_cb;   // Called each time RX watermark event is triggered
    spi_cb_t error_cb;  // Called when there was an error during transaction
} spi_callbacks_t;

/**
 * @brief Type holding Information to use SDK.
 */
typedef struct {
    spi_idx_e   idx;   // The identifier for the desired SPI devide
    uint32_t    id;    // spi_t instance ID
    bool        init;  // Indicates if initialization was successful
    spi_slave_t slave; // The slave with whom to communicate configuration 
} spi_t;

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
 * @brief Initialize desired SPI device and obtain structure to use SDK functions.
 *        This method has to be called prior to any use of other functions of the SDK.
 * 
 *  Important: This method validates the provided slave and adapts its frequency
 *  based on the MCU frequency and the SCK divider. If you do not create the spi_t
 *  structure by calling this function the slave may be invalid resulting in errors.
 * 
 * @param idx spi_idx_e representing the SPI device
 * @param slave spi_slave_t correctly configured slave SPI device to interact with
 * @return spi_t structure to use SDK functions
 */
spi_t spi_init(spi_idx_e idx, spi_slave_t slave);

/**
 * @brief Uninitializes the spi by unsetting all fields.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 */
void spi_deinit(spi_t* spi);

/**
 * @brief Completely reset the SPI device (clears all status, stops all transactions,
 *        empties FIFOs)
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_reset(spi_t* spi);

/**
 * @brief Change the TX watermark of the specific SPI Host peripheral
 *        Maximum is currently 72. Check SPI_HOST_PARAM_TX_DEPTH in HAL.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param watermark The desired new watermark
 * @return SPI_CODE_IDX_INVAL  if spi.idx not valid
 * @return SPI_CODE_NOT_INIT   if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_WM_EXCEEDS if specified watermark is too high
 * @return SPI_CODE_OK         if success
 */
spi_codes_e spi_set_txwm(spi_t* spi, uint8_t watermark);

/**
 * @brief Get the TX watermark of the specific SPI Host peripheral
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param watermark The current watermark will be stored in this variable
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_get_txwm(spi_t* spi, uint8_t* watermark);

/**
 * @brief Change the RX watermark of the specific SPI Host peripheral
 *        Maximum is currently 64. Check SPI_HOST_PARAM_RX_DEPTH in HAL.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param watermark The desired new watermark
 * @return SPI_CODE_IDX_INVAL  if spi.idx not valid
 * @return SPI_CODE_NOT_INIT   if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_WM_EXCEEDS if specified watermark is too high
 * @return SPI_CODE_OK         if success
 */
spi_codes_e spi_set_rxwm(spi_t* spi, uint8_t watermark);

/**
 * @brief Get the RX watermark of the specific SPI Host peripheral
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param watermark The current watermark will be stored in this variable
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_get_rxwm(spi_t* spi, uint8_t* watermark);

/**
 * @brief Set the timeout in milliseconds for blocking transactions for the 
 *        specific SPI Host peripheral.
 *        Maximum is UINT32_MAX * 1000 / core_frequency.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param watermark The desired new watermark
 * @return SPI_CODE_IDX_INVAL     if spi.idx not valid
 * @return SPI_CODE_NOT_INIT      if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_TIMEOUT_INVAL if the specified timout is too large
 * @return SPI_CODE_OK            if success
 */
spi_codes_e spi_set_timeout(spi_t* spi, uint32_t timeout);

/**
 * @brief Get the timeout in milliseconds for blocking transactions for the 
 *        specific SPI Host peripheral.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param timeout The current timeout will be stored in this variable
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_get_timeout(spi_t* spi, uint32_t* timeout);

/**
 * @brief Change the communication frequency of the slave
 *        /!\ If the frequency is higher than the maximum frequency it will just
 *            be capped (i.e. no error)
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param freq The new frequency the slave should be changed
 * @return SPI_CODE_IDX_INVAL        if spi.idx not valid
 * @return SPI_CODE_NOT_INIT         if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_SLAVE_CSID_INVAL if slave's csid is invalid
 * @return SPI_CODE_SLAVE_FREQ_INVAL if new frequency is too low
 * @return SPI_CODE_OK               if success
 */
spi_codes_e spi_set_slave_freq(spi_t* spi, uint32_t freq);

/**
 * @brief Read the SPI current state.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @return SPI_STATE_NONE      : SPI device was never initialized (should never happen!)
 * @return SPI_STATE_INIT      : SPI device never executed a transaction
 * @return SPI_STATE_BUSY      : SPI device is currently processing a transaction
 * @return SPI_STATE_DONE      : SPI device has successfully executed a transaction
 * @return SPI_STATE_ERROR     : There was an error during transaction
 * @return SPI_STATE_ARG_INVAL : The argument passed to this function was not valid
 */
spi_state_e spi_get_state(spi_t* spi);

/**
 * @brief Executes a TX command.
 *        /!\ Caution: len is in bytes.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param src_buffer An initialized buffer/array with all the data to send
 * @param len The size in bytes of the transaction (/!\ must be coherent with 
 *            src_buffer size)
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_transmit(spi_t* spi, const uint32_t* src_buffer, uint32_t len);

/**
 * @brief Executes an RX command.
 *        /!\ Caution: len is in bytes.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param dest_buffer An initialized buffer/array to store the received data
 * @param len The size in bytes of the transaction (/!\ must be coherent with 
 *            dest_buffer size)
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_receive(spi_t* spi, uint32_t* dest_buffer, uint32_t len);

/**
 * @brief Executes a Bidirectional (TX and RX) command.
 *        /!\ Caution: len is in bytes and is for the TX as well as RX.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param src_buffer An initialized buffer/array with all the data to send
 * @param dest_buffer An initialized buffer/array to store the received data
 * @param len The size in bytes of the transaction (/!\ must be coherent with 
 *            src_buffer and dest_buffer size)
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_transceive(spi_t* spi, const uint32_t* src_buffer, 
                           uint32_t* dest_buffer, uint32_t len);

/**
 * @brief Executes a transacton composed of multiple command segments.
 *        Each segment already contains the Information about the length and the 
 *        src/dest buffer concerned. Therefore no need for length parameter here.
 *        /!\ Caution: please be consistent with the length of src_buffer/dest_buffer
 *                     and the lenghts specified for the segments.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param segments An array of command segments
 * @param segments_len The size of segments array
 * @param src_buffer An initialized buffer/array with all the data to send
 * @param dest_buffer An initialized buffer/array to store the received data
 * @return SPI_CODE_IDX_INVAL     if spi.idx not valid
 * @return SPI_CODE_NOT_INIT      if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_SEGMENT_INVAL if segments contains an invalid segment
 * @return SPI_CODE_OK            if success
 */
spi_codes_e spi_execute(spi_t* spi, const spi_segment_t* segments, 
                        uint32_t segments_len, const uint32_t* src_buffer, 
                        uint32_t* dest_buffer);

/**
 * @brief Executes a TX command. This is Non-Blocking, the function will return 
 *        immediately.
 *        /!\ Caution: len is in bytes.
 *
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param src_buffer An initialized buffer/array with all the data to send
 * @param len The size in bytes of the transaction (/!\ must be coherent with 
 *            src_buffer size)
 * @param done_cb A callback function of type spi_cb_t that gets called when 
 *                transaction is done
 * @param error_cb A callback function of type spi_cb_t that gets called when there 
 *                 is an error during transaction
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_transmit_nb(spi_t* spi, const uint32_t* src_buffer, uint32_t len, 
                            spi_callbacks_t callbacks);

/**
 * @brief Executes an RX command. This is Non-Blocking, the function will return 
 *        immediately.
 *        /!\ Caution: len is in bytes.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param dest_buffer An initialized buffer/array to store the received data
 * @param len The size in bytes of the transaction (/!\ must be coherent with 
 *            dest_buffer size)
 * @param done_cb A callback function of type spi_cb_t that gets called when 
 *                transaction is done
 * @param error_cb A callback function of type spi_cb_t that gets called when there 
 *                 is an error during transaction
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_receive_nb(spi_t* spi, uint32_t* dest_buffer, uint32_t len, 
                           spi_callbacks_t callbacks);

/**
 * @brief Executes a Bidirectional (TX and RX) command. This is Non-Blocking, the 
 *        function will return immediately.
 *        /!\ Caution: len is in bytes and is for the TX as well as RX.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param src_buffer An initialized buffer/array with all the data to send
 * @param dest_buffer An initialized buffer/array to store the received data
 * @param len The size in bytes of the transaction (/!\ must be coherent with 
 *            src_buffer and dest_buffer size)
 * @param done_cb A callback function of type spi_cb_t that gets called when 
 *                transaction is done
 * @param error_cb A callback function of type spi_cb_t that gets called when there 
 *                 is an error during transaction
 * @return SPI_CODE_IDX_INVAL if spi.idx not valid
 * @return SPI_CODE_NOT_INIT  if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_OK        if success
 */
spi_codes_e spi_transceive_nb(spi_t* spi, const uint32_t* src_buffer, 
                              uint32_t* dest_buffer, uint32_t len, 
                              spi_callbacks_t callbacks);

/**
 * @brief Executes a transacton composed of multiple command segments. This is 
 *        Non-Blocking, the function will return immediately.
 *        Each segment already contains the Information about the length and the 
 *        src/dest buffer concerned. Therefore no need for length parameter here.
 *        /!\ Caution: please be consistent with the length of src_buffer/dest_buffer
 *                     and the lenghts specified for the segments.
 * 
 * @param spi Pointer to spi_t structure obtained through spi_init call
 * @param segments An array of command segments
 * @param segments_len The size of segments array
 * @param src_buffer An initialized buffer/array with all the data to send
 * @param dest_buffer An initialized buffer/array to store the received data
 * @param done_cb A callback function of type spi_cb_t that gets called when 
 *                transaction is done
 * @param error_cb A callback function of type spi_cb_t that gets called when there 
 *                 is an error during transaction
 * @return SPI_CODE_IDX_INVAL     if spi.idx not valid
 * @return SPI_CODE_NOT_INIT      if spi.init false (indicates if spi was initialized)
 * @return SPI_CODE_SEGMENT_INVAL if segments contains an invalid segment
 * @return SPI_CODE_OK            if success
 */
spi_codes_e spi_execute_nb(spi_t* spi, const spi_segment_t* segments, 
                           uint32_t segments_len, const uint32_t* src_buffer, 
                           uint32_t* dest_buffer, spi_callbacks_t callbacks);

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/


#ifdef __cplusplus
}
#endif

#endif // _SDK_SPI_H_

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
