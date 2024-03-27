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



void riscv_scale_q15(
  q15_t * pSrc,
  q15_t scaleFract,
  int8_t shift,
  q15_t * pDst,
  uint32_t blockSize)
{
  int kShift = 15 - shift;                    /* shift to apply after scaling */
  uint32_t blkCnt;                               /* loop counter */

#if defined (USE_DSP_RISCV)

  q31_t mul1;                  

  blkCnt = blockSize;
  while (blkCnt > 0u)
  {
    /*multiply by scale then shift and saturate*/
    q31_t x= (*pSrc++ * scaleFract) >> kShift;
    int precision=15;
    *pDst++ = (q15_t) ((x)<(-(1<<(precision)))?(-(1<<(precision))):(((x)>((1<<(precision))-1))?((1<<(precision))-1):(x)));
    //*pDst++ =  (q15_t)x_heep_clip((((q31_t) (*pSrc++) *scaleFract ) >> kShift),15);
    blkCnt--;
  }

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A * scale */
    /* Scale the input and then store the result in the destination buffer. */
    *pDst++ = (q15_t) (__SSAT(((q31_t) * pSrc++ * scaleFract) >> kShift, 16));

    /* Decrement the loop counter */
    blkCnt--;
  }
#endif
}
riscv_scale_q15(srcA_buf_q15, 0x225A,1, result_q15, MAX_BLOCKSIZE);
PRINT_Q(result_q15,MAX_BLOCKSIZE);
printf("\nCorrect answer:\n");
printf("0x3ED6 0x0A71 0x0B7B 0x24E8 0x0968 0x0F5D 0x2C38 0x05F8 0xBCEA 0xC475 0x1BB1 0x13E8 0x0E54 0x2954 0x3BA6 0x14EE 0x09B 0xF805 0xD30F 0x3708 0x3FAC 0x2D55 0xBF3C 0x1D5C 0x1264 0x1B9D 0xEFB7 0x3415 0x2D23 0x1C73 0xC46B 0x24B7");
printf("\n");
printf("hello world!\n");
return EXIT_SUCCESS;
}