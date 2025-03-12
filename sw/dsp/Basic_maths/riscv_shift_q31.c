/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_shift_q31.c    
*    
* Description:	Shifts the elements of a Q31 vector by a specified number of bits.    
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
 * @ingroup groupMath        
 */
/**        
 * @defgroup shift Vector Shift        
 *        
 * Shifts the elements of a fixed-point vector by a specified number of bits.        
 * There are separate functions for Q7, Q15, and Q31 data types.        
 * The underlying algorithm used is:        
 *        
 * <pre>        
 *     pDst[n] = pSrc[n] << shift,   0 <= n < blockSize.        
 * </pre>        
 *        
 * If <code>shift</code> is positive then the elements of the vector are shifted to the left.        
 * If <code>shift</code> is negative then the elements of the vector are shifted to the right.        
 *
 * The functions support in-place computation allowing the source and destination
 * pointers to reference the same memory buffer.
 */

/**        
 * @addtogroup shift        
 * @{        
 */

/**        
 * @brief  Shifts the elements of a Q31 vector a specified number of bits.        
 * @param[in]  *pSrc points to the input vector        
 * @param[in]  shiftBits number of bits to shift.  A positive value shifts left; a negative value shifts right.        
 * @param[out]  *pDst points to the output vector        
 * @param[in]  blockSize number of samples in the vector        
 * @return none.        
 *        
 *        
 * <b>Scaling and Overflow Behavior:</b>        
 * \par        
 * The function uses saturating arithmetic.        
 * Results outside of the allowable Q31 range [0x80000000 0x7FFFFFFF] will be saturated.        
 */

void riscv_shift_q31(
  q31_t * pSrc,
  int8_t shiftBits,
  q31_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign = (shiftBits & 0x80);             /* Sign of shiftBits */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;
  while(blkCnt > 0u)
  {
    /* C = A (>> or <<) shiftBits */
    /* Shift the input and then store the result in the destination buffer. */
    *pDst++ = (sign == 0u) ? clip_q63_to_q31((q63_t) * pSrc++ << shiftBits) :
      (*pSrc++ >> -shiftBits);

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**        
 * @} end of shift group        
 */