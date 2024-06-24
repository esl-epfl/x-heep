#ifndef IM2COL_SPC_H
#define IM2COL_SPC_H

/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 *  Info: This is the header of the driver library of the im2col SPC (Smart Peripheral Controller). 
 *        It defines functions and structures to easly define, verify and launch the im2col SPC.
 * 
 */

/* Base address of the im2col SPC */
#define IM2COL_SPC_BASE_ADDR EXT_PERIPHERAL_START_ADDRESS + 0x4000

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

typedef enum
{
    IM2COL_SPC_CONFIG_OK               = 0x0000, /*!< Im2col transfer was successfully
    configured. */
    IM2COL_SPC_CONFIG_SRC              = 0x0001, /*!< An issue was encountered in the
    source arrangement.  */
    IM2COL_SPC_CONFIG_DST              = 0x0002, /*!< An issue was encountered in the
    destination arrangement. */
    IM2COL_SPC_CONFIG_MISALIGN         = 0x0004, /*!< An arrangement is misaligned. */
    IM2COL_SPC_CONFIG_OVERLAP          = 0x0008, /*!< The increment is smaller than the
     data type size. */
    IM2COL_SPC_CONFIG_DISCONTINUOUS    = 0x0010, /*!< The increment is larger than the
    data type size. */
    IM2COL_SPC_CONFIG_OUTBOUNDS        = 0x0020, /*!< The operation goes beyond the
    memory boundaries. */
    IM2COL_SPC_CONFIG_INCOMPATIBLE     = 0x0040, /*!< Different arguments result in
    incompatible requests. */
    IM2COL_SPC_CONFIG_TRANS_OVERRIDE   = 0x0100, /*!< A transaction is running. Its
    values cannot be modified, nor can it be re-launched. */
    IM2COL_SPC_CONFIG_CRITICAL_ERROR   = 0x0200, /*!< This flag determines the function
    will return without the im2col performing any actions. */
} im2col_spc_config_flags_t;

/**
 * In some cases the im2col can overcome a misalignment issue if the data type is
 * set to a smaller size.
 * This action can be performed by the im2col_configure() function if allowed by
 * the user.
 */
typedef enum
{
    IM2COL_SPC_DO_NOT_ENABLE_REALIGN  = 0, /*!< If a misalignment is detected, it will
    be treated as an error. */
    IM2COL_SPC_ENABLE_REALIGN         = 1, /*!< If a misalignment is detected, the im2col spc
    HAL will try to overcome it. */
    IM2COL_SPC_ENABLE_REALIGN__size,       /*!< Not used, only for sanity checks. */
} im2col_spc_en_realign_t;

/**
 * It is possible to choose the level of safety with which the im2col operation
 * should be configured.
 * Not performing checks reduces the im2col setup overhead, but may result in a
 * faulty operation, especially if the configurations set are not
 * fixed but rather depend on the circumstance.
 * e.g. The source pointer is obtained as a result of a loop. It could happen
 * the pointer ends up pointing outside the memory range, or that the pointer
 * is close enough the the memory end to cause an overflow during reading.
 */
typedef enum{
    IM2COL_SPC_PERFORM_CHECKS_ONLY_SANITY = 0, /*!< No checks will be performed.
    Only sanity checks will be performed that no values are off-limits or
    containing errors. */
    IM2COL_SPC_PERFORM_CHECKS_INTEGRITY   = 1, /*!< Sanity AND integrity of the
    parameters is checked to make sure there are no inconsistencies.
    Not using this flag is only recommended when parameters are constant and
    the proper operation has been previously tested. */
    IM2COL_SPC_PERFORM_CHECKS__size,       /*!< Not used, only for sanity checks. */
} im2col_spc_perf_checks_t;

/**
 * Different possible actions that determine the end of the im2col transaction.
 * This choice does not affect the transaction, but only the way the
 * application is notified of its finalization.
 */
typedef enum
{
    IM2COL_SPC_TRANS_END_POLLING,   /*!< Interrupt for the im2col SPC will be disabled. The
    application will be in charge of monitoring the end of the transaction.*/
    IM2COL_SPC_TRANS_END_INTR,      /*!< Interrupt for the im2col SPC will be enabled. After
    launching the transaction, the im2col_spc_launch function will exit. */
    IM2COL_SPC_TRANS_END_INTR_WAIT, /*!< Interrupt for the im2col SPC will be enabled. After
     launching the transaction, the im2col SPC_launch function will wait in a
     wait_for_interrupt (wfi) state. */
    IM2COL_SPC_TRANS_END__size,     /*!< Not used, only for sanity checks. */
} im2col_spc_trans_end_evt_t;

/**
 * An environment is a region of memory defined by its start and end pointers.
 * The sole purpose of creating environments is preventing the im2col SPC from writing
 * on restricted memory regions (outside the environment).
 */
typedef struct
{
    uint8_t *start; /*!< Pointer to the start of the environment. */
    uint8_t *end;   /*!< Pointer to the last byte inside the environment. */
} im2col_spc_env_t;

#endif