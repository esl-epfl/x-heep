/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>

    Info: im2col_lib.c describes functions used to calculate im2col and verify it using 
    the golden result in im2colGolden.c.

    Notes: im2col_nchw_int32() and im2col_nhwc_int32() algorithms are inspired from the library SHL, developed by T-HEAD Semi.
    For reference, check out the following link:
    https:/*github.com/T-head-Semi/csi-nn2/blob/main/source/reference/im2col.c
*/

#include "im2col_lib.h"

int output_data[OH_NCHW*OW_NCHW];

int im2col_nchw_int32(uint8_t test_id, unsigned int *cycles)
{
    int size_transfer = 0;
    int size_transfer_d2 = 0;
    int counter = 0;
    int n_zeros_left = 0;
    int n_zeros_right = 0;
    int n_zeros_top = 0;
    int n_zeros_bottom = 0;
    uint32_t im_row = 0;
    uint32_t im_col = 0;
    int32_t im_c = 0; 
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
    int fw_minus_w_offset = FW - 1 - w_offset;
    int fh_minus_h_offset = FH - 1 - h_offset;
    unsigned int cycles_A = 0;
    unsigned int cycles_B = 0;
    unsigned int avg_first_zeros;
    unsigned int avg_last_zeros;
    unsigned int avg_patch = 0;
    int src_inc_d2 = 0;

    #if TIMING
        CSR_SET_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
        CSR_WRITE(CSR_REG_MCYCLE, 0);
        CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    #endif

    if (test_id == 0)
    {
        #if TIMING  
        CSR_READ(CSR_REG_MCYCLE, &cycles_A);
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
                        im_row = h_offset + h * STRIDES - PAD;
                        im_col = w_offset + w * STRIDES - PAD;

                        /* Calculate the index in the flattened output array where this value should be stored. */
                        col_index = ((c * BATCH + b) * N_PATCHES_H + h) * N_PATCHES_W + w;
                        
                        /* If the calculated indices are outside the bounds of the input image, perform padding. */
                        /* Otherwise, fetch the value from the input image and store it in the output array. */
                        if (im_row < 0 || im_col < 0 || im_row >= IH || im_col >= IW) {
                            output_data[col_index] = 0;
                        } else {
                            output_data[col_index] = input_image_nchw[get_index(CH, IH, IW, b, im_c, im_row, im_col)];                        
                        }
                    }
                }
            }
        }

        #if TIMING  
        CSR_READ(CSR_REG_MCYCLE, &cycles_B);
        *cycles = (cycles_B - cycles_A);
        #endif
    }

    else if (test_id == 1)
    {
        /* Implementation of im2col algorithm using the DMA 1D */

        uint32_t* input_image_ptr = &input_image_nchw[0];
        uint32_t* output_data_ptr = &output_data[0];

        dma_config_flags_t res;

        dma_target_t tgt_src = {
                                    .ptr        = input_image_nchw,
                                    .inc_du     = 1,
                                    .size_du    = IH*IW*CH,
                                    };
        dma_target_t tgt_dst = {
                                    .ptr        = output_data_ptr,
                                    .inc_du     = 1
                                    };

        dma_trans_t trans = {
                                    .src        = &tgt_src,
                                    .dst        = &tgt_dst
                                    };

        uint32_t* ptr; 

        #if TIMING  
        CSR_READ(CSR_REG_MCYCLE, &cycles_A);
        #endif

        /* The DMA is initialized (i.e. Any current transaction is cleaned.) */
        dma_init(NULL);

        for (int c = 0; c < CH_COL; ++c) {
            /* Calculate offsets within the kernel window. */
            /* These are used to move the filter around the input image. */

            w_offset = c % FW; 
            h_offset = (c / FW) % FH; 
            im_c = c / (FH * FW); 

            /* Iterate over each BATCH. */
            for (int32_t b = 0; b < BATCH; ++b) {
                /* Iterate over each patch on the IW of the input matrix. */
                for (int h = 0; h < N_PATCHES_H; ++h) {

                    im_row = h_offset + h * STRIDES - PAD;

                    im_col = w_offset - PAD;

                    n_zeros_left = 0;
                    n_zeros_right = 0;
                    size_transfer = 0;
                    minimum = 0;
                    start_max = 0;
                    start_min = 0;
                    stray_elements = 0;
                    last_position = 0;
                    tmp_pad = 0;

                    PRINTF_DEB("\n\rim_row: %d, im_col: %d", im_row, im_col);
                    PRINTF_DEB("\n\rw_offset: %d, h_offset: %d\n\r", w_offset, h_offset);

                    /* Iterate over each patch on the heigth in the output matrix. */
                    
                    if (PAD > 0 && im_row < 0)
                    {
                        /* im_row < 0 case: only the output_image_ptr needs to be updated */
                        output_data_ptr += N_PATCHES_W;
                        PRINTF_DEB("\n\rAdded the initial full row of 0s, %d elements", N_PATCHES_W);
                    } 
                    else if (PAD > 0 && im_row >= IW)
                    {
                        /* im_row >= IH case: only the output_image_ptr needs to be updated */
                        output_data_ptr += N_PATCHES_W;
                        PRINTF_DEB("\n\rAdded the final full row of 0s, %d elements", N_PATCHES_W);
                    }
                    else
                    {

                        /* Computing the number of zeros before the first element of the patch */

                        /* In the offset of the element in the filter is bigger than P, then no zeros are needed */
                        if ( w_offset >= PAD)
                        {
                            n_zeros_left = 0;
                        }
                        else if ( (PAD - w_offset) % STRIDES == 0 )
                        {
                            n_zeros_left = (PAD - w_offset) / STRIDES;
                        }
                        else
                        {
                            n_zeros_left = (PAD - w_offset) / STRIDES + 1;
                        }

                        /* Computing the number of zeros after the last element of the patch */

                        /* The stray elements are the elements that are not covered by the patches */
                        stray_elements = (2*PAD + IW) - STRIDES * (N_PATCHES_W - 1) - FW;

                        /* This computes the last position of the current element of the filter */
                        last_position = 2*PAD + IW - stray_elements - FW + w_offset;

                        /* To adapt the final case to the formulas used to the first padded region, let's compute an "adapted" padded region,
                        /* by removing the elements of the row uncovered by the sliding filter */
                        tmp_pad = PAD - stray_elements;

                        if (FW - 1 - w_offset >= PAD)
                        {
                            n_zeros_right = 0;
                        }
                        else if ( (tmp_pad - (FW - 1 - w_offset)) % STRIDES == 0 )
                        {
                            n_zeros_right = (tmp_pad - (FW - 1 - w_offset)) / STRIDES;
                        }
                        else
                        {
                            n_zeros_right = (tmp_pad - (FW - 1 - w_offset)) / STRIDES + 1;
                        }

                        /* Compute the number of elements to transfer */
                        size_transfer = N_PATCHES_W - n_zeros_left - n_zeros_right;

                        if (n_zeros_left > 0)
                        {
                            /* im_col < 0 case: only the output_image_ptr needs to be updated */
                            output_data_ptr += n_zeros_left;
                            
                            PRINTF_DEB("\n\rAdded %d '0's before",  n_zeros_left);

                            im_col += n_zeros_left * STRIDES;
                        }

                        /* DMA setup and transaction run */

                        input_image_ptr = &input_image_nchw[0] + get_index(CH, IH, IW, b, im_c, im_row, im_col);
                        tgt_src.ptr = input_image_ptr;
                        tgt_src.size_du = size_transfer;
                        tgt_src.inc_du = STRIDES;

                        tgt_dst.ptr = output_data_ptr;
                        tgt_dst.size_du = size_transfer;
                        
                        dma_run(&trans);

                        ptr = output_data_ptr;

                        #if DEBUG
                        PRINTF_DEB("\n\rWrote %d elements from input", tgt_src.size_du);
                        for (int i=0; i<tgt_src.size_du; i++)
                        {
                            PRINTF_DEB("\n\r%p %d", ptr, *ptr);
                            ptr += 1;
                        }
                        #endif

                        output_data_ptr += size_transfer;

                        if (n_zeros_right > 0)
                        {
                            /* im_col < 0 case: only the output_image_ptr needs to be updated */
                            output_data_ptr += n_zeros_right;
                            PRINTF_DEB("\n\rAdded %d '0's after",  n_zeros_right);
                        } 
                        PRINTF_DEB("\n\r");
                    }

                    #if DEBUG
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
                }        
        }

        #if TIMING  
        CSR_READ(CSR_REG_MCYCLE, &cycles_B);
        *cycles = (cycles_B - cycles_A);
        #endif
    }

    else if (test_id == 2)
    {
        /* Implementation of the nchw im2col algorithm using the DMA 2D feature */

        /* Iterate over each row of the output matrix. */

        uint32_t* input_image_ptr = &input_image_nchw[0];
        uint32_t* output_data_ptr = &output_data[0];


        dma_config_flags_t res;

        dma_target_t tgt_src = {
                                    .ptr        = input_image_nchw,
                                    .inc_du     = STRIDES,
                                    .type       = DMA_DATA_TYPE_WORD
                            };
        dma_target_t tgt_dst = {
                                    .ptr        = output_data_ptr,
                                    .inc_du     = 1,
                                    .inc_d2_du  = 1
                                    };

        dma_trans_t trans = {
                                    .src        = &tgt_src,
                                    .dst        = &tgt_dst,
                                    .mode       = DMA_TRANS_MODE_SINGLE,
                                    .dim        = DMA_DIM_CONF_2D,
                                    .end        = DMA_TRANS_END_INTR
                                    };

        uint32_t* ptr;

        #if TIMING  
        CSR_READ(CSR_REG_MCYCLE, &cycles_A);
        #endif

        /* The DMA is initialized (i.e. Any current transaction is cleaned.) */
        dma_init(NULL); 

        // for (int c = 0; c < CH_COL; ++c) {
        //     /* Calculate offsets within the kernel window. */

        //     w_offset = c % FW; 
        //     h_offset = (c / FW) % FH; 
        //     im_c = c / (FH * FW); 

        //     /* Iterate over each BATCH. */
        //     for (int32_t b = 0; b < BATCH; ++b) {
        //         /* Iterate over each patch on the IW of the input matrix. */
        //         im_row = h_offset - PAD;

        //         im_col = w_offset - PAD;

        //         n_zeros_left = 0;
        //         n_zeros_right = 0;
        //         n_zeros_top = 0;
        //         n_zeros_bottom = 0;
        //         size_transfer = 0;
        //         size_transfer_d2 = 0;
        //         minimum = 0;
        //         start_max = 0;
        //         start_min = 0;
        //         stray_elements = 0;
        //         last_position = 0;
        //         tmp_pad = 0;

        //         PRINTF_DEB("\n\rim_row: %d, im_col: %d", im_row, im_col);
        //         PRINTF_DEB("\n\rw_offset: %d, h_offset: %d\n\r", w_offset, h_offset);

        //         /* Computing the number of zeros on the left */

        //         /* In the offset of the element in the filter is bigger than P, then no zeros are needed */
        //         if ( w_offset >= PAD)
        //         {
        //             n_zeros_left = 0;
        //         }
        //         else if ( (PAD - w_offset) % STRIDES == 0 )
        //         {
        //             n_zeros_left = (PAD - w_offset) / STRIDES;
        //         }
        //         else
        //         {
        //             n_zeros_left = (PAD - w_offset) / STRIDES + 1;
        //         }

        //         /* Computing the number of zeros on the top */

        //         /* In the offset of the element in the filter is bigger than P, then no zeros are needed */
        //         if ( h_offset >= PAD)
        //         {
        //             n_zeros_top = 0;
        //         }
        //         else if ( (PAD - h_offset) % STRIDES == 0 )
        //         {
        //             n_zeros_top = (PAD - h_offset) / STRIDES;
        //         }
        //         else
        //         {
        //             n_zeros_top = (PAD - h_offset) / STRIDES + 1;
        //         }

        //         /* Computing the number of zeros on the right */

        //         /* The stray elements are the elements that are not covered by the patches */
        //         stray_elements = (2*PAD + IW) - STRIDES * (N_PATCHES_W - 1) - FW;

        //         /* This computes the last position of the current element of the filter */
        //         last_position = 2*PAD + IW - stray_elements - FW + w_offset;

        //         /* To adapt the final case to the formulas used to the first padded region, let's compute an "adapted" padded region,
        //         /* by removing the elements of the row uncovered by the sliding filter */
        //         tmp_pad = PAD - stray_elements;

        //         if (FW - 1 - w_offset >= PAD)
        //         {
        //             n_zeros_right = 0;
        //         }
        //         else if ( (tmp_pad - (FW - 1 - w_offset)) % STRIDES == 0 )
        //         {
        //             n_zeros_right = (tmp_pad - (FW - 1 - w_offset)) / STRIDES;
        //         }
        //         else
        //         {
        //             n_zeros_right = (tmp_pad - (FW - 1 - w_offset)) / STRIDES + 1;
        //         }

        //         /* Computing the number of zeros on the bottom */

        //         /* The stray elements are the elements that are not covered by the patches */
        //         stray_elements = (2*PAD + IH) - STRIDES * (N_PATCHES_H - 1) - FH;

        //         /* This computes the last position of the current element of the filter */
        //         last_position = 2*PAD + IH - stray_elements - FH + h_offset;

        //         /* To adapt the final case to the formulas used to the first padded region, let's compute an "adapted" padded region,
        //         /* by removing the elements of the row uncovered by the sliding filter */
        //         tmp_pad = PAD - stray_elements;

        //         if (FH - 1 - h_offset >= PAD)
        //         {
        //             n_zeros_bottom = 0;
        //         }
        //         else if ( (tmp_pad - (FH - 1 - h_offset)) % STRIDES == 0 )
        //         {
        //             n_zeros_bottom = (tmp_pad - (FH - 1 - h_offset)) / STRIDES;
        //         }
        //         else
        //         {
        //             n_zeros_bottom = (tmp_pad - (FH - 1 - h_offset)) / STRIDES + 1;
        //         }

        //         /* Compute the number of elements to transfer */
        //         size_transfer = N_PATCHES_W - n_zeros_left - n_zeros_right;
        //         size_transfer_d2 = N_PATCHES_H - n_zeros_top - n_zeros_bottom;

        //         PRINTF_DEB("\n\rn_zeros_left: %d, n_zeros_right: %d, n_zeros_top: %d, n_zeros_bottom: %d", n_zeros_left, n_zeros_right, n_zeros_top, n_zeros_bottom);
        //         PRINTF_DEB("\n\rsize_transfer: %d, size_transfer_d2: %d", size_transfer, size_transfer_d2);
    
        //         /* DMA setup and transaction run */
        //         int index = get_index(CH, IH, IW, b, im_c, im_row + n_zeros_top*STRIDES, im_col + n_zeros_left*STRIDES);
        //         input_image_ptr = &input_image_nchw[0] + index;
        //         tgt_src.ptr = input_image_ptr;
        //         tgt_src.size_du = size_transfer;
        //         tgt_src.size_d2_du = size_transfer_d2;
        //         tgt_src.inc_d2_du = (STRIDES * IW - (size_transfer - 1 + (STRIDES - 1) * (size_transfer - 1)));

        //         tgt_dst.ptr = output_data_ptr;
        //         tgt_dst.size_du = size_transfer;
        //         tgt_dst.size_d2_du = size_transfer_d2;

        //         trans.pad_top_du = n_zeros_top;
        //         trans.pad_left_du = n_zeros_left;
        //         trans.pad_right_du = n_zeros_right;
        //         trans.pad_bottom_du = n_zeros_bottom;

        //         PRINTF_DEB("\n\rindex: %d, input_image[0]: %d, src_inc_d2_du: %d", index, *input_image_ptr, tgt_src.inc_d2_du);
                
        //         dma_run(&trans);

        //         output_data_ptr += N_PATCHES_H * N_PATCHES_W;

        //         PRINTF_DEB("\n\r");

        //         #if DEBUG
        //         PRINTF_DEB("\n\rCurrent output matrix: \n\r");
        //         for (int i=0; i<OH_NCHW; i++)
        //         {
        //             for (int j=0; j<OW_NCHW; j++)
        //             {
        //                 PRINTF_DEB("%d ", output_data[i*OW_NCHW + j]);
        //             }
        //             PRINTF_DEB("\n\r");
        //         }
        //         #endif
        //     }
        //         PRINTF_DEB("\n\r");
                    
        // }

        for (int c = 0; c < CH_COL; ++c) {
            /* Calculate offsets within the kernel window. */
            w_offset = c % FW; 
            h_offset = (c / FW) % FH; 
            im_c = c / (FH * FW); 

            /* Precompute constants */
            im_col = w_offset - PAD;
            im_row = h_offset - PAD;
            fw_minus_w_offset = FW - 1 - w_offset;
            fh_minus_h_offset = FH - 1 - h_offset;

            /* Iterate over each BATCH. */
            for (int32_t b = 0; b < BATCH; ++b) {
                
                PRINTF_DEB("\n\rim_row: %d, im_col: %d", im_row, im_col);
                PRINTF_DEB("\n\rw_offset: %d, h_offset: %d\n\r", w_offset, h_offset);

                /* Computing the number of zeros on the left */
                n_zeros_left = (w_offset >= PAD) ? 0 : ((PAD - w_offset + STRIDES - 1) / STRIDES);

                /* Computing the number of zeros on the top */
                n_zeros_top = (h_offset >= PAD) ? 0 : ((PAD - h_offset + STRIDES - 1) / STRIDES);

                /* Computing the number of zeros on the right */
                n_zeros_right = (fw_minus_w_offset >= PAD) ? 0 : ((TMP_PAD_W - fw_minus_w_offset + STRIDES - 1) / STRIDES);

                /* Computing the number of zeros on the bottom */
                n_zeros_bottom = (fh_minus_h_offset >= PAD) ? 0 : ((TMP_PAD_H - fh_minus_h_offset + STRIDES - 1) / STRIDES);

                /* Compute the number of elements to transfer */
                size_transfer = N_PATCHES_W - n_zeros_left - n_zeros_right;
                size_transfer_d2 = N_PATCHES_H - n_zeros_top - n_zeros_bottom;

                PRINTF_DEB("\n\rn_zeros_left: %d, n_zeros_right: %d, n_zeros_top: %d, n_zeros_bottom: %d", n_zeros_left, n_zeros_right, n_zeros_top, n_zeros_bottom);
                PRINTF_DEB("\n\rsize_transfer: %d, size_transfer_d2: %d", size_transfer, size_transfer_d2);

                /* DMA setup and transaction run */
                int index = get_index(CH, IH, IW, b, im_c, im_row + n_zeros_top * STRIDES, im_col + n_zeros_left * STRIDES);
                input_image_ptr = &input_image_nchw[0] + index;
                tgt_src.ptr = input_image_ptr;
                tgt_src.size_du = size_transfer;
                tgt_src.size_d2_du = size_transfer_d2;

                tgt_dst.ptr = output_data_ptr;
                tgt_dst.size_du = size_transfer;
                tgt_dst.size_d2_du = size_transfer_d2;

                trans.pad_top_du = n_zeros_top;
                trans.pad_left_du = n_zeros_left;
                trans.pad_right_du = n_zeros_right;
                trans.pad_bottom_du = n_zeros_bottom;

                dma_run(&trans);

                output_data_ptr += N_PATCHES_H * N_PATCHES_W;

                PRINTF_DEB("\n\r");

                #if DEBUG
                PRINTF_DEB("\n\rCurrent output matrix: \n\r");
                for (int i = 0; i < OH_NCHW; i++) {
                    for (int j = 0; j < OW_NCHW; j++) {
                        PRINTF_DEB("%d ", output_data[i * OW_NCHW + j]);
                    }
                    PRINTF_DEB("\n\r");
                }
                #endif
            }
            PRINTF_DEB("\n\r");
        }

        #if TIMING  
        CSR_READ(CSR_REG_MCYCLE, &cycles_B);
        *cycles = (cycles_B - cycles_A);
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

// /*int im2col_nhwc_int32(uint8_t test_id, unsigned int *cycles)
// {
//     /* Implementation of the nhwc im2col algorithm using the DMA 2D feature */

//     /* Iterate over each row of the output matrix. */
//     int size_transfer = 0;
//     int im_row = 0;
//     int im_col = 0;
//     int w_offset = 0;  
//     int h_offset = 0; 
//     int im_c = 0; 
//     int col_index = 0;
//     unsigned int cycles_A = 0;
//     unsigned int cycles_B = 0;

//     #if TIMING
//         CSR_SET_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
//         CSR_WRITE(CSR_REG_MCYCLE, 0);
//         CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
//     #endif

//     if (test_id == 0)
//     {
//         /* Calculate the heigth of the output matrix */

//         #if TIMING  
//         CSR_READ(CSR_REG_MCYCLE, &cycles_A);
//         #endif

//         /* Iterate over each row of the output matrix. */
//         for (int b = 0; b < BATCH; ++b) {
//             /* Iterate over each BATCH. */
//             for (int h = 0; h < N_PATCHES_H; ++h) {
//                 /* Iterate over each patch on the IW of the input matrix. */
//                 for (int w = 0; w < N_PATCHES_W; ++w) {
//                     /* Iterate over each patch on the heigth in the output matrix. */
//                     for (int c = 0; c < CH_COL; ++c) {
//                         /* Calculate offsets within the kernel window. */
//                         /* These are used to move the filter around the input image */

//                         w_offset = c % FW;  
//                         h_offset = (c / FW) % FH;
//                         im_c = c / (FH * FW); /* Gets the CH on which the im2col is being performed depending on the row of the output image (c) */
                        
//                         /* Calculate the row and column indices in the original input image, applying the stride and offset. */
//                         im_row = h_offset + h * STRIDES - PAD;
//                         im_col = w_offset + w * STRIDES - PAD;

//                         /* Calculate the index in the flattened output array where this value should be stored. */
//                         col_index = ((b * N_PATCHES_H + h) * N_PATCHES_W + w) * CH_COL + c; 

//                         /* If the calculated indices are outside the bounds of the input image, set the output to 0 (padding effect).
//                         /* Otherwise, fetch the value from the input image and store it in the output array. */
//                         if (im_row < 0 || im_col < 0 || im_row >= IH || im_col >= IW) {
//                             output_data[col_index] = 0;
//                         } else {
//                             output_data[col_index] = input_image_nhwc[get_index(IH, IW, CH, b, im_row, im_col, im_c)];
//                         }                    
//                     }
//                 }
//             }
//         }
//         #if TIMING  
//         CSR_READ(CSR_REG_MCYCLE, &cycles_B);
//         *cycles = (cycles_B - cycles_A);
//         #endif
//     } 

//     else if (test_id == 2)
//     {

//     }

//     #if DEBUG
//     PRINTF("Final output matrix:\n\r\n\r");
//     for (int i=0; i<OH_NHWC; i++)
//     {
//         for (int j=0; j<OW_NHWC; j++)
//         {
//             PRINTF("%d ", output_data[i*OW_NHWC + j]);
//         }
//         PRINTF("\n\r");
//     }
//     #endif

//     /* Return a 0 to indicate a success */
//     return 0;
// }


int get_index(int dim1, int dim2, int dim3, int index0, int index1, int index2,
                          int index3)
{
    return ((index0 * dim1 + index1) * dim2 + index2) * dim3 + index3;
}

/* Verifies the im2col using golden values generated by "verification_script.py" */
int verify(int format)
{
    int errors = 0;

    if (format == 0)
    {
        for (int i=0; i<OH_NCHW; i++)
        {
            for (int j=0; j<OW_NCHW; j++)
            {    
                if (golden_im2col_nchw[i*OW_NCHW + j] != output_data[i*OW_NCHW + j])
                {
                    PRINTF("ERROR: Golden: %d, Output: %d, at %d %d\n\r", golden_im2col_nchw[i*OW_NCHW + j], output_data[i*OW_NCHW + j], i, j);
                    errors ++;
                }
            }
        }
    }
    else
    {
        // for (int i=0; i<OH_NHWC; i++)
        // {
        //     for (int j=0; j<OW_NHWC; j++)
        //     {    
        //         if (golden_im2col_nhwc[i*OW_NHWC + j] != output_data[i*OW_NHWC + j])
        //         {
        //             PRINTF("ERROR: Golden: %d, Output: %d, at %d %d\n\r", golden_im2col_nhwc[i*OW_NHWC + j], output_data[i*OW_NHWC + j], i, j);
        //             errors ++;
        //         }
        //     }
        // }
    }
    return errors;
}

void dma_run(dma_trans_t * trans)
{
    dma_validate_transaction(trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    dma_load_transaction(trans);
    dma_launch(trans);

    while( ! dma_is_ready(0))

    return;
}
