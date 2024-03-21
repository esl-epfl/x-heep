#include <stdio.h>
#include <stdlib.h>
#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/riscv_math.h"

int main(int argc, char *argv[])
{
   /*
    void perf_enable_id( int eventid){
        cpu_perf_conf_events(SPR_PCER_EVENT_MASK(eventid));
        cpu_perf_conf(SPR_PCMR_ACTIVE | SPR_PCMR_SATURATE);
    };
    */
    #define PRINT_F32(X,Y) printf("\n"); for(int i =0 ; i < (Y); i++) printf("%d  ",(int)(X[i]*100)); \
    printf("\n\n")
    #define PRINT_Q(X,Y) printf("\n"); for(int i =0 ; i < (Y); i++) printf("0x%X  ",X[i]); \
    printf("\n\n")
    //#define PRINT_OUTPUT  /*for testing functionality for each function, removed while benchmarking*/
    #define MAX_BLOCKSIZE     32
    #define EVENT_ID 0x00  /*number of cycles ID for benchmarking*/

     float32_t srcA_buf_f32[MAX_BLOCKSIZE] =
    {
    -0.4325648115282207,  -1.6655843782380970,  0.1253323064748307,
    0.2876764203585489,  -1.1464713506814637,  1.1909154656429988,
    1.1891642016521031,  -0.0376332765933176,  0.3272923614086541,
    0.1746391428209245,  -0.1867085776814394,  0.7257905482933027,
    -0.5883165430141887,   2.1831858181971011, -0.1363958830865957,
    0.1139313135208096,   1.0667682113591888,  0.0592814605236053,
    -0.0956484054836690,  -0.8323494636500225,  0.2944108163926404,
    -1.3361818579378040,   0.7143245518189522,  1.6235620644462707,
    -0.6917757017022868,   0.8579966728282626,  1.2540014216025324,
    -1.5937295764474768,  -1.4409644319010200,  0.5711476236581780,
    -0.3998855777153632,   0.6899973754643451
    };
    
    float32_t result_f32[MAX_BLOCKSIZE];

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
