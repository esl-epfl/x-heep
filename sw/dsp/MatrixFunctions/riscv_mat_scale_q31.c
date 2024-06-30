/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_mat_scale_q31.c    
*    
* Description:	Multiplies a Q31 matrix by a scalar.    
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
* ------------------------------------------------ */

#include "riscv_math.h"

/**        
 * @ingroup groupMatrix        
 */

/**        
 * @addtogroup MatrixScale        
 * @{        
 */

/**        
 * @brief Q31 matrix scaling.        
 * @param[in]       *pSrc points to input matrix        
 * @param[in]       scaleFract fractional portion of the scale factor        
 * @param[in]       shift number of bits to shift the result by        
 * @param[out]      *pDst points to output matrix structure        
 * @return     		The function returns either        
 * <code>RISCV_MATH_SIZE_MISMATCH</code> or <code>RISCV_MATH_SUCCESS</code> based on the outcome of size checking.        
 *        
 * @details        
 * <b>Scaling and Overflow Behavior:</b>        
 * \par        
 * The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.31 format.        
 * These are multiplied to yield a 2.62 intermediate result and this is shifted with saturation to 1.31 format.        
 */

riscv_status riscv_mat_scale_q31(
  const riscv_matrix_instance_q31 * pSrc,
  q31_t scaleFract,
  int32_t shift,
  riscv_matrix_instance_q31 * pDst)
{
  q31_t *pIn = pSrc->pData;                      /* input data matrix pointer */
  q31_t *pOut = pDst->pData;                     /* output data matrix pointer */
  uint32_t numSamples;                           /* total number of elements in the matrix */
  int32_t totShift = shift + 1;                  /* shift to apply after scaling */
  uint32_t blkCnt;                               /* loop counters  */
  riscv_status status;                             /* status of matrix scaling      */
  q31_t in1, in2, out1;                          /* temporary variabels */


#ifdef RISCV_MATH_MATRIX_CHECK
  /* Check for matrix mismatch  */
  if((pSrc->numRows != pDst->numRows) || (pSrc->numCols != pDst->numCols))
  {
    /* Set status as RISCV_MATH_SIZE_MISMATCH */
    status = RISCV_MATH_SIZE_MISMATCH;
  }
  else
#endif //    #ifdef RISCV_MATH_MATRIX_CHECK
  {
    /* Total number of samples in the input matrix */
    numSamples = (uint32_t) pSrc->numRows * pSrc->numCols;
    blkCnt = numSamples;
    while(blkCnt > 0u)
    {
      /* C(m,n) = A(m,n) * k */
      /* Scale, saturate and then store the results in the destination buffer. */
      in1 = *pIn++;

      in2 = ((q63_t) in1 * scaleFract) >> 32;

      out1 = in2 << totShift;

      if(in2 != (out1 >> totShift))
        out1 = 0x7FFFFFFF ^ (in2 >> 31);

      *pOut++ = out1;

      /* Decrement the numSamples loop counter */
      blkCnt--;
    }

    /* Set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**        
 * @} end of MatrixScale group        
 */
