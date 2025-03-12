/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. October 2015
* $Revision: 	V.1.4.5 a
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_cmplx_conj_q15.c    
*    
* Description:	Q15 complex conjugate.    
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

#include "riscv_math.h"
#include "x_heep_emul.h"
/**    
 * @ingroup groupCmplxMath    
 */

/**    
 * @addtogroup cmplx_conj    
 * @{    
 */

/**    
 * @brief  Q15 complex conjugate.    
 * @param  *pSrc points to the input vector    
 * @param  *pDst points to the output vector    
 * @param  numSamples number of complex samples in each vector    
 * @return none.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function uses saturating arithmetic.    
 * The Q15 value -1 (0x8000) will be saturated to the maximum allowable positive value 0x7FFF.    
 */

void riscv_cmplx_conj_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t numSamples)
{

  q15_t in;
  q31_t out;
#if defined (USE_DSP_RISCV)

  while (numSamples > 0u)
  {
    /* realOut + j (imagOut) = realIn+ j (-1) imagIn */
    /* Calculate Complex Conjugate and then store the results in the destination buffer. */
    *pDst++ = *pSrc++;/*real as it is*/
    out = -(*pSrc++); /*negate the imaginary part*/
    *pDst++ = (q15_t)x_heep_clip(out,15);/*saturate the imaginary part (in case of -2^15 it will become 2^15 - 1)*/
    /* Decrement the loop counter */
    numSamples--;
  }
#else
  while(numSamples > 0u)
  {
    /* realOut + j (imagOut) = realIn+ j (-1) imagIn */
    /* Calculate Complex Conjugate and then store the results in the destination buffer. */
    *pDst++ = *pSrc++;
    in = *pSrc++;
    *pDst++ = (in == (q15_t) 0x8000) ? 0x7fff : -in;

    /* Decrement the loop counter */
    numSamples--;
  }
#endif

}

/**    
 * @} end of cmplx_conj group    
 */

