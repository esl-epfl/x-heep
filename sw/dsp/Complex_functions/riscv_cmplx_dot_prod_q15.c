#include "riscv_math.h"
void riscv_cmplx_dot_prod_q15(
q15_t * pSrcA,
q15_t * pSrcB,
uint32_t numSamples,
q31_t * realResult,
q31_t * imagResult)
{
q63_t real_sum = 0, imag_sum = 0;              /* Temporary result storage */
q15_t a0,b0,c0,d0;

while(numSamples > 0u)
{
a0 = *pSrcA++;
b0 = *pSrcA++;
c0 = *pSrcB++;
d0 = *pSrcB++;  
/*real and imaginary parts*/
real_sum += a0 * c0;
imag_sum += a0 * d0;
real_sum -= b0 * d0;
imag_sum += b0 * c0;


    /* Decrement the loop counter */
numSamples--;
}
/* Store the real and imaginary results in 8.24 format  */
/* Convert real data in 34.30 to 8.24 by 6 right shifts */
*realResult = (q31_t) (real_sum >> 6);
/* Convert imaginary data in 34.30 to 8.24 by 6 right shifts */
*imagResult = (q31_t) (imag_sum >> 6);
}