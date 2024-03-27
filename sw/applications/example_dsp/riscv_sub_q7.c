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

 q7_t srcB_buf_q7[MAX_BLOCKSIZE] =
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

  void riscv_sub_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */


#if defined (USE_DSP_RISCV)

  shortV VectInA;
  shortV VectInB;  
  shortV VectInC; 
  /*loop Unrolling */
  blkCnt = blockSize >> 1u;

  while (blkCnt > 0u)
  {
    /*read 2 elements from each source*/
    VectInA[0] = (short)pSrcA[0];
    VectInA[1] = (short)pSrcA[1];
    VectInB[0] = (short)pSrcB[0];
    VectInB[1] = (short)pSrcB[1];
    /*subract them*/
    VectInC = x_heep_sub2(VectInA,VectInB); 
    /*saturate the results then save to destination buufer*/
    *pDst++ =(q7_t)x_heep_clip(VectInC[0],7);
    *pDst++ =(q7_t)x_heep_clip(VectInC[1],7);
    /*increment source buffer*/
    pSrcA+=2;
    pSrcB+=2;
    /*decrement loop counter*/
    blkCnt--;
  }

  blkCnt = blockSize % 0x2u;

  while (blkCnt > 0u)
  {
    /* C = A + B */
    /* subtract then saturate*/
    *pDst++ =(q7_t)x_heep_clip((*pSrcA++ - *pSrcB++),7);
    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A - B */
    /* Subtract and then store the result in the destination buffer. */
    *pDst++ = (q7_t) __SSAT((q15_t) * pSrcA++ - *pSrcB++, 8);

    /* Decrement the loop counter */
    blkCnt--;
  }
#endif
}



    riscv_sub_q7(srcA_buf_q7, srcB_buf_q7, result_q7, MAX_BLOCKSIZE);
    PRINT_Q(result_q7,MAX_BLOCKSIZE);
    printf("\nCorrect answer:\n");
    printf("0x7F 0x26 0x2A 0x7F 0x22 0x38 0x7F 0x16 0x80 0x80 0x66 0x4A 0x34 0x7F 0x7F 0x4C 0x2 0xE2 0x80 0x7F 0x7F 0x7F 0x80 0x6C 0x44 0x66 0xC2 0x7F 0x7F 0x6A 0x80 0x7F");
    printf("\n");
    printf("hello world!\n");
    return EXIT_SUCCESS;
}





