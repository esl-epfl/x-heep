#include <stdio.h>
#include <stdlib.h>
#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/riscv_math.h"

int main(int argc, char *argv[])
{
    #define PRINT_F32(X,Y) printf("\n"); for(int i =0 ; i < (Y); i++) printf("%d  ",(int)(X[i]*100)); \
    printf("\n\n")
    #define MAX_BLOCKSIZE     32
   
	 void riscv_abs_f32(
        float32_t * pSrc,
        float32_t * pDst,
        uint32_t blockSize)
    {
        uint32_t blkCnt;                               /* loop counter */

        #if defined (USE_DSP_RISCV)

        float32_t in1, in2, in3, in4;                  /* temporary variables */

        /*loop Unrolling */
        blkCnt = blockSize >> 2u;

        /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
        ** a second loop below computes the remaining 1 to 3 samples. */
        while (blkCnt > 0u)
        {
            /* C = |A| */
            /* Calculate absolute and then store the results in the destination buffer. */
            /* read sample from source */
            in1 = *pSrc;
            in2 = *(pSrc + 1);
            in3 = *(pSrc + 2);

            /* find absolute value */
            in1 = fabsf(in1);

            /* read sample from source */
            in4 = *(pSrc + 3);

            /* find absolute value */
            in2 = fabsf(in2);

            /* read sample from source */
            *pDst = in1;

            /* find absolute value */
            in3 = fabsf(in3);

            /* find absolute value */
            in4 = fabsf(in4);

            /* store result to destination */
            *(pDst + 1) = in2;

            /* store result to destination */
            *(pDst + 2) = in3;

            /* store result to destination */
            *(pDst + 3) = in4;


            /* Update source pointer to process next sampels */
            pSrc += 4u;

            /* Update destination pointer to process next sampels */
            pDst += 4u;

            /* Decrement the loop counter */
            blkCnt--;
        }

        /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
        ** No loop unrolling is used. */
        blkCnt = blockSize % 0x4u;

        #else

        /* Initialize blkCnt with number of samples */
        blkCnt = blockSize;

        #endif 

        while(blkCnt > 0u)
        {
            /* C = |A| */
            /* Calculate absolute and then store the results in the destination buffer. */
            *pDst++ = fabsf(*pSrc++);

            /* Decrement the loop counter */
            blkCnt--;
        }
    }
}
