/* ----------------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5  
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_q31_to_q15.c    
*    
* Description:	Converts the elements of the Q31 vector to Q15 vector.    
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
* ---------------------------------------------------------------------------- */

#include "riscv_math.h"
#include "x_heep_emul.h"
/**    
 * @ingroup groupSupport    
 */

/**    
 * @addtogroup q31_to_x    
 * @{    
 */

/**    
 * @brief Converts the elements of the Q31 vector to Q15 vector.    
 * @param[in]       *pSrc points to the Q31 input vector    
 * @param[out]      *pDst points to the Q15 output vector   
 * @param[in]       blockSize length of the input vector    
 * @return none.    
 *     
 * \par Description:    
 *    
 * The equation used for the conversion process is:    
 *   
 * <pre>    
 * 	pDst[n] = (q15_t) pSrc[n] >> 16;   0 <= n < blockSize.    
 * </pre>    
 *   
 */


void riscv_q31_to_q15(
  q31_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q31_t *pIn = pSrc;                             /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */
#if defined (USE_DSP_RISCV)
 blkCnt = blockSize >> 1u;
 q15_t out1, out2;
 shortV VectInA;
  while(blkCnt > 0u)
  {
    /* C = (q7_t) A >> 24 */
    /* convert from q31 to q7 and then store the results in the destination buffer */
    out1 = (q15_t) (*pIn++ >> 16);
    out2 = (q15_t) (*pIn++ >> 16);

    VectInA = x_heep_pack2(out1,out2);
    *(shortV*)pDst  = VectInA;
    pDst+=2;
    blkCnt--;
  }

 blkCnt = blockSize % 0x02;
#else

  /* Loop over blockSize number of values */
  blkCnt = blockSize;
#endif
  while(blkCnt > 0u)
  {
    /* C = (q15_t) A >> 16 */
    /* convert from q31 to q15 and then store the results in the destination buffer */
    *pDst++ = (q15_t) (*pIn++ >> 16);

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**    
 * @} end of q31_to_x group    
 */
