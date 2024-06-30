/* ----------------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_conv_partial_f32.c    
*    
* Description:	Partial convolution of floating-point sequences.    
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
* -------------------------------------------------------------------------- */

#include "riscv_math.h"

/**    
 * @ingroup groupFilters    
 */

/**    
 * @defgroup PartialConv Partial Convolution    
 *    
 * Partial Convolution is equivalent to Convolution except that a subset of the output samples is generated.    
 * Each function has two additional arguments.    
 * <code>firstIndex</code> specifies the starting index of the subset of output samples.    
 * <code>numPoints</code> is the number of output samples to compute.    
 * The function computes the output in the range    
 * <code>[firstIndex, ..., firstIndex+numPoints-1]</code>.    
 * The output array <code>pDst</code> contains <code>numPoints</code> values.    
 *    
 * The allowable range of output indices is [0 srcALen+srcBLen-2].    
 * If the requested subset does not fall in this range then the functions return RISCV_MATH_ARGUMENT_ERROR.    
 * Otherwise the functions return RISCV_MATH_SUCCESS.    
 * \note Refer riscv_conv_f32() for details on fixed point behavior.   
 *
 * 
 * <b>Fast Versions</b>
 *
 * \par 
 * Fast versions are supported for Q31 and Q15 of partial convolution.  Cycles for Fast versions are less compared to Q31 and Q15 of partial conv and the design requires
 * the input signals should be scaled down to avoid intermediate overflows.   
 *
 *
 * <b>Opt Versions</b>
 *
 * \par 
 * Opt versions are supported for Q15 and Q7.  Design uses internal scratch buffer for getting good optimisation.
 * These versions are optimised in cycles and consumes more memory(Scratch memory) compared to Q15 and Q7 versions of partial convolution
 */

/**    
 * @addtogroup PartialConv    
 * @{    
 */

/**    
 * @brief Partial convolution of floating-point sequences.    
 * @param[in]       *pSrcA points to the first input sequence.    
 * @param[in]       srcALen length of the first input sequence.    
 * @param[in]       *pSrcB points to the second input sequence.    
 * @param[in]       srcBLen length of the second input sequence.    
 * @param[out]      *pDst points to the location where the output result is written.    
 * @param[in]       firstIndex is the first output sample to start with.    
 * @param[in]       numPoints is the number of output points to be computed.    
 * @return  Returns either RISCV_MATH_SUCCESS if the function completed correctly or RISCV_MATH_ARGUMENT_ERROR if the requested subset is not in the range [0 srcALen+srcBLen-2].    
 */

riscv_status riscv_conv_partial_f32(
  float32_t * pSrcA,
  uint32_t srcALen,
  float32_t * pSrcB,
  uint32_t srcBLen,
  float32_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{

  float32_t *pIn1 = pSrcA;                       /* inputA pointer */
  float32_t *pIn2 = pSrcB;                       /* inputB pointer */
  float32_t sum;                                 /* Accumulator */
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
      sum = 0.0f;

      /* Loop to perform MAC operations according to convolution equation */
      for (j = 0u; j <= i; j++)
      {
        /* Check the array limitations for inputs */
        if((((i - j) < srcBLen) && (j < srcALen)))
        {
          /* z[i] += x[i-j] * y[j] */
          sum += pIn1[j] * pIn2[i - j];
        }
      }
      /* Store the output in the destination buffer */
      pDst[i] = sum;
    }
    /* set status as RISCV_SUCCESS as there are no argument errors */
    status = RISCV_MATH_SUCCESS;
  }
  return (status);



}

/**    
 * @} end of PartialConv group    
 */
