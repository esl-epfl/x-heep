/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_fir_q31.c    
*    
* Description:	Q31 FIR filter processing function.    
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
 * @addtogroup FIR    
 * @{    
 */

/**    
 * @param[in] *S points to an instance of the Q31 FIR filter structure.    
 * @param[in] *pSrc points to the block of input data.    
 * @param[out] *pDst points to the block of output data.    
 * @param[in] blockSize number of samples to process per call.    
 * @return none.    
 *    
 * @details    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function is implemented using an internal 64-bit accumulator.    
 * The accumulator has a 2.62 format and maintains full precision of the intermediate multiplication results but provides only a single guard bit.    
 * Thus, if the accumulator result overflows it wraps around rather than clip.    
 * In order to avoid overflows completely the input signal must be scaled down by log2(numTaps) bits.    
 * After all multiply-accumulates are performed, the 2.62 accumulator is right shifted by 31 bits and saturated to 1.31 format to yield the final result.  
 *    
 * \par    
 * Refer to the function <code>riscv_fir_fast_q31()</code> for a faster but less precise implementation of this filter for Cortex-M3 and Cortex-M4.    
 */

void riscv_fir_q31(
  const riscv_fir_instance_q31 * S,
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t *pState = S->pState;                     /* State pointer */
  q31_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q31_t *pStateCurnt;                            /* Points to the current sample of the state */



  q31_t *px;                                     /* Temporary pointer for state */
  q31_t *pb;                                     /* Temporary pointer for coefficient buffer */
  q63_t acc;                                     /* Accumulator */
  uint32_t numTaps = S->numTaps;                 /* Length of the filter */
  uint32_t i, tapCnt, blkCnt;                    /* Loop counters */

  /* S->pState buffer contains previous frame (numTaps - 1) samples */
  /* pStateCurnt points to the location where the new input data should be written */
  pStateCurnt = &(S->pState[(numTaps - 1u)]);

  /* Initialize blkCnt with blockSize */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* Copy one sample at a time into state buffer */
    *pStateCurnt++ = *pSrc++;

    /* Set the accumulator to zero */
    acc = 0;

    /* Initialize state pointer */
    px = pState;

    /* Initialize Coefficient pointer */
    pb = pCoeffs;

    i = numTaps;

    /* Perform the multiply-accumulates */
    do
    {
      /* acc =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0] */
      acc += (q63_t) * px++ * *pb++;
      i--;
    } while(i > 0u);

    /* The result is in 2.62 format.  Convert to 1.31         
     ** Then store the output in the destination buffer. */
    *pDst++ = (q31_t) (acc >> 31u);

    /* Advance state pointer by 1 for the next sample */
    pState = pState + 1;

    /* Decrement the samples loop counter */
    blkCnt--;
  }

  /* Processing is complete.         
   ** Now copy the last numTaps - 1 samples to the starting of the state buffer.       
   ** This prepares the state buffer for the next function call. */

  /* Points to the start of the state buffer */
  pStateCurnt = S->pState;

  /* Copy numTaps number of values */
  tapCnt = numTaps - 1u;

  /* Copy the data */
  while(tapCnt > 0u)
  {
    *pStateCurnt++ = *pState++;

    /* Decrement the loop counter */
    tapCnt--;
  }

}

/**    
 * @} end of FIR group    
 */
