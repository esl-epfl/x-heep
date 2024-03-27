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
    #define MAX_BLOCKSIZE     32

    void riscv_abs_q7(
  q7_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q7_t in;                                       /* Input value1 */

#if defined (USE_DSP_RISCV)


  q31_t in1, in2, in3, in4;                      /* temporary input variables */
  q31_t out1, out2, out3, out4;                  /* temporary output variables */
  charV *VectInA;
  charV VectInC; 
  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  while (blkCnt > 0u)
  {
    VectInA = (charV*)pSrc; 
    VectInC = x_heep_abs4(*VectInA); /*calculate the absolute of 4 q7 at the same time */
    /*check for each one to saturate it*/
    *pDst++ = ( VectInC[0] == -128)?0x7f:VectInC[0];
    *pDst++ = ( VectInC[1] == -128)?0x7f:VectInC[1];
    *pDst++ = ( VectInC[2] == -128)?0x7f:VectInC[2];
    *pDst++ = ( VectInC[3] == -128)?0x7f:VectInC[3];
    
    pSrc+=4; /*inc pointer for next loop*/
    blkCnt--; /*dec loop counter*/
  }

  blkCnt = blockSize % 0x4u;
#else
  blkCnt = blockSize;
#endif
  while(blkCnt > 0u)
  {
    /* C = |A| */
    /* Read the input */
    in = *pSrc++;

    /* Store the Absolute result in the destination buffer */
    *pDst++ = (in > 0) ? in : ((in == (q7_t) 0x80) ? 0x7f : -in);

    /* Decrement the loop counter */
    blkCnt--;
  }
}
    return EXIT_SUCCESS;
}
