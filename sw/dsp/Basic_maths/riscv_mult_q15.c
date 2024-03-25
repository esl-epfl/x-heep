#include <stdio.h>
#include <stdlib.h>
#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/riscv_math.h"
#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/x_heep_emul.h"
// HAD RO MODIFY THE X_HEEP_EMUL FILE AND ADD ADD2v
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
    q15_t result_q15[MAX_BLOCKSIZE];

    q15_t srcA_buf_q15[MAX_BLOCKSIZE] =
    {
    0x7512,  0x1375,   0x1565,
    0x44C3,  0x1188,   0x1CA1,
    0x5264,  0x0B20,   0x8305,
    0x9112,  0x3399,   0x2518,
    0x1AB2,  0x4D01,   0x6F23,
    0x26FF,  0x0121,   0xF123,
    0xAC47,  0x6688,   0x76A2,
    0x5476,  0x8756,   0x36B3,
    0x2245,  0x3373,   0xE1A9,
    0x610A,  0x5419,   0x3501,
    0x9100,  0x4469
    };

    q15_t srcB_buf_q15[MAX_BLOCKSIZE] =
    {
    0x7512,  0x1375,   0x1565,
    0x44C3,  0x1188,   0x1CA1,
    0x5264,  0x0B20,   0x8305,
    0x9112,  0x3399,   0x2518,
    0x1AB2,  0x4D01,   0x6F23,
    0x26FF,  0x0121,   0xF123,
    0xAC47,  0x6688,   0x76A2,
    0x5476,  0x8756,   0x36B3,
    0x2245,  0x3373,   0xE1A9,
    0x610A,  0x5419,   0x3501,
    0x9100,  0x4469
    };

    void riscv_mult_q15(
    q15_t * pSrcA,
    q15_t * pSrcB,
    q15_t * pDst,
    uint32_t blockSize)
    {
        uint32_t blkCnt;                               /* loop counters */

        #if defined (USE_DSP_RISCV)

        q31_t mul1;                  /* temporary variables */

        blkCnt = blockSize;
        while (blkCnt > 0u)
        {
        /*mulsN perform multiplication the normalization*/
            mul1 = x_heep_mulsN(*pSrcA++, *pSrcB++,15);
        /*saturate teh result then save it to the destination buffer*/
            *pDst++ =  (q15_t)x_heep_clip(mul1,15);
        /* decrement loop counter*/
            blkCnt--;
        }



        #else
        /* Initialize blkCnt with number of samples */
        blkCnt = blockSize;

        while(blkCnt > 0u)
        {
            /* C = A * B */
            /* Multiply the inputs and store the result in the destination buffer */
            *pDst++ = (q15_t) __SSAT((((q31_t) (*pSrcA++) * (*pSrcB++)) >> 15), 16);

            /* Decrement the blockSize loop counter */
            blkCnt--;
        }
        #endif
    }
riscv_mult_q15(srcA_buf_q15, srcB_buf_q15, result_q15, MAX_BLOCKSIZE);
PRINT_Q(result_q15,MAX_BLOCKSIZE);
printf("\nCorrect answer:\n");
printf("0x6B12 0x02F5 0x0393 0x24F0 0x0266 0x0667 0x3508 0x00F7 0x7A08 0x6022 0x14CC 0x0ABF 0x0591 0x2E53 0x607E 0x0BE1 0x0002 0x01B9 0x36C2 0x5221 0x6DF3 0x37BB 0x71BF 0x1760 0x092C 0x14AE 0x0731 0x4991 0x3740 0x15F2 0x6042 0x248F");
printf("\n");
printf("hello world!\n");
return EXIT_SUCCESS;
}