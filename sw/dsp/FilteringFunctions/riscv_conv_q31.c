/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_conv_q31.c    
*    
* Description:	Convolution of Q31 sequences.  
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
 * @ingroup groupFilters    
 */

/**    
 * @addtogroup Conv    
 * @{    
 */

/**    
 * @brief Convolution of Q31 sequences.    
 * @param[in] *pSrcA points to the first input sequence.    
 * @param[in] srcALen length of the first input sequence.    
 * @param[in] *pSrcB points to the second input sequence.    
 * @param[in] srcBLen length of the second input sequence.    
 * @param[out] *pDst points to the location where the output result is written.  Length srcALen+srcBLen-1.    
 * @return none.    
 *    
 * @details    
 * <b>Scaling and Overflow Behavior:</b>    
 *    
 * \par    
 * The function is implemented using an internal 64-bit accumulator.    
 * The accumulator has a 2.62 format and maintains full precision of the intermediate multiplication results but provides only a single guard bit.    
 * There is no saturation on intermediate additions.    
 * Thus, if the accumulator overflows it wraps around and distorts the result.    
 * The input signals should be scaled down to avoid intermediate overflows.    
 * Scale down the inputs by log2(min(srcALen, srcBLen)) (log2 is read as log to the base 2) times to avoid overflows,    
 * as maximum of min(srcALen, srcBLen) number of additions are carried internally.    
 * The 2.62 accumulator is right shifted by 31 bits and saturated to 1.31 format to yield the final result.    
 *    
 * \par    
 * See <code>riscv_conv_fast_q31()</code> for a faster but less precise implementation of this function for Cortex-M3 and Cortex-M4.    
 */

void riscv_conv_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst)
{

  q31_t *pIn1 = pSrcA;                           /* input pointer */
  q31_t *pIn2 = pSrcB;                           /* coefficient pointer */
  q63_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counter */

  /* Loop to calculate output of convolution for output length number of times */
  for (i = 0; i < (srcALen + srcBLen - 1); i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0; j <= i; j++)
    {
      /* Check the array limitations */
      if(((i - j) < srcBLen) && (j < srcALen))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += ((q63_t) pIn1[j] * (pIn2[i - j]));
      }
    }

    /* Store the output in the destination buffer */
    pDst[i] = (q31_t) (sum >> 31u);
  }

}

/**    
 * @} end of Conv group    
 */
