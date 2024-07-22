/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 *  Info: This is the driver library of the im2col SPC (Smart Peripheral Controller). 
 *        It defines functions and structures to easly define, verify and launch the im2col SPC.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "im2colGolden.h"
#include "dma.h"
#include "im2col_spc_regs.h"
#include "im2col_spc.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "rv_plic.h"
#include "csr.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Structure representing the parameters for the im2col transformation.
 *
 * This structure holds the necessary parameters for performing the im2col operation
 * on an input matrix. The im2col transformation is commonly used in convolutional neural
 * networks to convert the input matrix into a column matrix, which simplifies the
 * convolution operation.
 */
typedef struct {
  uint32_t* src_ptr; /**< Pointer to start address from where data will be processed */
  uint32_t* dst_ptr; /**< Pointer to destination of the processed data */
  uint8_t num_ch; /**< Number of DMA channels that the SPC has access to */
  dma_data_type_t datatype; /**< Datatype of the data to be copied */
  uint8_t filter_height; /**< Height of the filter */
  uint8_t filter_width; /**< Width of the filter */
  uint32_t input_height; /**< Height of the input matrix */
  uint32_t input_width; /**< Width of the input matrix */
  uint32_t ch_col; /**< Number of channels in the output matrix */
  uint32_t n_patches_width; /**< Number of patches in the width direction */
  uint32_t n_patches_height; /**< Number of patches in the height direction */
  uint32_t pad_top_du; /**< Top padding */
  uint32_t pad_bottom_du; /**< Bottom padding */
  uint32_t pad_left_du; /**< Left padding */
  uint32_t pad_right_du; /**< Right padding */
  uint32_t stride_d1; /**< Stride in the first dimension */
  uint32_t stride_d2; /**< Stride in the second dimension */
  uint32_t batch; /**< Number of batches */
  uint32_t channels; /**< Number of channels */
  uint32_t adapted_pad_right; /**< Right padding after the adaptation */
  uint32_t adapted_pad_bottom; /**< Bottom padding after the adaptation */
  im2col_spc_trans_end_evt_t end; /**< End event */
  im2col_spc_config_flags_t  flags;  /*!< A mask with possible issues aroused from
  the creation of the transaction. */
  im2col_spc_env_t* src_env; /**< Environment of the source */
  im2col_spc_env_t* dst_env; /**< Environment of the destination */
} im2col_spc_trans_t;


/**
 * @brief Writes a given value into the specified register. Its operation
 * mimics that of bitfield_field32_write(), but does not require the use of
 * a field structure, that is not always provided in the _regs.h file.
 * @param p_val The value to be written.
 * @param p_offset The register's offset from the peripheral's base address
 *  where the target register is located.
 * @param p_mask The variable's mask to only modify its bits inside the whole
 * register.
 * @param p_sel The selection index (i.e. From which bit inside the register
 * the value is to be written).
 */
static inline volatile void write_register( uint32_t  p_val,
                                uint32_t  p_offset,
                                uint32_t  p_mask,
                                uint8_t   p_sel) 
{
    uint32_t* addr  =  (IM2COL_SPC_BASE_ADDR + p_offset);
    /*
     * An intermediate variable "value" is used to prevent writing twice into
     * the register.
     */
    uint32_t value  =  *addr;
    value           &= ~( p_mask << p_sel );
    value           |= (p_val & p_mask) << p_sel;
    *addr = value;
};

/**
 * @brief Reads the value from a specific register.
 *
 * This function reads the value from a specific register by adding the register offset
 * to the base address of the IM2COL_SPC module.
 *
 * @param reg_offset The offset of the register to read.
 * @return The value read from the register.
 */
static inline volatile read_register(uint32_t reg_offset)
{
  return * (volatile uint32_t * )(IM2COL_SPC_BASE_ADDR + reg_offset);
}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/


static inline uint8_t is_region_outbound(   im2col_spc_trans_t * spc_trans)
{
  /* This function computes the output dimension of an im2col operation
   * and checks if it is within the bounds of the destination environment.
   */

  uint32_t out_width = spc_trans->channels * spc_trans->filter_height * spc_trans->filter_width * spc_trans->batch;
  uint32_t out_height = spc_trans->n_patches_height * spc_trans->n_patches_width;

  uint32_t out_size = out_width * out_height;

  uint32_t rangeSize          = DMA_DATA_TYPE_2_SIZE(spc_trans->datatype) * out_size;
  uint32_t lastByteInsideRange = (uint32_t)spc_trans->dst_env->start + rangeSize -1;
  return ( spc_trans->dst_env->end < lastByteInsideRange );
}

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void im2col_spc_init(void)
{
  /* Reset the SPC */

  /* Write the number of DMA channels the SPC has access to */
  write_register( 0,
                  IM2COL_SPC_SPC_CH_MASK_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the source */
  write_register( 0,
                  IM2COL_SPC_SRC_PTR_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write the destination */
  write_register( 0,
                  IM2COL_SPC_DST_PTR_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the datatype */
  write_register( 0,
                  IM2COL_SPC_DATA_TYPE_REG_OFFSET,
                  IM2COL_SPC_DATA_TYPE_DATA_TYPE_MASK,
                  IM2COL_SPC_DATA_TYPE_DATA_TYPE_OFFSET);
  
  /* Write the filter dimensions */
  write_register( 0,
                  IM2COL_SPC_FW_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( 0,
                  IM2COL_SPC_FH_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the image dimensions */
  write_register( 0,
                  IM2COL_SPC_IW_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( 0,
                  IM2COL_SPC_IH_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write the CH_COL */
  write_register( 0,
                  IM2COL_SPC_CH_COL_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write n_patches */
  write_register( 0,
                  IM2COL_SPC_N_PATCHES_W_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( 0,
                  IM2COL_SPC_N_PATCHES_H_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the padding */
  write_register( 0,
                  IM2COL_SPC_PAD_LEFT_REG_OFFSET,
                  IM2COL_SPC_PAD_LEFT_PAD_MASK,
                  IM2COL_SPC_PAD_LEFT_PAD_OFFSET);
  
  write_register( 0,
                  IM2COL_SPC_PAD_RIGHT_REG_OFFSET,
                  IM2COL_SPC_PAD_RIGHT_PAD_MASK,
                  IM2COL_SPC_PAD_RIGHT_PAD_OFFSET);
  
  write_register( 0,
                  IM2COL_SPC_PAD_TOP_REG_OFFSET,
                  IM2COL_SPC_PAD_TOP_PAD_MASK,
                  IM2COL_SPC_PAD_TOP_PAD_OFFSET);

  write_register( 0,
                  IM2COL_SPC_PAD_BOTTOM_REG_OFFSET,
                  IM2COL_SPC_PAD_BOTTOM_PAD_MASK,
                  IM2COL_SPC_PAD_BOTTOM_PAD_OFFSET);
  
  /* 
    * Write the strides. With respect to test_2 these are the application-point-of-view
    * strides, so they are the same as STRIDE_D1 and STRIDE_D2.
    */
  write_register( 0,
                  IM2COL_SPC_LOG_STRIDES_D1_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( 0,
                  IM2COL_SPC_LOG_STRIDES_D2_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the batch size */
  write_register( 0,
                  IM2COL_SPC_BATCH_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write the adapted pad regions */
  write_register( 0,
                  IM2COL_SPC_ADPT_PAD_RIGHT_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( 0,
                  IM2COL_SPC_ADPT_PAD_BOTTOM_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Enable the interrupt logic */
  write_register( 0,
                  IM2COL_SPC_INTERRUPT_EN_REG_OFFSET,
                  0x1,
                  IM2COL_SPC_INTERRUPT_EN_EN_BIT);
  
  /* Write the number of channels to start the process */
  write_register( 0,
                  IM2COL_SPC_NUM_CH_REG_OFFSET,
                  0xffffffff,
                  0);
}

im2col_spc_config_flags_t im2col_spc_validate(im2col_spc_trans_t *spc_trans,
                                              im2col_spc_en_realign_t spc_enRealign,
                                              im2col_spc_perf_checks_t  spc_check )
{
  /*
    * SANITY CHECKS
    */

    /* Data type is not necessary. If something is provided anyways it should
     be valid.*/
    IM2COL_SPC_STATIC_ASSERT( spc_trans->datatype   < DMA_DATA_TYPE__size,
                       "Data type not valid");
    /* The end event should be a valid end event. */
    IM2COL_SPC_STATIC_ASSERT( spc_trans->end    < IM2COL_SPC_TRANS_END__size,
                       "End event not valid");
    /* The alignment permission should be a valid permission. */
    IM2COL_SPC_STATIC_ASSERT( spc_enRealign     < IM2COL_SPC_ENABLE_REALIGN__size,
                       "Alignment not valid");
    /* The checks request should be a valid request. */
    IM2COL_SPC_STATIC_ASSERT( spc_check         < IM2COL_SPC_PERFORM_CHECKS__size,
                       "Check request not valid");
    /* The padding should be a valid number */
    IM2COL_SPC_STATIC_ASSERT( ((spc_trans->pad_top_du >= 0 && spc_trans->pad_top_du < 64) && 
                        (spc_trans->pad_bottom_du >= 0 && spc_trans->pad_bottom_du < 64) && 
                        (spc_trans->pad_left_du >= 0 && spc_trans->pad_left_du < 64) &&
                        (spc_trans->pad_right_du >= 0 && spc_trans->pad_right_du < 64)), 
                       "Padding not valid");

    /*
     * CHECK IF TARGETS HAVE ERRORS
     */

    /*
     * The transaction is NOT created if the targets include errors.
     * A successful target validation has to be done before loading it to the
     * SPC.
     */
    uint8_t errorSrc = validate_target( spc_trans->src_ptr );
    uint8_t errorDst = validate_target( spc_trans->dst_ptr );

    /*
     * If there are any errors or warnings in the valdiation of the targets,
     * they are added to the transaction flags, adding the source/destination
     * identifying flags as well.
     */
    spc_trans->flags |= errorSrc ? (errorSrc | IM2COL_SPC_CONFIG_SRC ) : IM2COL_SPC_CONFIG_OK;
    spc_trans->flags |= errorDst ? (errorDst | IM2COL_SPC_CONFIG_SRC ) : IM2COL_SPC_CONFIG_OK;

    /* If a critical error was detected, no further action is performed. */
    if( spc_trans->flags & IM2COL_SPC_CONFIG_CRITICAL_ERROR )
    {
        return spc_trans->flags;
    }

    /*
     * COMPUTE THE PATCHES
     */

    spc_trans->n_patches_width = (spc_trans->input_height + (spc_trans->pad_top_du + spc_trans->pad_bottom_du) - spc_trans->filter_height)/spc_trans->stride_d2 + 1;
    spc_trans->n_patches_height = (spc_trans->input_width + (spc_trans->pad_left_du + spc_trans->pad_right_du) - spc_trans->filter_width)/spc_trans->stride_d1 + 1;


    /*
     * CHECK IF THERE ARE MISALIGNMENTS
     */

    if( spc_check )
    {
        /*
         * The source and destination targets are analyzed.
         * If the target is a peripheral (i.e. uses one of trigger slots)
         * then the misalignment is not checked.
         */
        uint8_t misalignment = 0;
        uint8_t dstMisalignment = 0;

        misalignment = get_misalignment_b( spc_trans->src_ptr, spc_trans->datatype );

        dstMisalignment = get_misalignment_b( spc_trans->dst_ptr, spc_trans->datatype );

        spc_trans->flags  |= ( misalignment ? IM2COL_SPC_CONFIG_SRC : IM2COL_SPC_CONFIG_OK );
        spc_trans->flags  |= ( dstMisalignment ? IM2COL_SPC_CONFIG_DST : IM2COL_SPC_CONFIG_OK );

        /* Only the largest misalignment is preserved.*/
        if( misalignment < dstMisalignment )
        {
            misalignment = dstMisalignment;
        }

        if( misalignment != 0 )
        {
            /*
             * Misalignment flags will only be stored in the transaction, as
             * they are data type dependent, and could vary from transaction
             * to transaction, even with the same pair of targets.
             */
            spc_trans->flags |= IM2COL_SPC_CONFIG_MISALIGN;

            /*
             * If a misalignment is detected and realignment is not allowed,
             * an error is returned. No operation should be performed by the DMA.
             * No further operations are done to prevent corrupting information
             * that could be useful for debugging purposes.
             */
            if( !spc_enRealign)
            {
                return spc_trans->flags |= IM2COL_SPC_CONFIG_CRITICAL_ERROR;
            }

            /*
             * CHECK IF THERE IS A DISCONTINUITY
             */

            /*
             * If there is a misalignment AND the source or destination
             * arrangements are discontinuous, it is not possible to use the
             * DMA. An error is returned.
             * A discontinuity in an arrangement is defined as the increment
             * being larger than the data type size.
             * e.g.
             * |AAAA|AAAA|BBBB|BBBB|      |____|AAAA|AAAA|____|
             * 0    1    2    3           0    1    2    3
             *
             * |CCCC|CCCC|____|____| ==>  |____|BBBB|BBBB|____|
             * 4    5    6    7           4    5    6    7
             *
             * |____|____|____|____|      |____|CCCC|CCCC|____|
             * 8    9    10   11          8    9    10   11
             *
             * In this case the source arrangement has 16bit HALF WORDs, with
             * an increment of 1 data unit, so all data is continuous.
             * The destination arrangement has also 16bit HALF WORDs, but
             * misaligned and with 2 data units of increment.
             * To copy a misaligned arrangement of HALF WORDs, the DMA should
             * use BYTEs. However, by using bytes it would not be able to
             * write twice and skip twice, as it has a single skip increment.
             *
             * The misalignment and discontinuity can be found in the source
             * and destination respectively and vice versa, and this
             * limitation would still exist.
             *
             * The discontinuous flag is added (the misaligned one was
             * already there), and it is turned into a critical error.'
             *
             * No further operations are done to prevent corrupting
             * information that could be useful for debugging purposes.
             */
            if(( spc_trans->stride_d1 > 1 ))
            {
                spc_trans->flags |= IM2COL_SPC_CONFIG_DISCONTINUOUS;
                spc_trans->flags |= IM2COL_SPC_CONFIG_CRITICAL_ERROR;
                return spc_trans->flags;
            }

            /*
             * PERFORM THE REALIGNMENT
             */

            /*
             * If realignment is allowed and there are no discontinuities,
             * a more granular data type is used according to the detected
             * misalignment in order to overcome it.
             */
            spc_trans->datatype += misalignment;
            /*
             * Source and destination increment should now be of the size
             * of the data.
             * As increments are given in bytes, in both cases should be the
             * size of a data unit.
             */
            spc_trans->stride_d1 = IM2COL_SPC_DATA_TYPE_2_SIZE( spc_trans->datatype );
            /* The copy size does not change, as it is already stored in bytes.*/
        }

        /*
         * CHECK IF THERE IS CROSS-OUTBOUND
         */

        /*
         * As the source target does not know the constraints of the
         * destination target, it is possible that the copied information
         * ends up beyond the bounds of the destination environment.
         * This check is not performed if no destination environment was set.
         *
         * e.g.
         * If 10 HALF WORDs are to be transferred with a HALF WORD in between
         * (increment of 2 data units: writes 2 bytes, skips 2 bytes, ...),
         * then ( 10 data units * 2 bytes ) + ( 9 data units * 2 bytes ) are
         * traveled for each = 38 bytes
         * that need to be gone over in order to complete the transaction.
         * Having the transaction size in bytes, they need to be converted
         * to data units:
         * size [data units] = size [bytes] / convertionRatio [bytes/data unit].
         *
         * The transaction can be performed if the whole affected area can fit
         * inside the destination environment
         * (i.e. The start pointer + the 38 bytes -in this case-, is smaller
         * than the end pointer of the environment).
         *
         * No further operations are done to prevent corrupting information
         * that could be useful for debugging purposes.
         */
        uint8_t isEnv = (spc_trans->dst_env != NULL);

        if(isEnv) {
            uint8_t isOutb = is_region_outbound(spc_trans);
            if( isOutb )
            {
                spc_trans->flags |= IM2COL_SPC_CONFIG_DST;
                spc_trans->flags |= IM2COL_SPC_CONFIG_OUTBOUNDS;
                spc_trans->flags |= IM2COL_SPC_CONFIG_CRITICAL_ERROR;

                return spc_trans->flags;
            }
        }
    }

  return spc_trans->flags;
}

im2col_spc_config_flags_t im2col_spc_run(im2col_spc_trans_t *spc_trans)
{
  
  /* Compute the adapted padded regions */
  spc_trans->adapted_pad_right = spc_trans->stride_d1 * (spc_trans->n_patches_width - 1) + spc_trans->filter_width - (spc_trans->pad_left_du + spc_trans->input_width);
  spc_trans->adapted_pad_bottom = spc_trans->stride_d2 * (spc_trans->n_patches_height - 1) + spc_trans->filter_height - (spc_trans->pad_top_du + spc_trans->input_height);
  //(CH * FH * FW)
  spc_trans->ch_col = spc_trans->channels * spc_trans->filter_height * spc_trans->filter_width;

  /* Load the transaction to the SPC */

  /* Write the number of DMA channels the SPC has access to */
  write_register( spc_trans->num_ch,
                  IM2COL_SPC_SPC_CH_MASK_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the source */
  write_register( spc_trans->src_ptr,
                  IM2COL_SPC_SRC_PTR_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write the destination */
  write_register( spc_trans->dst_ptr,
                  IM2COL_SPC_DST_PTR_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the datatype */
  write_register( spc_trans->datatype,
                  IM2COL_SPC_DATA_TYPE_REG_OFFSET,
                  IM2COL_SPC_DATA_TYPE_DATA_TYPE_MASK,
                  IM2COL_SPC_DATA_TYPE_DATA_TYPE_OFFSET);
  
  /* Write the filter dimensions */
  write_register( spc_trans->filter_width,
                  IM2COL_SPC_FW_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( spc_trans->filter_height,
                  IM2COL_SPC_FH_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the image dimensions */
  write_register( spc_trans->input_width,
                  IM2COL_SPC_IW_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( spc_trans->input_height,
                  IM2COL_SPC_IH_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write the CH_COL */
  write_register( spc_trans->ch_col,
                  IM2COL_SPC_CH_COL_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write n_patches */
  write_register( spc_trans->n_patches_width,
                  IM2COL_SPC_N_PATCHES_W_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( spc_trans->n_patches_height,
                  IM2COL_SPC_N_PATCHES_H_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the padding */
  write_register( spc_trans->pad_left_du,
                  IM2COL_SPC_PAD_LEFT_REG_OFFSET,
                  IM2COL_SPC_PAD_LEFT_PAD_MASK,
                  IM2COL_SPC_PAD_LEFT_PAD_OFFSET);
  
  write_register( spc_trans->pad_right_du,
                  IM2COL_SPC_PAD_RIGHT_REG_OFFSET,
                  IM2COL_SPC_PAD_RIGHT_PAD_MASK,
                  IM2COL_SPC_PAD_RIGHT_PAD_OFFSET);
  
  write_register( spc_trans->pad_top_du,
                  IM2COL_SPC_PAD_TOP_REG_OFFSET,
                  IM2COL_SPC_PAD_TOP_PAD_MASK,
                  IM2COL_SPC_PAD_TOP_PAD_OFFSET);

  write_register( spc_trans->pad_bottom_du,
                  IM2COL_SPC_PAD_BOTTOM_REG_OFFSET,
                  IM2COL_SPC_PAD_BOTTOM_PAD_MASK,
                  IM2COL_SPC_PAD_BOTTOM_PAD_OFFSET);
  
  /* 
    * Write the strides. With respect to test_2 these are the application-point-of-view
    * strides, so they are the same as STRIDE_D1 and STRIDE_D2.
    */
  write_register( spc_trans->stride_d1,
                  IM2COL_SPC_LOG_STRIDES_D1_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( spc_trans->stride_d2,
                  IM2COL_SPC_LOG_STRIDES_D2_REG_OFFSET,
                  0xffffffff,
                  0);

  /* Write the batch size */
  write_register( spc_trans->batch,
                  IM2COL_SPC_BATCH_REG_OFFSET,
                  0xffffffff,
                  0);
  
  /* Write the adapted pad regions */
  write_register( spc_trans->adapted_pad_right,
                  IM2COL_SPC_ADPT_PAD_RIGHT_REG_OFFSET,
                  0xffffffff,
                  0);
  
  write_register( spc_trans->adapted_pad_bottom,
                  IM2COL_SPC_ADPT_PAD_BOTTOM_REG_OFFSET,
                  0xffffffff,
                  0);

  if (spc_trans->end != IM2COL_SPC_TRANS_END_POLLING)
  {
    /* Enable global interrupt. */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8 );

    /* Enable the interrupt logic */
    write_register( 0x1,
                    IM2COL_SPC_INTERRUPT_EN_REG_OFFSET,
                    0x1,
                    IM2COL_SPC_INTERRUPT_EN_EN_BIT);
  }

  /* Write the number of channels to start the process */
  write_register( spc_trans->channels,
                  IM2COL_SPC_NUM_CH_REG_OFFSET,
                  0xffffffff,
                  0);
  
  while(spc_trans->end == IM2COL_SPC_TRANS_END_INTR_WAIT
            && ( read_register(IM2COL_SPC_STATUS_REG_OFFSET) != 0x0 ) ) {
              wait_for_interrupt();
    }
}