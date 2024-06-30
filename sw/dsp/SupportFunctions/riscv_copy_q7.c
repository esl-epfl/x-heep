/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5  
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_copy_q7.c    
*    
* Description:	Copies the elements of a Q7 vector.    
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
 * @ingroup groupSupport    
 */

/**    
 * @addtogroup copy    
 * @{    
 */

/**    
 * @brief Copies the elements of a Q7 vector.    
 * @param[in]       *pSrc points to input vector    
 * @param[out]      *pDst points to output vector    
 * @param[in]       blockSize length of the input vector   
 * @return none.    
 *    
 */

void riscv_copy_q7(
  q7_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
#if defined (USE_DSP_RISCV)
  blkCnt = blockSize >>2;

  while(blkCnt > 0u)
  {
    /* C = A */
    /* Copy and then store the results in the destination buffer */
   *(charV*)pDst =  *(charV*)pSrc;
   pDst+=4;
   pSrc+=4;

    /* Decrement the loop counter */
    blkCnt--;
  }
  blkCnt = blockSize % 0x4;
  while(blkCnt > 0u)
  {
    /* C = A */
    /* Copy and then store the results in the destination buffer */
   *pDst++ = *pSrc++;

    /* Decrement the loop counter */
    blkCnt--;
  }
#else
  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A */
    /* Copy and then store the results in the destination buffer */
    *pDst++ = *pSrc++;

    /* Decrement the loop counter */
    blkCnt--;
  }
#endif
}

/**    
 * @} end of BasicCopy group    
 */
