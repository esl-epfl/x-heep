/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:        arm_mat_scale_f32.c    
*    
* Description:	Multiplies a floating-point matrix by a scalar.    
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
 * @ingroup groupMatrix        
 */

/**        
 * @defgroup MatrixScale Matrix Scale        
 *        
 * Multiplies a matrix by a scalar.  This is accomplished by multiplying each element in the        
 * matrix by the scalar.  For example:        
 * \image html MatrixScale.gif "Matrix Scaling of a 3 x 3 matrix"        
 *        
 * The function checks to make sure that the input and output matrices are of the same size.        
 *        
 * In the fixed-point Q15 and Q31 functions, <code>scale</code> is represented by        
 * a fractional multiplication <code>scaleFract</code> and an arithmetic shift <code>shift</code>.        
 * The shift allows the gain of the scaling operation to exceed 1.0.        
 * The overall scale factor applied to the fixed-point data is        
 * <pre>        
 *     scale = scaleFract * 2^shift.        
 * </pre>        
 */

/**        
 * @addtogroup MatrixScale        
 * @{        
 */

/**        
 * @brief Floating-point matrix scaling.        
 * @param[in]       *pSrc points to input matrix structure        
 * @param[in]       scale scale factor to be applied         
 * @param[out]      *pDst points to output matrix structure        
 * @return     		The function returns either <code>RISCV_MATH_SIZE_MISMATCH</code>         
 * or <code>RISCV_MATH_SUCCESS</code> based on the outcome of size checking.        
 *        
 */

riscv_status riscv_mat_scale_f32(
  const riscv_matrix_instance_f32 * pSrc,
  float32_t scale,
  riscv_matrix_instance_f32 * pDst)
{
  float32_t *pIn = pSrc->pData;                  /* input data matrix pointer */
  float32_t *pOut = pDst->pData;                 /* output data matrix pointer */
  uint32_t numSamples;                           /* total number of elements in the matrix */
  uint32_t blkCnt;                               /* loop counters */
  riscv_status status;                             /* status of matrix scaling     */

#ifdef RISCV_MATH_MATRIX_CHECK
  /* Check for matrix mismatch condition */
  if((pSrc->numRows != pDst->numRows) || (pSrc->numCols != pDst->numCols))
  {
    /* Set status as RISCV_MATH_SIZE_MISMATCH */
    status = RISCV_MATH_SIZE_MISMATCH;
  }
  else
#endif /*    #ifdef RISCV_MATH_MATRIX_CHECK    */
  {
    /* Total number of samples in the input matrix */
    numSamples = (uint32_t) pSrc->numRows * pSrc->numCols;


    /* Initialize blkCnt with number of samples */
    blkCnt = numSamples;

    while(blkCnt > 0u)
    {
      /* C(m,n) = A(m,n) * scale */
      /* The results are stored in the destination buffer. */
      *pOut++ = (*pIn++) * scale;

      /* Decrement the loop counter */
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
