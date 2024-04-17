/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>

    Info: im2col_lib.c describes functions used to calculate im2col and verify it using 
    the golden result in im2colGolden.c.

    Notes: im2col_nchw_int32() and im2col_nhwc_int32() algorithms are inspired from the library SHL, developed by T-HEAD Semi.
    For reference, check out the following link:
    https://github.com/T-head-Semi/csi-nn2/blob/main/source/reference/im2col.c
*/

#include "im2col_lib.h"

uint32_t output_data[OH*OW];

int im2col_nchw_int32()
{
    PRINTF("%d %d\n", OH, OW);

    // Calculate the IH and IW of the output matrix (column) based on the input dimensions, padding, kernel size, and stride.
    // This essentially calculates how many sliding window positions fit into the input dimensions.
    int patches_h = (IH + PAD + PAD - FH) / STRIDES + 1;
    int patches_w = (IW + PAD + PAD - FW) / STRIDES + 1;

    // Calculate the heigth of the output matrix
    int output_h = CH * FH * FW; 
    int counter = 0;
    int n_zeros_before = 0;
    int n_zeros_after = 0;
    int size_transfer = 0;
    uint32_t im_row = 0;
    uint32_t im_col = 0;
    int w_offset = 0;  // the offset ALONG the IW
    int h_offset = 0; // the ofset ALONG the IH
    int32_t im_c = 0; // Gets the CH on which the im2col is being performed depending on the row of the output image (c)
    int col_index = 0;
    int minimum = 0;
    int start_max = 0;
    int start_min = 0;
    int stray_elements = 0;
    int last_position = 0;
    int tmp_pad = 0;
    unsigned int cycles_A = 0;
    unsigned int cycles_B = 0;
    unsigned int avg_first_zeros;
    unsigned int avg_last_zeros;
    unsigned int avg_patch = 0;

    // Exploit the CPU for im2col
    #if HW_CONFIG == 0
    
    // Iterate over each row of the output matrix.
    for (int c = 0; c < output_h; ++c) {
        // Calculate offsets within the kernel window.
        // These are used to move the filter around the input image

        w_offset = c % FW;  
        h_offset = (c / FW) % FH;
        im_c = c / (FH * FW); // Gets the CH on which the im2col is being performed depending on the row of the output image (c)

        // Iterate over each BATCH.
        for (int32_t b = 0; b < BATCH; ++b) {
            // Iterate over each patch on the IW of the input matrix.
            for (int h = 0; h < patches_h; ++h) {
                // Iterate over each patch on the heigth in the output matrix.
                for (int w = 0; w < patches_w; ++w) {
                    // Calculate the row and column indices in the original input image, applying the stride and offset.
                    im_row = h_offset + h * STRIDES - PAD;
                    im_col = w_offset + w * STRIDES - PAD;

                    // Calculate the index in the flattened output array where this value should be stored.
                    col_index = ((c * BATCH + b) * patches_h + h) * patches_w + w;
                    
                    // If the calculated indices are outside the bounds of the input image, set the output to 0 (padding effect).
                    // Otherwise, fetch the value from the input image and store it in the output array.
                    if (im_row < 0 || im_col < 0 || im_row >= IH || im_col >= IW) {
                        output_data[col_index] = 0;
                    } else {
                        output_data[col_index] = input_image[get_index(CH, IH, IW, b, im_c, im_row, im_col)];
                    }
                }
            }
        }
        
    }
    #elif HW_CONFIG == 1
    // Use DMA 1D for im2col
    // Iterate over each row of the output matrix.

    uint32_t* input_image_ptr = &input_image[0];
    uint32_t* output_data_ptr = &output_data[0];

    // The DMA is initialized (i.e. Any current transaction is cleaned.)
    dma_init(NULL); // NULL because we want to use the integrated DMA

    dma_config_flags_t res;

    dma_target_t tgt_src = {
                                .ptr        = input_image,
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

    for (int c = 0; c < output_h; ++c) {
        // Calculate offsets within the kernel window.
        // These are used to move the filter around the input image

        w_offset = c % FW;  // Offset along the IW
        h_offset = (c / FW) % FH; // Offset along the IH
        im_c = c / (FH * FW); // Gets the CH on which the im2col is being performed depending on the row of the output image (c)

        // Iterate over each BATCH.
        for (int32_t b = 0; b < BATCH; ++b) {
            // Iterate over each patch on the IW of the input matrix.
            for (int h = 0; h < patches_h; ++h) {

                im_row = h_offset + h * STRIDES - PAD;

                im_col = w_offset - PAD;

                n_zeros_before = 0;
                n_zeros_after = 0;
                size_transfer = 0;
                minimum = 0;
                start_max = 0;
                start_min = 0;
                stray_elements = 0;
                last_position = 0;
                tmp_pad = 0;

                PRINTF_DEB("\nim_row: %d, im_col: %d", im_row, im_col);
                PRINTF_DEB("\nw_offset: %d, h_offset: %d\n", w_offset, h_offset);

                // Iterate over each patch on the heigth in the output matrix.
                
                if (PAD > 0 && im_row < 0)
                {
                    // im_row < 0 case: only the output_image_ptr needs to be updated
                    output_data_ptr += patches_w;
                    PRINTF_DEB("\nAdded the initial full row of 0s, %d elements", patches_w);
                } 
                else if (PAD > 0 && im_row >= IW)
                {
                    // im_row >= IH case: only the output_image_ptr needs to be updated
                    output_data_ptr += patches_w;
                    PRINTF("\nAdded the final full row of 0s, %d elements", patches_w);
                }
                else
                {
                    #if TIMING
                    CSR_READ(CSR_REG_MCYCLE, &cycles_A);
                    #endif

                    // Computing the number of zeros before the first element of the patch

                    // In the offset of the element in the filter is bigger than P, then no zeros are needed
                    if ( w_offset >= PAD)
                    {
                        n_zeros_before = 0;
                    }
                    else if ( (PAD - w_offset) % STRIDES == 0 )
                    {
                        n_zeros_before = (PAD - w_offset) / STRIDES;
                    }
                    else
                    {
                        n_zeros_before = (PAD - w_offset) / STRIDES + 1;
                    }

                    #if TIMING
                    CSR_READ(CSR_REG_MCYCLE, &cycles_B);
                    avg_first_zeros += cycles_B - cycles_A;
                    CSR_READ(CSR_REG_MCYCLE, &cycles_A);
                    #endif

                    // Computing the number of zeros after the last element of the patch

                    // The stray elements are the elements that are not covered by the patches
                    stray_elements = (2*PAD + IW) - STRIDES * (patches_w - 1) - FW;

                    // This computes the last position of the current element of the filter
                    last_position = 2*PAD + IW - stray_elements - FW + w_offset;

                    // To adapt the final case to the formulas used to the first padded region, let's compute an "adapted" padded region,
                    // by removing the elements of the row uncovered by the sliding filter
                    tmp_pad = PAD - stray_elements;

                    if (FW - 1 - w_offset >= PAD)
                    {
                        n_zeros_after = 0;
                    }
                    else if ( (tmp_pad - (FW - 1 - w_offset)) % STRIDES == 0 )
                    {
                        n_zeros_after = (tmp_pad - (FW - 1 - w_offset)) / STRIDES;
                    }
                    else
                    {
                        n_zeros_after = (tmp_pad - (FW - 1 - w_offset)) / STRIDES + 1;
                    }

                    #if TIMING
                    CSR_READ(CSR_REG_MCYCLE, &cycles_B);
                    avg_last_zeros += cycles_B - cycles_A;
                    #endif

                    // Compute the number of elements to transfer
                    size_transfer = patches_w - n_zeros_before - n_zeros_after;

                    if (n_zeros_before > 0)
                    {
                        // im_col < 0 case: only the output_image_ptr needs to be updated
                        output_data_ptr += n_zeros_before;
                        
                        PRINTF_DEB("\nAdded %d '0's before",  n_zeros_before);

                        im_col += n_zeros_before * STRIDES;
                    }

                    #if TIMING
                    CSR_READ(CSR_REG_MCYCLE, &cycles_A);
                    #endif

                    // DMA setup and transaction run

                    input_image_ptr = &input_image[0] + get_index(CH, IH, IW, b, im_c, im_row, im_col);
                    tgt_src.ptr = input_image_ptr;
                    tgt_src.size_du = size_transfer;
                    tgt_src.inc_du = STRIDES;

                    tgt_dst.ptr = output_data_ptr;
                    tgt_dst.size_du = size_transfer;
                    
                    dma_run(&trans);

                    ptr = output_data_ptr;

                    #if TIMING
                    CSR_READ(CSR_REG_MCYCLE, &cycles_B);
                    avg_patch += cycles_B - cycles_A;
                    #endif

                    #if DEBUG
                    PRINTF_DEB("\nWrote %d elements from input", tgt_src.size_du);
                    for (int i=0; i<tgt_src.size_du; i++)
                    {
                        PRINTF_DEB("\n%p %d", ptr, *ptr);
                        ptr += 1;
                    }
                    #endif

                    output_data_ptr += size_transfer;

                    if (n_zeros_after > 0)
                    {
                        // im_col < 0 case: only the output_image_ptr needs to be updated
                        output_data_ptr += n_zeros_after;
                        PRINTF_DEB("\nAdded %d '0's after",  n_zeros_after);
                    } 
                    PRINTF_DEB("\n");
                }

                #if DEBUG
                PRINTF_DEB("\nCurrent output matrix: \n");
                for (int i=0; i<OH; i++)
                {
                    for (int j=0; j<OW; j++)
                    {
                        PRINTF_DEB("%d ", output_data[i*OW + j]);
                    }
                    PRINTF_DEB("\n");
                }
                #endif
                }
                PRINTF_DEB("\n");
            }        
    }
    #elif HW_CONFIG == 2
    // Use DMA 2D for im2col
    #endif

    // Finished!

    #if TIMING
    PRINTF("\nAverage cycles for the first zeros: %d/%d", avg_first_zeros , (OW*OH));
    PRINTF("\nAverage cycles for the last zeros: %d/%d", avg_last_zeros , (OW*OH));
    PRINTF("\nAverage cycles for the patch: %d/%d", avg_patch , (OW*OH));
    #endif

    PRINTF("Final output matrix:\n\n");

    #if DEBUG
    for (int i=0; i<OH; i++)
    {
        for (int j=0; j<OW; j++)
        {
            PRINTF("%d ", output_data[i*OW + j]);
        }
        PRINTF("\n");
    }
    #endif

    // Return a 0 to indicate a success
    return 0;
}

int im2col_nhwc_int32()
{
    PRINTF("OH: %d, OW: %d\n", OH, OW);
    
    // Calculate the IH and IW of the output matrix (column) based on the input dimensions, padding, kernel size, and stride.
    // This essentially calculates how many sliding window positions fit into the input dimensions.
    int patches_h = (IH + PAD + PAD - FH) / STRIDES + 1;
    int patches_w = (IW + PAD + PAD - FW) / STRIDES + 1;

    // Calculate the heigth of the output matrix
    int output_h = CH * FH * FW; 
    int counter = 0;

    // Exploit the CPU for im2col
    #if HW_CONFIG == 0
    // Iterate over each row of the output matrix.
    for (int32_t b = 0; b < BATCH; ++b) {
        // Iterate over each BATCH.
        for (int h = 0; h < patches_h; ++h) {
            // Iterate over each patch on the IW of the input matrix.
            for (int w = 0; w < patches_w; ++w) {
                // Iterate over each patch on the heigth in the output matrix.
                for (int c = 0; c < output_h; ++c) {
                    // Calculate offsets within the kernel window.
                    // These are used to move the filter around the input image

                    int w_offset = c % FW;  
                    int h_offset = (c / FW) % FH;
                    int32_t im_c = c / (FH * FW); // Gets the CH on which the im2col is being performed depending on the row of the output image (c)

                    
                    // Calculate the row and column indices in the original input image, applying the stride and offset.
                    int32_t im_row = h_offset + h * STRIDES - PAD;
                    int32_t im_col = w_offset + w * STRIDES - PAD;

                    // Calculate the index in the flattened output array where this value should be stored.
                    int32_t col_index = ((b * patches_h + h) * patches_w + w) * output_h + c; //  ((c * BATCH + b) * patches_h + h) * patches_w + w;
                    
                    // If the calculated indices are outside the bounds of the input image, set the output to 0 (padding effect).
                    // Otherwise, fetch the value from the input image and store it in the output array.
                    if (im_row < 0 || im_col < 0 || im_row >= IH || im_col >= IW) {
                        output_data[col_index] = 0;
                    } else {
                        output_data[col_index] = input_image[get_index(IH, IW, CH, b, im_row, im_col, im_c)];
                    }
                }
            }
        }
    }
    #elif HW_CONFIG == 1

    // Use DMA 1D for im2col
    #elif HW_CONFIG == 2
    // Use DMA 2D for im2col
    #endif

    PRINTF("Final output matrix:\n\n");

    #if DEBUG
    for (int i=0; i<OH; i++)
    {
        for (int j=0; j<OW; j++)
        {
            PRINTF("%d ", output_data[i*OW + j]);
        }
        PRINTF("\n");
    }
    #endif

    // Return a 0 to indicate a success
    return 0;
}


int32_t get_index(int32_t dim1, int32_t dim2, int32_t dim3, int32_t index0, int32_t index1, int32_t index2,
                          int32_t index3)
{
    return ((index0 * dim1 + index1) * dim2 + index2) * dim3 + index3;
}

// Verifies the im2col using golden values generated by "verification_script.py"
uint16_t verify()
{
    uint16_t errors = 0;

    for (uint16_t i=0; i<OH; i++)
    {
        for (uint16_t j=0; j<OW; j++)
        {    
            if (golden_im2col[i*OW + j] != output_data[i*OW + j])
            {
                PRINTF("ERROR: Golden: %d, Output: %d, at %d %d\n", golden_im2col[i*OW + j], output_data[i*OW + j], i, j);
                errors ++;
            }
        }
    }
    return errors;
}

void dma_run(dma_trans_t * trans)
{
    int res = dma_validate_transaction(trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    //PRINTF("tran: %u \t%s\n", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_load_transaction(trans);
    //PRINTF("load: %u \t%s\n", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_launch(trans);
    //PRINTF("laun: %u \t%s\n", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");

    while( ! dma_is_ready()) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready() == 0 ) {
            wait_for_interrupt();
            //from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}
