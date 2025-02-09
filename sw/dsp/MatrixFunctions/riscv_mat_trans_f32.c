/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_mat_trans_f32.c    
*    
* Description:	Floating-point matrix transpose.    
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

/**    
 * @defgroup MatrixTrans Matrix Transpose    
 *    
 * Tranposes a matrix.    
 * Transposing an <code>M x N</code> matrix flips it around the center diagonal and results in an <code>N x M</code> matrix.    
 * \image html MatrixTranspose.gif "Transpose of a 3 x 3 matrix"    
 */

#include "riscv_math.h"

/**    
 * @ingroup groupMatrix    
 */

/**    
 * @addtogroup MatrixTrans    
 * @{    
 */

/**    
  * @brief Floating-point matrix transpose.    
  * @param[in]  *pSrc points to the input matrix    
  * @param[out] *pDst points to the output matrix    
  * @return 	The function returns either  <code>RISCV_MATH_SIZE_MISMATCH</code>    
  * or <code>RISCV_MATH_SUCCESS</code> based on the outcome of size checking.    
  */


riscv_status riscv_mat_trans_f32(
  const riscv_matrix_instance_f32 * pSrc,
  riscv_matrix_instance_f32 * pDst)
{
  float32_t *pIn = pSrc->pData;                  /* input data matrix pointer */
  float32_t *pOut = pDst->pData;                 /* output data matrix pointer */
  float32_t *px;                                 /* Temporary output data matrix pointer */
  uint16_t nRows = pSrc->numRows;                /* number of rows */
  uint16_t nColumns = pSrc->numCols;             /* number of columns */

  uint16_t col, i = 0u, row = nRows;             /* loop counters */
  riscv_status status;                             /* status of matrix transpose  */


#ifdef RISCV_MATH_MATRIX_CHECK

  /* Check for matrix mismatch condition */
  if((pSrc->numRows != pDst->numCols) || (pSrc->numCols != pDst->numRows))
  {
    /* Set status as RISCV_MATH_SIZE_MISMATCH */
    status = RISCV_MATH_SIZE_MISMATCH;
  }
  else
#endif /*      #ifdef RISCV_MATH_MATRIX_CHECK    */

  {
    /* Matrix transpose by exchanging the rows with columns */
    /* row loop     */
    do
    {
      /* The pointer px is set to starting address of the column being processed */
      px = pOut + i;

      /* Initialize column loop counter */
      col = nColumns;

      while(col > 0u)
      {
        /* Read and store the input element in the destination */
        *px = *pIn++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += nRows;

        /* Decrement the column loop counter */
        col--;
      }
      i++;
      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);          /* row loop end  */

    /* Set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**    
 * @} end of MatrixTrans group    
 */
