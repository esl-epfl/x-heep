/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_mat_sub_q15.c    
*    
* Description:	Q15 Matrix subtraction    
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
#include "x_heep_emul.h"

/**    
 * @ingroup groupMatrix    
 */

/**    
 * @addtogroup MatrixSub    
 * @{    
 */

/**    
 * @brief Q15 matrix subtraction.    
 * @param[in]       *pSrcA points to the first input matrix structure    
 * @param[in]       *pSrcB points to the second input matrix structure    
 * @param[out]      *pDst points to output matrix structure    
 * @return     		The function returns either    
 * <code>RISCV_MATH_SIZE_MISMATCH</code> or <code>RISCV_MATH_SUCCESS</code> based on the outcome of size checking.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function uses saturating arithmetic.    
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.    
 */

riscv_status riscv_mat_sub_q15(
  const riscv_matrix_instance_q15 * pSrcA,
  const riscv_matrix_instance_q15 * pSrcB,
  riscv_matrix_instance_q15 * pDst)
{
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B */
  q15_t *pOut = pDst->pData;                     /* output data matrix pointer */
  uint32_t numSamples;                           /* total number of elements in the matrix */
  uint32_t blkCnt;                               /* loop counters  */
  riscv_status status;                             /* status of matrix subtraction  */


#ifdef RISCV_MATH_MATRIX_CHECK


  /* Check for matrix mismatch condition */
  if((pSrcA->numRows != pSrcB->numRows) ||
     (pSrcA->numCols != pSrcB->numCols) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcA->numCols != pDst->numCols))
  {
    /* Set status as RISCV_MATH_SIZE_MISMATCH */
    status = RISCV_MATH_SIZE_MISMATCH;
  }
  else
#endif /*    #ifdef RISCV_MATH_MATRIX_CHECK    */

  {
    /* Total number of samples in the input matrix */
    numSamples = (uint32_t) pSrcA->numRows * pSrcA->numCols;

    /* Initialize blkCnt with number of samples */
    blkCnt = numSamples;

    while(blkCnt > 0u)
    {
      /* C(m,n) = A(m,n) - B(m,n) */
      /* Subtract and then store the results in the destination buffer. */
#if defined (USE_DSP_RISCV)
        int precision=15;
      float32_t x=(*pInA++ - *pInB++);
      q15_t a=(q15_t)((x)<(-(1<<(precision)))?(-(1<<(precision))):(((x)>((1<<(precision))-1))?((1<<(precision))-1):(x)));
      *pOut++ = a;
#else
      *pOut++ = (q15_t) __SSAT(((q31_t) * pInA++ - *pInB++), 16);
#endif
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
 * @} end of MatrixSub group    
 */
