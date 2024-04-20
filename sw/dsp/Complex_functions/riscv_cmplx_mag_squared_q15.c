#include "riscv_math.h"
#include "x_heep_emul.h"

void riscv_cmplx_mag_squared_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{
  q31_t acc0, acc1;                              /* Accumulators */

  q15_t real, imag;                              /* Temporary variables to store real and imaginary values */

#if defined (USE_DSP_RISCV)

  shortV *VectInA;
  while (numSamples > 0u)
  {
    VectInA =  (shortV*)pSrc;/*read 2 elements (a complex number)*/
    acc0 = x_heep_dotp2(*VectInA, *VectInA);/*dot product with itself == magnitude squared*/
    *pDst++ = (q15_t) (((q63_t)acc0) >> 17);/*normalize*/
    pSrc+=2;
    numSamples--;
  }

#else
  while(numSamples > 0u)
  {
    /* out = ((real * real) + (imag * imag)) */
    real = *pSrc++;
    imag = *pSrc++;
    acc0 = (real * real);
    acc1 = (imag * imag);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ = (q15_t) (((q63_t) acc0 + acc1) >> 17);

    /* Decrement the loop counter */
    numSamples--;
  }
#endif
}