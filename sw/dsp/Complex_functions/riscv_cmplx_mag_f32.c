/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_cmplx_mag_f32.c    
*    
* Description:	Floating-point complex magnitude.    
*    
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the 
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.  

 Modifications 2017  Mostafa Saleh       (Ported to RISC-V PULPino)
 Modifications 2024  ESL
* ---------------------------------------------------------------------------- */
#include <math.h>
#include <stdio.h>
#include "riscv_math.h"


/**    
 * @ingroup groupCmplxMath    
 */

/**    
 * @defgroup cmplx_mag Complex Magnitude    
 *    
 * Computes the magnitude of the elements of a complex data vector.    
 *   
 * The <code>pSrc</code> points to the source data and    
 * <code>pDst</code> points to the where the result should be written.    
 * <code>numSamples</code> specifies the number of complex samples    
 * in the input array and the data is stored in an interleaved fashion    
 * (real, imag, real, imag, ...).    
 * The input array has a total of <code>2*numSamples</code> values;    
 * the output array has a total of <code>numSamples</code> values.    
 * The underlying algorithm is used:    
 *    
 * <pre>    
 * for(n=0; n<numSamples; n++) {    
 *     pDst[n] = sqrt(pSrc[(2*n)+0]^2 + pSrc[(2*n)+1]^2);    
 * }    
 * </pre>    
 *    
 * There are separate functions for floating-point, Q15, and Q31 data types.    
 */

/**    
 * @addtogroup cmplx_mag    
 * @{    
 */
/**    
 * @brief Floating-point complex magnitude.    
 * @param[in]       *pSrc points to complex input buffer    
 * @param[out]      *pDst points to real output buffer    
 * @param[in]       numSamples number of complex samples in the input vector    
 * @return none.    
 *    
 */


void riscv_cmplx_mag_f32(
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t numSamples)
{
  float32_t realIn, imagIn;                      /* Temporary variables to hold input values */
  while(numSamples > 0u)
  {
    /* out = sqrt((real * real) + (imag * imag)) */
    realIn = *pSrc++;
    imagIn = *pSrc++;
    /* store the result in the destination buffer. */
    riscv_sqrt_f32((realIn * realIn) + (imagIn * imagIn), pDst++);

    /* Decrement the loop counter */
    numSamples--;
  }


}

/**    
 * @} end of cmplx_mag group    
 */