/* ----------------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5  
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_float_to_q31.c    
*    
* Description:	Converts the elements of the floating-point vector to Q31 vector.    
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

/**    
 * @ingroup groupSupport    
 */

/**    
 * @defgroup float_to_x  Convert 32-bit floating point value    
 */

/**    
 * @addtogroup float_to_x    
 * @{    
 */

/**    
 * @brief Converts the elements of the floating-point vector to Q31 vector.    
 * @param[in]       *pSrc points to the floating-point input vector    
 * @param[out]      *pDst points to the Q31 output vector   
 * @param[in]       blockSize length of the input vector    
 * @return none.    
 *    
 *\par Description:    
 * \par   
 * The equation used for the conversion process is:    
 *   
 * <pre>    
 * 	pDst[n] = (q31_t)(pSrc[n] * 2147483648);   0 <= n < blockSize.    
 * </pre>    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function uses saturating arithmetic.    
 * Results outside of the allowable Q31 range[0x80000000 0x7FFFFFFF] will be saturated.    
 *   
 * \note In order to apply rounding, the library should be rebuilt with the ROUNDING macro     
 * defined in the preprocessor section of project options.     
 */


void riscv_float_to_q31(
  float32_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  float32_t *pIn = pSrc;                         /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#ifdef RISCV_MATH_ROUNDING

  float32_t in;

#endif /*      #ifdef RISCV_MATH_ROUNDING        */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {

#ifdef RISCV_MATH_ROUNDING

    /* C = A * 2147483648 */
    /* convert from float to Q31 and then store the results in the destination buffer */
    in = *pIn++;
    in = (in * 2147483648.0f);
    in += in > 0 ? 0.5f : -0.5f;
    *pDst++ = clip_q63_to_q31((q63_t) (in));

#else

    /* C = A * 2147483648 */
    /* convert from float to Q31 and then store the results in the destination buffer */
    *pDst++ = clip_q63_to_q31((q63_t) (*pIn++ * 2147483648.0f));

#endif /*      #ifdef RISCV_MATH_ROUNDING        */

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**    
 * @} end of float_to_x group    
 */
