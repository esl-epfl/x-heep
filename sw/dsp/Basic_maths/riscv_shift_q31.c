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
        q31_t result_q31[MAX_BLOCKSIZE];



    q31_t srcA_buf_q31[MAX_BLOCKSIZE] =
    {
    0x75122542,  0x1375A138,   0x15654473,
    0x44C34463,  0x118854C3,   0x1CA18291,
    0x5264E236,  0x0B200590,   0x83059A77,
    0x91129713,  0x3399D123,   0x2518BBD1,
    0x1AB2037A,  0x4D01D1F1,   0x6F237531,
    0x26FF5569,  0x01213159,   0xF123CD77,
    0xAC475456,  0x6688A12E,   0x76A2FF96,
    0x54761269,  0x8756D438,   0x36B3B697,
    0x22459643,  0x3373C368,   0xE1A901D3,
    0x610A0540,  0x54199643,   0x350112B5,
    0x91000436,  0x4469AA15
    };
  void riscv_shift_q31(
  q31_t * pSrc,
  int8_t shiftBits,
  q31_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign = (shiftBits & 0x80);             /* Sign of shiftBits */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;
  while(blkCnt > 0u)
  {
    /* C = A (>> or <<) shiftBits */
    /* Shift the input and then store the result in the destination buffer. */
    *pDst++ = (sign == 0u) ? clip_q63_to_q31((q63_t) * pSrc++ << shiftBits) :
      (*pSrc++ >> -shiftBits);

    /* Decrement the loop counter */
    blkCnt--;
  }

}
  riscv_shift_q31(srcA_buf_q31, 1, result_q31, MAX_BLOCKSIZE);
  PRINT_Q(result_q31,MAX_BLOCKSIZE);
  printf("\nCorrect answer:\n");
  printf("0x7FFFFFFF 0x26EB4270 0x2ACA88E6 0x7FFFFFFF 0x2310A986 0x39430522 0x7FFFFFFF 0x16400B20 0x80000000 0x80000000 0x6733A246 0x4A3177A2 0x356406F4 0x7FFFFFFF 0x7FFFFFFF 0x4DFEAAD2 0x24262B2 0xE2479AEE 0x80000000 0x7FFFFFFF 0x7FFFFFFF 0x7FFFFFFF 0x80000000 0x6D676D2E 0x448B2C86 0x66E786D0 0xC35203A6 0x7FFFFFFF 0x7FFFFFFF 0x6A02256A 0x80000000 0x7FFFFFFF ");
    printf("\n");
  printf("hello world!\n");
  return EXIT_SUCCESS;
}