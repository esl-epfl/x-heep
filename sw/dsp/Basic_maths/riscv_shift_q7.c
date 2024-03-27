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

  void riscv_shift_q7(
  q7_t * pSrc,
  int8_t shiftBits,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign;                                  /* Sign of shiftBits */


  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

#if defined (USE_DSP_RISCV)

  charV *VectInA;
  charV VectInC,VectInB; 
  blkCnt = blockSize >> 2u;
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      int precision=7;
      q15_t x=(q15_t) * pSrc++ << shiftBits;
      *pDst++ = (q7_t) x_heep_clip(x,precision);
      x=(q15_t) * pSrc++ << shiftBits;
      *pDst++ = (q7_t) x_heep_clip(x,precision);
      x=(q15_t) * pSrc++ << shiftBits;
      *pDst++ = (q7_t) x_heep_clip(x,precision);
      x=(q15_t) * pSrc++ << shiftBits;
      *pDst++ = (q7_t) x_heep_clip(x,precision);
      x=(q15_t) * pSrc++ << shiftBits;
      *pDst++ = (q7_t) x_heep_clip(x,precision);
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {

    /* Initialize blkCnt with number of samples */
    VectInB = x_heep_pack4(-shiftBits,-shiftBits,-shiftBits,-shiftBits);
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      VectInA = (charV*)pSrc; 
      VectInC = x_heep_sra4(*VectInA,VectInB); 
      *(charV*)pDst =  VectInC;
      pDst+=4;
      pSrc+=4;
     
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  blkCnt = blockSize % 0x4u;
#else
    blkCnt = blockSize;
#endif
  /* If the shift value is positive then do right shift else left shift */
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (q7_t) __SSAT(((q15_t) * pSrc++ << shiftBits), 8);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (*pSrc++ >> -shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }

}

    riscv_shift_q7(srcA_buf_q7, 3, result_q7, MAX_BLOCKSIZE);    
    PRINT_Q(result_q7,MAX_BLOCKSIZE);
    printf("\nCorrect answer:\n");
    printf("0x7F 0x7F 0x7F 0x7F 0x7F 0x7F 0x7F 0x58 0x80 0x80 0x7F 0x7F 0x7F 0x7F 0x7F 0x7F 0x08 0x88 0x80 0x7F 0x7F 0x7F 0x80 0x7F 0x7F 0x7F 0x80 0x7F 0x7F 0x7F 0x80 0x7F");    printf("\n");
    
    printf("hello world!\n");
    return EXIT_SUCCESS;
}