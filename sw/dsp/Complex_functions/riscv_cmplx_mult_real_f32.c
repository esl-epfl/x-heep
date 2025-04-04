/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_cmplx_mult_real_f32.c    
*    
* Description:	Floating-point complex by real multiplication    
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
* -------------------------------------------------------------------- */

#include "riscv_math.h"

/**        
 * @ingroup groupCmplxMath        
 */

/**        
 * @defgroup CmplxByRealMult Complex-by-Real Multiplication        
 *        
 * Multiplies a complex vector by a real vector and generates a complex result.        
 * The data in the complex arrays is stored in an interleaved fashion        
 * (real, imag, real, imag, ...).        
 * The parameter <code>numSamples</code> represents the number of complex        
 * samples processed.  The complex arrays have a total of <code>2*numSamples</code>        
 * real values while the real array has a total of <code>numSamples</code>        
 * real values.        
 *        
 * The underlying algorithm is used:        
 *        
 * <pre>        
 * for(n=0; n<numSamples; n++) {        
 *     pCmplxDst[(2*n)+0] = pSrcCmplx[(2*n)+0] * pSrcReal[n];        
 *     pCmplxDst[(2*n)+1] = pSrcCmplx[(2*n)+1] * pSrcReal[n];        
 * }        
 * </pre>        
 *        
 * There are separate functions for floating-point, Q15, and Q31 data types.        
 */

/**        
 * @addtogroup CmplxByRealMult        
 * @{        
 */


/**        
 * @brief  Floating-point complex-by-real multiplication        
 * @param[in]  *pSrcCmplx points to the complex input vector        
 * @param[in]  *pSrcReal points to the real input vector        
 * @param[out]  *pCmplxDst points to the complex output vector        
 * @param[in]  numSamples number of samples in each vector        
 * @return none.        
 */

void riscv_cmplx_mult_real_f32(
  float32_t * pSrcCmplx,
  float32_t * pSrcReal,
  float32_t * pCmplxDst,
  uint32_t numSamples)
{
  float32_t in;                                  /* Temporary variable to store input value */
  uint32_t blkCnt;                               /* loop counters */
  blkCnt = numSamples;

  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    in = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ = (*pSrcCmplx++) * (in);
    *pCmplxDst++ = (*pSrcCmplx++) * (in);

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }
}

/**        
 * @} end of CmplxByRealMult group        
 */