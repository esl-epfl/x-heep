#include <stdio.h>
#include <stdlib.h>
#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/riscv_math.h"
#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/x_heep_emul.h"
// HAD RO MODIFY THE X_HEEP_EMUL FILE AND ADD NEG2
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

void riscv_shift_q15(
  q15_t * pSrc,
  int8_t shiftBits,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign;                                  /* Sign of shiftBits */


  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

#if defined (USE_DSP_RISCV)
  shortV *VectInA;
  shortV VectInC,VectInB; 
  blkCnt = blockSize >> 1u;
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      int precision=0xF;
      
      q31_t x=((q31_t) * pSrc++ << shiftBits);
      *pDst++ = (q31_t)x_heep_clip(x,precision);
      x=((q31_t) * pSrc++ << shiftBits);
      *pDst++ = (q31_t)x_heep_clip(x,precision);
      
      //*pDst++ = (q31_t)x_heep_clip((q31_t)((q31_t)* pSrc++ )<< shiftBits,(int)0xF);
      //*pDst++ = (q31_t)x_heep_clip((q31_t)((q31_t)* pSrc++ )<< shiftBits,(int)0xF);
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else /*shift right*/
  {

    /* Initialize blkCnt with number of samples */
    VectInB = x_heep_pack2(-shiftBits,-shiftBits);
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      VectInA = (shortV*)pSrc; 
      VectInC = x_heep_sra2(*VectInA,VectInB); 
      //*pDst++ = VectInC[0];
      //*pDst++ = VectInC[1];
      *(shortV*)pDst =  VectInC;
      pDst+=2;
      pSrc+=2;
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  blkCnt = blockSize % 0x2u;

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
      /* Shift and then store the results in the destination buffer. */
      *pDst++ = __SSAT(((q31_t) * pSrc++ << shiftBits), 16);

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
      /* Shift the inputs and then store the results in the destination buffer. */
      *pDst++ = (*pSrc++ >> -shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }

}
riscv_shift_q15(srcA_buf_q15, 2, result_q15, MAX_BLOCKSIZE);
PRINT_Q(result_q15,MAX_BLOCKSIZE);
printf("\nCorrect answer:\n");
printf("0x7FFF 0x4DD4 0x5594 0x7FFF 0x4620 0x7284 0x7FFF 0x2C80 0x8000 0x8000 0x7FFF 0x7FFF 0x6AC8 0x7FFF 0x7FFF 0x7FFF 0x0484 0xC48C 0x8000 0x7FFF 0x7FFF 0x7FFF 0x8000 0x7FFF 0x7FFF 0x7FFF 0x86A4 0x7FFF 0x7FFF 0x7FFF 0x8000 0x7FFF");
printf("\n");
printf("hello world!\n");
return EXIT_SUCCESS;
}



