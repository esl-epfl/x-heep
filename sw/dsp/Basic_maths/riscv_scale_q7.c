#include "riscv_math.h"
#include "x_heep_emul.h"


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

