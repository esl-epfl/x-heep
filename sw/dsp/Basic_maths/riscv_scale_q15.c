/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_scale_q15.c    
*    
* Description:	Multiplies a Q15 vector by a scalar.    
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
 * @addtogroup scale    
 * @{    
 */

/**    
 * @brief Multiplies a Q15 vector by a scalar.    
 * @param[in]       *pSrc points to the input vector    
 * @param[in]       scaleFract fractional portion of the scale value    
 * @param[in]       shift number of bits to shift the result by    
 * @param[out]      *pDst points to the output vector    
 * @param[in]       blockSize number of samples in the vector    
 * @return none.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.15 format.    
 * These are multiplied to yield a 2.30 intermediate result and this is shifted with saturation to 1.15 format.    
 */


void riscv_scale_q15(
  q15_t * pSrc,
  q15_t scaleFract,
  int8_t shift,
  q15_t * pDst,
  uint32_t blockSize)
{
  int kShift = 15 - shift;                    /* shift to apply after scaling */
  uint32_t blkCnt;                               /* loop counter */

#if defined (USE_DSP_RISCV)

  q31_t mul1;                  

  blkCnt = blockSize;
  while (blkCnt > 0u)
  {
    /*multiply by scale then shift and saturate*/
    q31_t x= (*pSrc++ * scaleFract) >> kShift;
    int precision=15;
    *pDst++ = (q15_t) ((x)<(-(1<<(precision)))?(-(1<<(precision))):(((x)>((1<<(precision))-1))?((1<<(precision))-1):(x)));
    //*pDst++ =  (q15_t)x_heep_clip((((q31_t) (*pSrc++) *scaleFract ) >> kShift),15);
    blkCnt--;
  }

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A * scale */
    /* Scale the input and then store the result in the destination buffer. */
    *pDst++ = (q15_t) (__SSAT(((q31_t) * pSrc++ * scaleFract) >> kShift, 16));

    /* Decrement the loop counter */
    blkCnt--;
  }
#endif
}

/**    
 * @} end of scale group    
 */