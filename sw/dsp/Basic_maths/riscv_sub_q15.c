/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_sub_q15.c    
*    
* Description:	Q15 vector subtraction.    
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
 * @ingroup groupMath    
 */

/**    
 * @addtogroup BasicSub    
 * @{    
 */

/**    
 * @brief Q15 vector subtraction.    
 * @param[in]       *pSrcA points to the first input vector    
 * @param[in]       *pSrcB points to the second input vector    
 * @param[out]      *pDst points to the output vector    
 * @param[in]       blockSize number of samples in each vector    
 * @return none.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function uses saturating arithmetic.    
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.    
 */

void riscv_sub_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  #if defined (USE_DSP_RISCV)
  q31_t inA1, inA2, inB1, inB2;

/*loop Unrolling */
  blkCnt = blockSize >> 1u;
  while (blkCnt > 0u)
  {

    /* read 2 elements from each source*/
    inA1 = *pSrcA++;
    inA2 = *pSrcA++;
    inB1 = *pSrcB++;
    inB2 = *pSrcB++;
    /*subract then saturate*/
    *pDst++ =(q15_t)x_heep_clip((inA1 - inB1),15);
    *pDst++ =(q15_t)x_heep_clip((inA2 - inB2),15);

    /* Decrement the loop counter */
    blkCnt--;
  }

  blkCnt = blockSize % 0x2u;

  while (blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ = (q15_t)x_heep_clip((*pSrcA++ - *pSrcB++),15);
    /* Decrement the loop counter */
    blkCnt--;
  }

  #else


  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A - B */
    /* Subtract and then store the result in the destination buffer. */
    *pDst++ = (q15_t) __SSAT(((q31_t) * pSrcA++ - *pSrcB++), 16);

    /* Decrement the loop counter */
    blkCnt--;
  }
  #endif
}


/**    
 * @} end of BasicSub group    
 */