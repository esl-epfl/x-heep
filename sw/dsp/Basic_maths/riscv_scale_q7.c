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
    #define PRINT_Q(X,Y) printf("\n"); for(int i =0 ; i < (Y); i++) printf("0x%x  ",X[i]); \
    printf("\n\n")
    //#define PRINT_OUTPUT  /*for testing functionality for each function, removed while benchmarking*/
    #define MAX_BLOCKSIZE     32
    #define EVENT_ID 0x00  /*number of cycles ID for benchmarking*/
    q7_t result_q7[MAX_BLOCKSIZE];
    q7_t srcA_buf_q7[MAX_BLOCKSIZE] =
{
   0x75,  0x13,   0x15,
   0x44,  0x11,   0x1C,
   0x52,  0x0B,   0x83,
   0x91,  0x33,   0x25,
   0x1A,  0x4D,   0x6F,
   0x26,  0x01,   0xF1,
   0xAC,  0x66,   0x76,
   0x54,  0x87,   0x36,
   0x22,  0x33,   0xE1,
   0x61,  0x54,   0x35,
   0x91,  0x49
};


void riscv_scale_q7(
  q7_t * pSrc,
  q7_t scaleFract,
  int8_t shift,
  q7_t * pDst,
  uint32_t blockSize)
{
  int8_t kShift = 7 - shift;                     /* shift to apply after scaling */
  uint32_t blkCnt;                               /* loop counter */

#if defined (USE_DSP_RISCV)


  blkCnt = blockSize;
  while (blkCnt > 0u)
  {
    /*multiply by scale then shift and saturate*/
    q15_t x= (*pSrc++ * scaleFract) >> kShift;
    int precision=7;
    *pDst++ = ((x)<(-(1<<(precision)))?(-(1<<(precision))):(((x)>((1<<(precision))-1))?((1<<(precision))-1):(x)));
    //*pDst++ =  (q7_t)x_heep_clip((((q15_t) (*pSrc++) * scaleFract) >> kShift),7);
    blkCnt--;
  }

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while (blkCnt > 0u)
  {
    /* C = A * scale */
    /* Scale the input and then store the result in the destination buffer. */
    *pDst++ = (q7_t) (__SSAT((((q15_t) * pSrc++ * scaleFract) >> kShift), 8));
    /* Decrement the loop counter */
    blkCnt--;
  }

#endif 

}

riscv_scale_q7(srcA_buf_q7, 0x15,1, result_q7, MAX_BLOCKSIZE);
PRINT_Q(result_q7,MAX_BLOCKSIZE);
printf("\nCorrect answer:\n");
printf("0x26 0x06 0x06 0x16 0x05 0x09 0x1A 0x03 0xD6 0xDB 0x10 0x0C 0x08 0x19 0x24 0x0C 0x00 0xFB 0xE4 0x21 0x26 0x1B 0xD8 0x11 0x0B 0x10 0xF5 0x1F 0x1B 0x11 0xDB 0x17");
printf("\n");
printf("hello world!\n");
return EXIT_SUCCESS;
}