/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5  
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_rms_q15.c    
*    
* Description:	Root Mean Square of the elements of a Q15 vector.  
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
* ---------------------------------------------------------------------------- */

#include "riscv_math.h"
#include "x_heep_emul.h"

/**    
 * @addtogroup RMS    
 * @{    
 */

/**    
 * @brief Root Mean Square of the elements of a Q15 vector.    
 * @param[in]       *pSrc points to the input vector    
 * @param[in]       blockSize length of the input vector    
 * @param[out]      *pResult rms value returned here    
 * @return none.    
 *    
 * @details    
 * <b>Scaling and Overflow Behavior:</b>    
 *    
 * \par    
 * The function is implemented using a 64-bit internal accumulator.    
 * The input is represented in 1.15 format.    
 * Intermediate multiplication yields a 2.30 format, and this    
 * result is added without saturation to a 64-bit accumulator in 34.30 format.    
 * With 33 guard bits in the accumulator, there is no risk of overflow, and the    
 * full precision of the intermediate multiplication is preserved.    
 * Finally, the 34.30 result is truncated to 34.15 format by discarding the lower     
 * 15 bits, and then saturated to yield a result in 1.15 format.    
 *    
 */

void riscv_rms_q15(
  q15_t * pSrc,
  uint32_t blockSize,
  q15_t * pResult)
{
  q63_t sum = 0;                                 /* accumulator */

  q15_t in;                                      /* temporary variable to store the input value */
  uint32_t blkCnt;                               /* loop counter */
#if defined (USE_DSP_RISCV)
  blkCnt = blockSize>>1;
  shortV *VectInA;

  while(blkCnt > 0u)
  {
    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
    VectInA = (shortV*)pSrc;
    sum += x_heep_dotp2(*VectInA, *VectInA);
    pSrc+=2;
    /* Decrement the loop counter */
    blkCnt--;
  }
  blkCnt = blockSize%0x02;
  while(blkCnt > 0u)
  {
    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
    sum += *pSrc++;

    /* Decrement the loop counter */
    blkCnt--;
  }
  riscv_sqrt_q15(x_heep_clip((sum / (q63_t)blockSize) >> 15,16), pResult);
#else

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute sum of the squares and then store the results in a temporary variable, sum */
    in = *pSrc++;
    sum += ((q31_t) in * in);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Truncating and saturating the accumulator to 1.15 format */
  /* Store the result in the destination */
  riscv_sqrt_q15(__SSAT((sum / (q63_t)blockSize) >> 15, 16), pResult);
#endif
}

/**    
 * @} end of RMS group    
 */
