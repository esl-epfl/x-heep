/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_shift_q15.c    
*    
* Description:	Shifts the elements of a Q15 vector by a specified number of bits.    
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
 * @addtogroup shift    
 * @{    
 */

/**    
 * @brief  Shifts the elements of a Q15 vector a specified number of bits.    
 * @param[in]  *pSrc points to the input vector    
 * @param[in]  shiftBits number of bits to shift.  A positive value shifts left; a negative value shifts right.    
 * @param[out]  *pDst points to the output vector    
 * @param[in]  blockSize number of samples in the vector    
 * @return none.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function uses saturating arithmetic.    
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] will be saturated.    
 */


void riscv_shift_q15(
  q15_t * pSrc,
  int8_t shiftBits,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign;                                  /* Sign of shiftBits */


  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

#if defined (USE_DSP_RISCV)
  shortV *VectInA;
  shortV VectInC,VectInB; 
  blkCnt = blockSize >> 1u;
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      int precision=0xF;
      
      q31_t x=((q31_t) * pSrc++ << shiftBits);
      *pDst++ = (q31_t)x_heep_clip(x,precision);
      x=((q31_t) * pSrc++ << shiftBits);
      *pDst++ = (q31_t)x_heep_clip(x,precision);
      
      //*pDst++ = (q31_t)x_heep_clip((q31_t)((q31_t)* pSrc++ )<< shiftBits,(int)0xF);
      //*pDst++ = (q31_t)x_heep_clip((q31_t)((q31_t)* pSrc++ )<< shiftBits,(int)0xF);
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else /*shift right*/
  {

    /* Initialize blkCnt with number of samples */
    VectInB = x_heep_pack2(-shiftBits,-shiftBits);
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      VectInA = (shortV*)pSrc; 
      VectInC = x_heep_sra2(*VectInA,VectInB); 
      //*pDst++ = VectInC[0];
      //*pDst++ = VectInC[1];
      *(shortV*)pDst =  VectInC;
      pDst+=2;
      pSrc+=2;
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  blkCnt = blockSize % 0x2u;

#else
    blkCnt = blockSize;
#endif
  /* If the shift value is positive then do right shift else left shift */
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */


    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift and then store the results in the destination buffer. */
      *pDst++ = __SSAT(((q31_t) * pSrc++ << shiftBits), 16);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the inputs and then store the results in the destination buffer. */
      *pDst++ = (*pSrc++ >> -shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
}

/**    
 * @} end of shift group    
 */


