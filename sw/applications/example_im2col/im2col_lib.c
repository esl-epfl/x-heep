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

int output_data[OH_NCHW*OW_NCHW];

int im2col_nchw_int32()
{
    PRINTF("OH: %d, OW: %d\n", OH_NCHW, OW_NCHW);

    int size_transfer = 0;
    int im_row = 0;
    int im_col = 0;
    int w_offset = 0;  // the offset ALONG the IW
    int h_offset = 0; // the offset ALONG the IH
    int im_c = 0; // Gets the CH on which the im2col is being performed depending on the row of the output image (c)
    int col_index = 0;
    
    // Iterate over each row of the output matrix.
    for (int c = 0; c < CH_COL; ++c) {
        // Calculate offsets within the kernel window.
        // These are used to move the filter around the input image

        w_offset = c % FW;  
        h_offset = (c / FW) % FH;
        im_c = c / (FH * FW); // Gets the CH on which the im2col is being performed depending on the row of the output image (c)

        // Iterate over each BATCH.
        for (int b = 0; b < BATCH; ++b) {
            // Iterate over each patch on the IW of the input matrix.
            for (int h = 0; h < N_PATCHES_H; ++h) {
                // Iterate over each patch on the heigth in the output matrix.
                for (int w = 0; w < N_PATCHES_W; ++w) {
                    // Calculate the row and column indices in the original input image, applying the stride and offset.
                    im_row = h_offset + h * STRIDES - PAD;
                    im_col = w_offset + w * STRIDES - PAD;

                    // Calculate the index in the flattened output array where this value should be stored.
                    col_index = ((c * BATCH + b) * N_PATCHES_H + h) * N_PATCHES_W + w;
                    
                    // If the calculated indices are outside the bounds of the input image, set the output to 0 (padding effect).
                    // Otherwise, fetch the value from the input image and store it in the output array.
                    if (im_row < 0 || im_col < 0 || im_row >= IH || im_col >= IW) {
                        output_data[col_index] = 0;
                    } else {
                        output_data[col_index] = input_image_nchw[get_index(CH, IH, IW, b, im_c, im_row, im_col)];                        
                    }
                }
            }
        }
    }

    // Finished!

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
    PRINTF("OH: %d, OW: %d\n", OH_NHWC, OW_NHWC);

    int size_transfer = 0;
    int im_row = 0;
    int im_col = 0;
    int w_offset = 0;  // the offset ALONG the IW
    int h_offset = 0; // the offset ALONG the IH
    int im_c = 0; // Gets the CH on which the im2col is being performed depending on the row of the output image (c)
    int col_index = 0;

    // Calculate the heigth of the output matrix

    // Iterate over each row of the output matrix.
    for (int b = 0; b < BATCH; ++b) {
        // Iterate over each BATCH.
        for (int h = 0; h < N_PATCHES_H; ++h) {
            // Iterate over each patch on the IW of the input matrix.
            for (int w = 0; w < N_PATCHES_W; ++w) {
                // Iterate over each patch on the heigth in the output matrix.
                for (int c = 0; c < CH_COL; ++c) {
                    // Calculate offsets within the kernel window.
                    // These are used to move the filter around the input image

                    w_offset = c % FW;  
                    h_offset = (c / FW) % FH;
                    im_c = c / (FH * FW); // Gets the CH on which the im2col is being performed depending on the row of the output image (c)
                    
                    // Calculate the row and column indices in the original input image, applying the stride and offset.
                    im_row = h_offset + h * STRIDES - PAD;
                    im_col = w_offset + w * STRIDES - PAD;

                    // Calculate the index in the flattened output array where this value should be stored.
                    col_index = ((b * N_PATCHES_H + h) * N_PATCHES_W + w) * CH_COL + c; //  ((c * BATCH + b) * N_PATCHES_H + h) * N_PATCHES_W + w;
                    
                    // If the calculated indices are outside the bounds of the input image, set the output to 0 (padding effect).
                    // Otherwise, fetch the value from the input image and store it in the output array.
                    if (im_row < 0 || im_col < 0 || im_row >= IH || im_col >= IW) {
                        output_data[col_index] = 0;
                    } else {
                        output_data[col_index] = input_image_nhwc[get_index(IH, IW, CH, b, im_row, im_col, im_c)];
                    }                    
                }
            }
        }
    }

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


int get_index(int dim1, int dim2, int dim3, int index0, int index1, int index2,
                          int index3)
{
    return ((index0 * dim1 + index1) * dim2 + index2) * dim3 + index3;
}

// Verifies the im2col using golden values generated by "verification_script.py"
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
                    PRINTF("ERROR: Golden: %d, Output: %d, at %d %d\n", golden_im2col_nchw[i*OW_NCHW + j], output_data[i*OW_NCHW + j], i, j);
                    errors ++;
                }
            }
        }
    }
    else
    {
        for (int i=0; i<OH_NHWC; i++)
        {
            for (int j=0; j<OW_NHWC; j++)
            {    
                if (golden_im2col_nhwc[i*OW_NHWC + j] != output_data[i*OW_NHWC + j])
                {
                    PRINTF("ERROR: Golden: %d, Output: %d, at %d %d\n", golden_im2col_nhwc[i*OW_NHWC + j], output_data[i*OW_NHWC + j], i, j);
                    errors ++;
                }
            }
        }
    }
    return errors;
}

void dma_run(dma_trans_t * trans)
{
    int res = dma_validate_transaction(trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    res = dma_load_transaction(trans);
    res = dma_launch(trans);

    while( ! dma_is_ready(0)) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            //from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}
