/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_scale_q31.c    
*    
* Description:	Multiplies a Q31 vector by a scalar.    
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
 * @brief Multiplies a Q31 vector by a scalar.       
 * @param[in]       *pSrc points to the input vector       
 * @param[in]       scaleFract fractional portion of the scale value       
 * @param[in]       shift number of bits to shift the result by       
 * @param[out]      *pDst points to the output vector       
 * @param[in]       blockSize number of samples in the vector       
 * @return none.       
 *       
 * <b>Scaling and Overflow Behavior:</b>       
 * \par       
 * The input data <code>*pSrc</code> and <code>scaleFract</code> are in 1.31 format.       
 * These are multiplied to yield a 2.62 intermediate result and this is shifted with saturation to 1.31 format.       
 */

void riscv_scale_q31(
  q31_t * pSrc,
  q31_t scaleFract,
  int8_t shift,
  q31_t * pDst,
  uint32_t blockSize)
{
  int8_t kShift = shift + 1;                     /* Shift to apply after scaling */
  int8_t sign = (kShift & 0x80);
  uint32_t blkCnt;                               /* loop counter */
  q31_t in, out;



  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;


  if(sign == 0)
  {
	  while(blkCnt > 0u)
	  {
		/* C = A * scale */
		/* Scale the input and then store the result in the destination buffer. */
		in = *pSrc++;
		in = ((q63_t) in * scaleFract) >> 32;

		out = in << kShift;
		
		if(in != (out >> kShift))
			out = 0x7FFFFFFF ^ (in >> 31);

		*pDst++ = out;

		/* Decrement the loop counter */
		blkCnt--;
	  }
  }
  else
  {
	  while(blkCnt > 0u)
	  {
		/* C = A * scale */
		/* Scale the input and then store the result in the destination buffer. */
		in = *pSrc++;
		in = ((q63_t) in * scaleFract) >> 32;

		out = in >> -kShift;

		*pDst++ = out;

		/* Decrement the loop counter */
		blkCnt--;
	  }
  
  }
}

/**       
 * @} end of scale group       
 */