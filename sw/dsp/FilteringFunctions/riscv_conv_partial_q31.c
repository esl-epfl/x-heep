/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_conv_partial_q31.c    
*    
* Description:	Partial convolution of Q31 sequences.    
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
 * @addtogroup PartialConv    
 * @{    
 */

/**    
 * @brief Partial convolution of Q31 sequences.    
 * @param[in]       *pSrcA points to the first input sequence.    
 * @param[in]       srcALen length of the first input sequence.    
 * @param[in]       *pSrcB points to the second input sequence.    
 * @param[in]       srcBLen length of the second input sequence.    
 * @param[out]      *pDst points to the location where the output result is written.    
 * @param[in]       firstIndex is the first output sample to start with.    
 * @param[in]       numPoints is the number of output points to be computed.    
 * @return Returns either RISCV_MATH_SUCCESS if the function completed correctly or RISCV_MATH_ARGUMENT_ERROR if the requested subset is not in the range [0 srcALen+srcBLen-2].    
 *    
 * See <code>riscv_conv_partial_fast_q31()</code> for a faster but less precise implementation of this function for Cortex-M3 and Cortex-M4.    
 */

riscv_status riscv_conv_partial_q31(
  q31_t * pSrcA,
  uint32_t srcALen,
  q31_t * pSrcB,
  uint32_t srcBLen,
  q31_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{

  q31_t *pIn1 = pSrcA;                           /* inputA pointer */
  q31_t *pIn2 = pSrcB;                           /* inputB pointer */
  q63_t sum;                                     /* Accumulator */
  uint32_t i, j;                                 /* loop counters */
  riscv_status status;                             /* status of Partial convolution */

  /* Check for range of output samples to be calculated */
  if((firstIndex + numPoints) > ((srcALen + (srcBLen - 1u))))
  {
    /* Set status as RISCV_ARGUMENT_ERROR */
    status = RISCV_MATH_ARGUMENT_ERROR;
  }
  else
  {
    /* Loop to calculate convolution for output length number of values */
    for (i = firstIndex; i <= (firstIndex + numPoints - 1); i++)
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
    /* set status as RISCV_SUCCESS as there are no argument errors */
    status = RISCV_MATH_SUCCESS;
  }

  return (status);

}

/**    
 * @} end of PartialConv group    
 */
