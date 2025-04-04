/* ----------------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5  
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_q31_to_q7.c    
*    
* Description:	Converts the elements of the Q31 vector to Q7 vector.    
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
 * @brief Converts the elements of the Q31 vector to Q7 vector.    
 * @param[in]       *pSrc points to the Q31 input vector    
 * @param[out]      *pDst points to the Q7 output vector   
 * @param[in]       blockSize length of the input vector    
 * @return none.    
 *    
 * \par Description:    
 *    
 * The equation used for the conversion process is:    
 *   
 * <pre>    
 * 	pDst[n] = (q7_t) pSrc[n] >> 24;   0 <= n < blockSize.     
 * </pre>    
 *   
 */


void riscv_q31_to_q7(
  q31_t * pSrc,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q31_t *pIn = pSrc;
#if defined (USE_DSP_RISCV)
 blkCnt = blockSize >> 2u;
 q7_t out1, out2, out3, out4;
 charV VectInA;
  while(blkCnt > 0u)
  {
    /* C = (q7_t) A >> 24 */
    /* convert from q31 to q7 and then store the results in the destination buffer */
    out1 = (q7_t) (*pIn++ >> 24);
    out2 = (q7_t) (*pIn++ >> 24);
    out3 = (q7_t) (*pIn++ >> 24);
    out4 = (q7_t) (*pIn++ >> 24);
    /* Decrement the loop counter */
    VectInA = x_heep_pack4(out1,out2,out3,out4);
    *(charV*)pDst  = VectInA;
    pDst+=4;
    blkCnt--;
  }

 blkCnt = blockSize % 0x04;
#else
  /* Loop over blockSize number of values */
  blkCnt = blockSize;
#endif
  while(blkCnt > 0u)
  {
    /* C = (q7_t) A >> 24 */
    /* convert from q31 to q7 and then store the results in the destination buffer */
    *pDst++ = (q7_t) (*pIn++ >> 24);

    /* Decrement the loop counter */
    blkCnt--;
  }

}

/**    
 * @} end of q31_to_x group    
 */
