/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_abs_q7.c    
*    
* Description:	Q7 vector absolute value.    
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
 * @addtogroup BasicAbs        
 * @{        
 */

/**        
 * @brief Q7 vector absolute value.        
 * @param[in]       *pSrc points to the input buffer        
 * @param[out]      *pDst points to the output buffer        
 * @param[in]       blockSize number of samples in each vector        
 * @return none.        
 *    
 * \par Conditions for optimum performance    
 *  Input and output buffers should be aligned by 32-bit    
 *    
 *        
 * <b>Scaling and Overflow Behavior:</b>        
 * \par        
 * The function uses saturating arithmetic.        
 */

void riscv_abs_q7(
  q7_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q7_t in;                                       /* Input value1 */

#if defined (USE_DSP_RISCV)


  q31_t in1, in2, in3, in4;                      /* temporary input variables */
  q31_t out1, out2, out3, out4;                  /* temporary output variables */
  charV *VectInA;
  charV VectInC; 
  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  while (blkCnt > 0u)
  {
    VectInA = (charV*)pSrc; 
    VectInC = x_heep_abs4(*VectInA); /*calculate the absolute of 4 q7 at the same time */
    /*check for each one to saturate it*/
    *pDst++ = ( VectInC[0] == -128)?0x7f:VectInC[0];
    *pDst++ = ( VectInC[1] == -128)?0x7f:VectInC[1];
    *pDst++ = ( VectInC[2] == -128)?0x7f:VectInC[2];
    *pDst++ = ( VectInC[3] == -128)?0x7f:VectInC[3];
    
    pSrc+=4; /*inc pointer for next loop*/
    blkCnt--; /*dec loop counter*/
  }

  blkCnt = blockSize % 0x4u;
#else
  blkCnt = blockSize;
#endif
  while(blkCnt > 0u)
  {
    /* C = |A| */
    /* Read the input */
    in = *pSrc++;

    /* Store the Absolute result in the destination buffer */
    *pDst++ = (in > 0) ? in : ((in == (q7_t) 0x80) ? 0x7f : -in);

    /* Decrement the loop counter */
    blkCnt--;
  }
}

/**        
 * @} end of BasicAbs group        
 */
