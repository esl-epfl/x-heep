/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
                            <tommaso.terzano@gmail.com>

    Info: im2col_lib.c describes functions used to calculate im2col and verify it using 
    the golden result in im2colGolden.c.

    Notes: im2col_nchw_int32() algorithm is inspired from the library SHL, developed by T-HEAD Semi.
    For reference, check out the following link:
    https:/*github.com/T-head-Semi/csi-nn2/blob/main/source/reference/im2col.c
*/

#include "im2col_lib.h"

#if INPUT_DATATYPE == 2
    typedef uint8_t data_t;
#elif INPUT_DATATYPE == 1
    typedef uint16_t data_t;
#elif INPUT_DATATYPE == 0
    typedef uint32_t data_t;
#endif

data_t output_data[OH_NCHW*OW_NCHW];
data_t* input_image_ptr = &input_image_nchw[0];
data_t* output_data_ptr = &output_data[0];

char im2col_done = 0;
int ifr_status;

void handler_irq_im2col_spc( void )
{
  im2col_done = 1;

  /* Read the IFR to lower the interrupt flag */
  ifr_status = * (volatile uint32_t * )(EXT_PERIPHERAL_START_ADDRESS + IM2COL_PER_OFFSET + IM2COL_SPC_SPC_IFR_REG_OFFSET);
  return;
}

/* Used to wait for the SPC interrupt handler to end */
__attribute__ ((optimize("O0"))) void waiting_for_spc_irq( void )
{
  while (im2col_done == 0)
  {
    asm volatile ("wfi");
  }
}

int im2col_nchw_int32(uint8_t test_id, unsigned int *cycles)
{
    int size_transfer = 0;
    int size_transfer_d2 = 0;
    int counter = 0;
    int n_zeros_left = 0;
    int n_zeros_right = 0;
    int n_zeros_top = 0;
    int n_zeros_bottom = 0;
    int im_row = 0;
    int im_col = 0;
    int im_c = 0; 
    int w_offset = 0;  
    int h_offset = 0;
    int col_index = 0;
    int minimum = 0;
    int start_max = 0;
    int start_min = 0;
    int stray_elements = 0;
    int last_position = 0;
    int tmp_pad = 0;
    int w_offset_pad = 0;
    int h_offset_pad = 0;
    int fw_minus_w_offset = 0;
    int fh_minus_h_offset = 0;
    unsigned int cycles_A = 0;
    unsigned int cycles_B = 0;
    unsigned int avg_first_zeros;
    unsigned int avg_last_zeros;
    unsigned int avg_patch = 0;
    int src_inc_d2 = 0;
    int w_offset_counter = 0;
    int h_offset_counter = 0;
    int h_offset_tmp_counter = 0;
    int im_c_counter = 0;
    int w_offset_vs_PAD = 0;
    int h_offset_vs_PAD = 0;
    int fw_minus_w_offset_vs_PAD = 0;
    int fh_minus_h_offset_vs_PAD = 0;
    int h_offset_tmp = 0;
    int n_zeros_top_counter = 0;
    int n_zeros_top_cond = 0;
    int pad_min_w_offset = 0;

    for (int i=0; i<OH_NCHW*OW_NCHW; i++)
    {
        output_data[i] = 0;
    }

    #if TIMING
        timer_cycles_init();
    #endif

    /* Implementation of im2col algorithm using the CPU */
    if (test_id == 0)
    {
        #if TIMING
        timer_start();
        #endif

        for (int c = 0; c < CH_COL; ++c) {
            /* Calculate offsets within the kernel window. */
            /* These are used to move the filter around the input image */

            w_offset = c % FW;  
            h_offset = (c / FW) % FH;
            
            /* CH on which the im2col is being performed depending on the row of the output image (c) */
            im_c = c / (FH * FW); 

            /* Iterate over each BATCH. */
            for (int b = 0; b < BATCH; ++b) {
                /* Iterate over each patch on the IW of the input matrix.*/
                for (int h = 0; h < N_PATCHES_H; ++h) {
                    /* Iterate over each patch on the heigth in the output matrix.*/
                    for (int w = 0; w < N_PATCHES_W; ++w) {
                        /* Calculate the row and column indices in the original input image, applying the stride and offset. */
                        im_row = h_offset + h * STRIDE_D2 - TOP_PAD;
                        im_col = w_offset + w * STRIDE_D1 - LEFT_PAD;

                        /* Calculate the index in the flattened output array where this value should be stored. */
                        col_index = ((c * BATCH + b) * N_PATCHES_H + h) * N_PATCHES_W + w;
                        PRINTF_DEB("\n\rim_row: %d, im_col: %d,", im_row, im_col);
                        PRINTF_DEB("\n\rw_offset: %d, h_offset: %d\n\r", w_offset, h_offset);

                        /* If the calculated indices are outside the bounds of the input image, perform padding. */
                        /* Otherwise, fetch the value from the input image and store it in the output array. */
                        if (im_row < 0 || im_col < 0 || im_row >= IH || im_col >= IW) {
                            output_data[col_index] = 0;
                            PRINTF_DEB("Padding with 0\n\r");
                        } else {
                            output_data[col_index] = input_image_nchw[get_index(CH, IH, IW, b, im_c, im_row, im_col)];                        
                            PRINTF_DEB("Value: %d\n\r", input_image_nchw[get_index(CH, IH, IW, b, im_c, im_row, im_col)]);
                        }
                    }
                }
            }
        }

        #if TIMING  
        *cycles = timer_stop();
        #endif
    }

    /* Implementation of the nchw im2col algorithm using the DMA 2D feature */
    else if (test_id == 1)
    {
      
        data_t* input_image_ptr = &input_image_nchw[0];
        data_t* output_data_ptr = &output_data[0];
        /* Iterate over each row of the output matrix. */

        dma_config_flags_t res;

        static dma_target_t tgt_src = {
                                    .ptr        = input_image_nchw,
                                    .inc_d1_du  = STRIDE_D1,
                                    .type       = INPUT_DATATYPE
                            };

        static dma_target_t tgt_dst = {
                                    .inc_d1_du  = 1,
                                    .inc_d2_du  = 1,
                                    .type       = INPUT_DATATYPE
                                    };

        static dma_trans_t trans = {
                                    .src        = &tgt_src,
                                    .dst        = &tgt_dst,
                                    .mode       = DMA_TRANS_MODE_SINGLE,
                                    .dim        = DMA_DIM_CONF_2D,
                                    .end        = DMA_TRANS_END_INTR,
                                    .channel    = 0
                                    };

        #if TIMING
        timer_start();
        #endif

        /* The DMA is initialized (i.e. Any current transaction is cleaned.) */
        dma_init(NULL); 

        w_offset = 0;
        h_offset = 0;
        pad_min_w_offset = LEFT_PAD;

        for (int c = 0; c < CH_COL; ++c) {

            /* Iterate over each patch on the IW of the input matrix. */
            fw_minus_w_offset = FW - 1 - w_offset;
            fh_minus_h_offset = FH - 1 - h_offset;

            /* Calculate offsets within the kernel window. */

            /* Iterate over each BATCH. */
            for (int32_t b = 0; b < BATCH; ++b) {

                im_row = h_offset - TOP_PAD;
                im_col = w_offset - LEFT_PAD;
                
                PRINTF_DEB("\n\rim_row: %d, im_col: %d", im_row, im_col);
                PRINTF_DEB("\n\rw_offset: %d, h_offset: %d\n\r", w_offset, h_offset);

                /* Computing the number of zeros on the left */

                /* In the offset of the element in the filter is bigger than P, then no zeros are needed */
                if ( w_offset >= LEFT_PAD)
                {
                    n_zeros_left = 0;
                }
                else if ( (LEFT_PAD - w_offset) % STRIDE_D1 == 0 )
                {
                    n_zeros_left = (LEFT_PAD - w_offset) / STRIDE_D1;
                }
                else
                {
                    n_zeros_left = (LEFT_PAD - w_offset) / STRIDE_D1 + 1;
                }

                /* Computing the number of zeros on the top */

                /* In the offset of the element in the filter is bigger than P, then no zeros are needed */
                if ( h_offset >= TOP_PAD)
                {
                    n_zeros_top = 0;
                }
                else if ( (TOP_PAD - h_offset) % STRIDE_D2 == 0 )
                {
                    n_zeros_top = (TOP_PAD - h_offset) / STRIDE_D2;
                }
                else
                {
                    n_zeros_top = (TOP_PAD - h_offset) / STRIDE_D2 + 1;
                }

                /* Computing the number of zeros on the right */

                /* To adapt the final case to the formulas used to the first padded region, let's compute an "adapted" padded region,
                /* by removing the elements of the row uncovered by the sliding filter */
                
                if (fw_minus_w_offset >= RIGHT_PAD || ADPT_PAD_RIGHT == 0)
                {
                    n_zeros_right = 0;
                }
                else if ( (ADPT_PAD_RIGHT - (fw_minus_w_offset)) % STRIDE_D1 == 0 )
                {
                    n_zeros_right = (ADPT_PAD_RIGHT - (fw_minus_w_offset)) / STRIDE_D1;
                }
                else
                {
                    n_zeros_right = (ADPT_PAD_RIGHT - (fw_minus_w_offset)) / STRIDE_D1 + 1;
                }

                /* Computing the number of zeros on the bottom */

                /* To adapt the final case to the formulas used to the first padded region, let's compute an "adapted" padded region,
                /* by removing the elements of the row uncovered by the sliding filter */

                if (fh_minus_h_offset >= BOTTOM_PAD || ADPT_PAD_BOTTOM == 0)
                {
                    n_zeros_bottom = 0;
                }
                else if ( (ADPT_PAD_BOTTOM - (fh_minus_h_offset)) % STRIDE_D2 == 0)
                {
                    n_zeros_bottom = (ADPT_PAD_BOTTOM - (fh_minus_h_offset)) / STRIDE_D2;
                }
                else
                {
                    n_zeros_bottom = (ADPT_PAD_BOTTOM - (fh_minus_h_offset)) / STRIDE_D2 + 1;
                }

                /* Compute the number of elements to transfer */
                size_transfer = N_PATCHES_W - n_zeros_left - n_zeros_right;
                size_transfer_d2 = N_PATCHES_H - n_zeros_top - n_zeros_bottom;

                PRINTF_DEB("\n\rn_zeros_left: %d, n_zeros_right: %d, n_zeros_top: %d, n_zeros_bottom: %d", n_zeros_left, n_zeros_right, n_zeros_top, n_zeros_bottom);
                PRINTF_DEB("\n\rsize_transfer: %d, size_transfer_d2: %d\n\r", size_transfer, size_transfer_d2);
    
                /* DMA setup and transaction run */
                int index = get_index(CH, IH, IW, b, im_c, im_row + n_zeros_top*STRIDE_D2, im_col + n_zeros_left*STRIDE_D1);
                src_inc_d2 = (STRIDE_D2 * IW - (size_transfer - 1 + (STRIDE_D1 - 1) * (size_transfer - 1)));

                PRINTF_DEB("\n\rindex: %d, src_inc_d2: %d", index, src_inc_d2);
                
                input_image_ptr = &input_image_nchw[0] + index;
                PRINTF_DEB("\n\rsrc_ptr: %x dst_ptr: %x\n\r", input_image_ptr, output_data_ptr);

                tgt_src.ptr = input_image_ptr;
                tgt_src.inc_d2_du = src_inc_d2;

                tgt_dst.ptr = output_data_ptr;

                trans.src = &tgt_src;
                trans.dst = &tgt_dst;
                trans.size_d1_du = size_transfer;
                trans.size_d2_du = size_transfer_d2;
                trans.pad_top_du = n_zeros_top;
                trans.pad_left_du = n_zeros_left;
                trans.pad_right_du = n_zeros_right;
                trans.pad_bottom_du = n_zeros_bottom;
                
                dma_run(&trans);

                output_data_ptr += OW_NCHW;

                PRINTF_DEB("\n\r");

                #ifndef DEBUG
                PRINTF_DEB("\n\rCurrent output matrix: \n\r");
                for (int i=0; i<OH_NCHW; i++)
                {
                    for (int j=0; j<OW_NCHW; j++)
                    {
                        PRINTF_DEB("%d ", output_data[i*OW_NCHW + j]);
                    }
                    PRINTF_DEB("\n\r");
                }
                #endif
            }
                PRINTF_DEB("\n\r");
            
            /* Optimized w_offset computation: w_offset = c % FW */
            if (w_offset == FW - 1)
            {
                w_offset = 0;
            } 
            else
            {
                w_offset++;
            }

            /* Optimized h_offset computation: h_offset = (h_offset_tmp) % FH with h_offset_tmp = c / FW */
            if (h_offset_counter == FW - 1)
            {
                h_offset_counter = 0;
                if (h_offset == FH - 1)
                {
                    h_offset = 0;
                } 
                else
                {
                    h_offset++;
                }
            } 
            else
            {
                h_offset_counter++;
            }

            /* Optimized im_c computation: im_c = c / FW*FH */
            if (im_c_counter == FH*FW - 1)
            {
                im_c_counter = 0;
                im_c++;
            } 
            else
            {
                im_c_counter++;
            }     
        }

        #if TIMING  
        *cycles = timer_stop();
        #endif
    }

    /* Implementation of im2col algorithm using the dedicated Smart Peripheral Controller */
    else if (test_id == 2)
    {
        input_image_ptr = &input_image_nchw[0];
        output_data_ptr = &output_data[0];

        #if TIMING
        timer_start();
        #endif

        im2col_spc_init(NULL);

        static im2col_trans_t im2col_spc_trans = {
          .ch_mask = SPC_CH_MASK,
          .im_width = IW,
          .im_height = IH,
          .filter_width = FW,
          .filter_height = FH,
          .num_channels = CH,
          .num_channels_col = CH_COL,
          .stride_d1 = STRIDE_D1,
          .stride_d2 = STRIDE_D2,
          .batch = BATCH,
          .n_patches_w = N_PATCHES_W,
          .n_patches_h = N_PATCHES_H,
          .left_pad = LEFT_PAD,
          .right_pad = RIGHT_PAD,
          .top_pad = TOP_PAD,
          .bottom_pad = BOTTOM_PAD,
          .adpt_pad_right = ADPT_PAD_RIGHT,
          .adpt_pad_bottom = ADPT_PAD_BOTTOM,
          .datatype = INPUT_DATATYPE
        };

        im2col_spc_trans.src = input_image_ptr;
        im2col_spc_trans.dst = output_data_ptr;

        im2col_spc_run(im2col_spc_trans);

        waiting_for_spc_irq();

        #if TIMING  
        *cycles = timer_stop();
        #endif
    }
    /* Finished! */

    #if DEBUG
    PRINTF("Final output matrix:\n\r\n\r");
    for (int i=0; i<OH_NCHW; i++)
    {
        for (int j=0; j<OW_NCHW; j++)
        {
            PRINTF("%d ", output_data[i*OW_NCHW + j]);
        }
        PRINTF("\n\r");
    }
    #endif

    /* Return a 0 to indicate a success */
    return 0;
}

int get_index(int dim1, int dim2, int dim3, int index0, int index1, int index2,
                          int index3)
{
    return ((index0 * dim1 + index1) * dim2 + index2) * dim3 + index3;
}

/* Verifies the im2col using golden values generated by "verification_script.py" */
int verify()
{
  int errors = 0;

  for (int i=0; i<OH_NCHW; i++)
      {
          for (int j=0; j<OW_NCHW; j++)
          {    
              if (golden_im2col_nchw[i*OW_NCHW + j] != output_data[i*OW_NCHW + j])
              {
                  PRINTF("ERROR: Golden: %d, Output: %d, at %d %d %x\n\r", golden_im2col_nchw[i*OW_NCHW + j], output_data[i*OW_NCHW + j], i, j, &output_data[i*OW_NCHW + j]);
                  errors ++;
              }
          }
      }
    
    return errors;
}

void dma_run(dma_trans_t * trans)
{
    int res = dma_validate_transaction(trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF_DEB("DMA validation result: %d\n\r", res);
    res = dma_load_transaction(trans);
    PRINTF_DEB("DMA load result: %d\n\r", res);
    res = dma_launch(trans);
    PRINTF_DEB("DMA launch result: %d\n\r", res);

    while( ! dma_is_ready(0)) {
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            asm volatile("wfi");
            /* From here the core wakes up even if we did not jump to the ISR */
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}
