#include "riscv_math.h"
#include "x_heep_emul.h"

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

