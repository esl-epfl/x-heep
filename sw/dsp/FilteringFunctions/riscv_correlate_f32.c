/* ----------------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_correlate_f32.c    
*    
* Description:	 Correlation of floating-point sequences.    
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
 * @defgroup Corr Correlation    
 *    
 * Correlation is a mathematical operation that is similar to convolution.    
 * As with convolution, correlation uses two signals to produce a third signal.    
 * The underlying algorithms in correlation and convolution are identical except that one of the inputs is flipped in convolution.    
 * Correlation is commonly used to measure the similarity between two signals.    
 * It has applications in pattern recognition, cryptanalysis, and searching.    
 * The CMSIS library provides correlation functions for Q7, Q15, Q31 and floating-point data types.    
 * Fast versions of the Q15 and Q31 functions are also provided.    
 *    
 * \par Algorithm    
 * Let <code>a[n]</code> and <code>b[n]</code> be sequences of length <code>srcALen</code> and <code>srcBLen</code> samples respectively.    
 * The convolution of the two signals is denoted by    
 * <pre>    
 *                   c[n] = a[n] * b[n]    
 * </pre>    
 * In correlation, one of the signals is flipped in time    
 * <pre>    
 *                   c[n] = a[n] * b[-n]    
 * </pre>    
 *    
 * \par    
 * and this is mathematically defined as    
 * \image html CorrelateEquation.gif    
 * \par    
 * The <code>pSrcA</code> points to the first input vector of length <code>srcALen</code> and <code>pSrcB</code> points to the second input vector of length <code>srcBLen</code>.    
 * The result <code>c[n]</code> is of length <code>2 * max(srcALen, srcBLen) - 1</code> and is defined over the interval <code>n=0, 1, 2, ..., (2 * max(srcALen, srcBLen) - 2)</code>.    
 * The output result is written to <code>pDst</code> and the calling function must allocate <code>2 * max(srcALen, srcBLen) - 1</code> words for the result.    
 *    
 * <b>Note</b>   
 * \par  
 * The <code>pDst</code> should be initialized to all zeros before being used.  
 *  
 * <b>Fixed-Point Behavior</b>    
 * \par    
 * Correlation requires summing up a large number of intermediate products.    
 * As such, the Q7, Q15, and Q31 functions run a risk of overflow and saturation.    
 * Refer to the function specific documentation below for further details of the particular algorithm used.    
 *
 *
 * <b>Fast Versions</b>
 *
 * \par 
 * Fast versions are supported for Q31 and Q15.  Cycles for Fast versions are less compared to Q31 and Q15 of correlate and the design requires
 * the input signals should be scaled down to avoid intermediate overflows.   
 *
 *
 * <b>Opt Versions</b>
 *
 * \par 
 * Opt versions are supported for Q15 and Q7.  Design uses internal scratch buffer for getting good optimisation.
 * These versions are optimised in cycles and consumes more memory(Scratch memory) compared to Q15 and Q7 versions of correlate 
 */

/**    
 * @addtogroup Corr    
 * @{    
 */
/**    
 * @brief Correlation of floating-point sequences.    
 * @param[in]  *pSrcA points to the first input sequence.    
 * @param[in]  srcALen length of the first input sequence.    
 * @param[in]  *pSrcB points to the second input sequence.    
 * @param[in]  srcBLen length of the second input sequence.    
 * @param[out] *pDst points to the location where the output result is written.  Length 2 * max(srcALen, srcBLen) - 1.    
 * @return none.    
 */

void riscv_correlate_f32(
  float32_t * pSrcA,
  uint32_t srcALen,
  float32_t * pSrcB,
  uint32_t srcBLen,
  float32_t * pDst)
{



  float32_t *pIn1 = pSrcA;                       /* inputA pointer */
  float32_t *pIn2 = pSrcB + (srcBLen - 1u);      /* inputB pointer */
  float32_t sum;                                 /* Accumulator */
  uint32_t i = 0u, j;                            /* loop counters */
  uint32_t inv = 0u;                             /* Reverse order flag */
  uint32_t tot = 0u;                             /* Length */

  /* The algorithm implementation is based on the lengths of the inputs. */
  /* srcB is always made to slide across srcA. */
  /* So srcBLen is always considered as shorter or equal to srcALen */
  /* But CORR(x, y) is reverse of CORR(y, x) */
  /* So, when srcBLen > srcALen, output pointer is made to point to the end of the output buffer */
  /* and a varaible, inv is set to 1 */
  /* If lengths are not equal then zero pad has to be done to  make the two    
   * inputs of same length. But to improve the performance, we assume zeroes    
   * in the output instead of zero padding either of the the inputs*/
  /* If srcALen > srcBLen, (srcALen - srcBLen) zeroes has to included in the    
   * starting of the output buffer */
  /* If srcALen < srcBLen, (srcALen - srcBLen) zeroes has to included in the   
   * ending of the output buffer */
  /* Once the zero padding is done the remaining of the output is calcualted   
   * using convolution but with the shorter signal time shifted. */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2u);

  if(srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if(srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1u);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate convolution for output length number of times */
  for (i = 0u; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0.0f;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0u; j <= i; j++)
    {
      /* Check the array limitations */
      if((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += pIn1[j] * pIn2[-((int32_t) i - j)];
      }
    }
    /* Store the output in the destination buffer */
    if(inv == 1)
      *pDst-- = sum;
    else
      *pDst++ = sum;
  }


}

/**    
 * @} end of Corr group    
 */
