#include "riscv_math.h"
#include "x_heep_emul.h"//AFTER MODIFICATION FOR SQRT FUNCTION

void riscv_cmplx_mag_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{
  q31_t acc0, acc1;                              /* Accumulators */
  q15_t real, imag;                              /* Temporary variables to hold input values */
#if defined (USE_DSP_RISCV)

  shortV *VectInA;
  while (numSamples > 0u)
  {
    VectInA =  (shortV*)pSrc;/*read 2 elements (a complex number)*/
    acc0 = x_heep_dotp2(*VectInA, *VectInA); /*dot product with itself == magnitude squared*/
    q15_t x=(q15_t)(((q63_t)acc0) >> 17);
    riscv_sqrt_q15(x, pDst++); /*normalize and square root*/
    pSrc+=2;
    numSamples--;
  }



#else
  while(numSamples > 0u)
  {
    /* out = sqrt(real * real + imag * imag) */
    real = *pSrc++;
    imag = *pSrc++;

    acc0 = (real * real);
    acc1 = (imag * imag);
    /* store the result in 2.14 format in the destination buffer. */
    riscv_sqrt_q15((q15_t) (((q63_t) acc0 + acc1) >> 17), pDst++);

    /* Decrement the loop counter */
    numSamples--;
  }
#endif
}