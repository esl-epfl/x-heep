/* ----------------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_conv_f32.c    
*    
* Description:	Convolution of floating-point sequences.    
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
 * @defgroup Conv Convolution    
 *    
 * Convolution is a mathematical operation that operates on two finite length vectors to generate a finite length output vector.    
 * Convolution is similar to correlation and is frequently used in filtering and data analysis.    
 * The CMSIS DSP library contains functions for convolving Q7, Q15, Q31, and floating-point data types.    
 * The library also provides fast versions of the Q15 and Q31 functions on Cortex-M4 and Cortex-M3.    
 *    
 * \par Algorithm    
 * Let <code>a[n]</code> and <code>b[n]</code> be sequences of length <code>srcALen</code> and <code>srcBLen</code> samples respectively.    
 * Then the convolution    
 *    
 * <pre>    
 *                   c[n] = a[n] * b[n]    
 * </pre>    
 *    
 * \par    
 * is defined as    
 * \image html ConvolutionEquation.gif    
 * \par    
 * Note that <code>c[n]</code> is of length <code>srcALen + srcBLen - 1</code> and is defined over the interval <code>n=0, 1, 2, ..., srcALen + srcBLen - 2</code>.    
 * <code>pSrcA</code> points to the first input vector of length <code>srcALen</code> and    
 * <code>pSrcB</code> points to the second input vector of length <code>srcBLen</code>.    
 * The output result is written to <code>pDst</code> and the calling function must allocate <code>srcALen+srcBLen-1</code> words for the result.    
 *    
 * \par    
 * Conceptually, when two signals <code>a[n]</code> and <code>b[n]</code> are convolved,    
 * the signal <code>b[n]</code> slides over <code>a[n]</code>.    
 * For each offset \c n, the overlapping portions of a[n] and b[n] are multiplied and summed together.    
 *    
 * \par    
 * Note that convolution is a commutative operation:    
 *    
 * <pre>    
 *                   a[n] * b[n] = b[n] * a[n].    
 * </pre>    
 *    
 * \par    
 * This means that switching the A and B arguments to the convolution functions has no effect.    
 *    
 * <b>Fixed-Point Behavior</b>    
 *    
 * \par    
 * Convolution requires summing up a large number of intermediate products.    
 * As such, the Q7, Q15, and Q31 functions run a risk of overflow and saturation.    
 * Refer to the function specific documentation below for further details of the particular algorithm used.    
 *
 *
 * <b>Fast Versions</b>
 *
 * \par 
 * Fast versions are supported for Q31 and Q15.  Cycles for Fast versions are less compared to Q31 and Q15 of conv and the design requires
 * the input signals should be scaled down to avoid intermediate overflows.   
 *
 *
 * <b>Opt Versions</b>
 *
 * \par 
 * Opt versions are supported for Q15 and Q7.  Design uses internal scratch buffer for getting good optimisation.
 * These versions are optimised in cycles and consumes more memory(Scratch memory) compared to Q15 and Q7 versions 
 */

/**    
 * @addtogroup Conv    
 * @{    
 */

/**    
 * @brief Convolution of floating-point sequences.    
 * @param[in] *pSrcA points to the first input sequence.    
 * @param[in] srcALen length of the first input sequence.    
 * @param[in] *pSrcB points to the second input sequence.    
 * @param[in] srcBLen length of the second input sequence.    
 * @param[out] *pDst points to the location where the output result is written.  Length srcALen+srcBLen-1.    
 * @return none.    
 */

void riscv_conv_f32(
  float32_t * pSrcA,
  uint32_t srcALen,
  float32_t * pSrcB,
  uint32_t srcBLen,
  float32_t * pDst)
{

  float32_t *pIn1 = pSrcA;                       /* inputA pointer */
  float32_t *pIn2 = pSrcB;                       /* inputB pointer */
  float32_t sum;                                 /* Accumulator */
  uint32_t i, j;                                 /* loop counters */

  /* Loop to calculate convolution for output length number of times */
  for (i = 0u; i < ((srcALen + srcBLen) - 1u); i++)
  {
    /* Initialize sum with zero to carry out MAC operations */
    sum = 0.0f;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0u; j <= i; j++)
    {
      /* Check the array limitations */
      if((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += pIn1[j] * pIn2[i - j];
      }
    }
    /* Store the output in the destination buffer */
    pDst[i] = sum;
  }
}

/**    
 * @} end of Conv group    
 */
