/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_conv_partial_opt_q15.c    
*    
* Description:	Partial convolution of Q15 sequences.   
*    
* Target Processor: Cortex-M4/Cortex-M3
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
 * @ingroup groupFilters    
 */

/**    
 * @addtogroup PartialConv    
 * @{    
 */

/**    
 * @brief Partial convolution of Q15 sequences.    
 * @param[in]       *pSrcA points to the first input sequence.    
 * @param[in]       srcALen length of the first input sequence.    
 * @param[in]       *pSrcB points to the second input sequence.    
 * @param[in]       srcBLen length of the second input sequence.    
 * @param[out]      *pDst points to the location where the output result is written.    
 * @param[in]       firstIndex is the first output sample to start with.    
 * @param[in]       numPoints is the number of output points to be computed.    
 * @param[in]       *pScratch1 points to scratch buffer of size max(srcALen, srcBLen) + 2*min(srcALen, srcBLen) - 2.   
 * @param[in]       *pScratch2 points to scratch buffer of size min(srcALen, srcBLen).   
 * @return  Returns either RISCV_MATH_SUCCESS if the function completed correctly or RISCV_MATH_ARGUMENT_ERROR if the requested subset is not in the range [0 srcALen+srcBLen-2].    
 *    
 * \par Restrictions    
 *  If the silicon does not support unaligned memory access enable the macro UNALIGNED_SUPPORT_DISABLE    
 *	In this case input, output, state buffers should be aligned by 32-bit    
 *    
 * Refer to <code>riscv_conv_partial_fast_q15()</code> for a faster but less precise version of this function for Cortex-M3 and Cortex-M4.   
 *  
 * 
 */


riscv_status riscv_conv_partial_opt_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints,
  q15_t * pScratch1,
  q15_t * pScratch2)
{

  q15_t *pOut = pDst;                            /* output pointer */
  q15_t *pScr1 = pScratch1;                      /* Temporary pointer for scratch1 */
  q15_t *pScr2 = pScratch2;                      /* Temporary pointer for scratch1 */
  q63_t acc0, acc1, acc2, acc3;                  /* Accumulator */
  q31_t x1, x2, x3;                              /* Temporary variables to hold state and coefficient values */
  q31_t y1, y2;                                  /* State variables */
  q15_t *pIn1;                                   /* inputA pointer */
  q15_t *pIn2;                                   /* inputB pointer */
  q15_t *px;                                     /* Intermediate inputA pointer  */
  q15_t *py;                                     /* Intermediate inputB pointer  */
  uint32_t j, k, blkCnt;                         /* loop counter */
  riscv_status status;                             /* Status variable */
  uint32_t tapCnt;                               /* loop count */
  shortV *VectInA;
  shortV *VectInB;
  shortV *VectInC;
  shortV VectInD;
  shortV *VectInC1;
  q15_t a, b;

  /* Check for range of output samples to be calculated */
  if((firstIndex + numPoints) > ((srcALen + (srcBLen - 1u))))
  {
    /* Set status as RISCV_MATH_ARGUMENT_ERROR */
    status = RISCV_MATH_ARGUMENT_ERROR;
  }
  else
  {

    /* The algorithm implementation is based on the lengths of the inputs. */
    /* srcB is always made to slide across srcA. */
    /* So srcBLen is always considered as shorter or equal to srcALen */
    if(srcALen >= srcBLen)
    {
      /* Initialization of inputA pointer */
      pIn1 = pSrcA;

      /* Initialization of inputB pointer */
      pIn2 = pSrcB;
    }
    else
    {
      /* Initialization of inputA pointer */
      pIn1 = pSrcB;

      /* Initialization of inputB pointer */
      pIn2 = pSrcA;

      /* srcBLen is always considered as shorter or equal to srcALen */
      j = srcBLen;
      srcBLen = srcALen;
      srcALen = j;
    }

    /* Temporary pointer for scratch2 */
    py = pScratch2;

    /* pointer to take end of scratch2 buffer */
    pScr2 = pScratch2 + srcBLen - 1;

    /* points to smaller length sequence */
    px = pIn2;

    /* Apply loop unrolling and do 4 Copies simultaneously. */
    k = srcBLen >> 2u;

    /* First part of the processing with loop unrolling copies 4 data points at a time.       
     ** a second loop below copies for the remaining 1 to 3 samples. */
    while(k > 0u)
    {
      /* copy second buffer in reversal manner */
      *pScr2-- = *px++;
      *pScr2-- = *px++;
      *pScr2-- = *px++;
      *pScr2-- = *px++;

      /* Decrement the loop counter */
      k--;
    }

    /* If the count is not a multiple of 4, copy remaining samples here.       
     ** No loop unrolling is used. */
    k = srcBLen % 0x4u;

    while(k > 0u)
    {
      /* copy second buffer in reversal manner for remaining samples */
      *pScr2-- = *px++;

      /* Decrement the loop counter */
      k--;
    }

    /* Initialze temporary scratch pointer */
    pScr1 = pScratch1;

    /* Fill (srcBLen - 1u) zeros in scratch buffer */
    riscv_fill_q15(0, pScr1, (srcBLen - 1u));

    /* Update temporary scratch pointer */
    pScr1 += (srcBLen - 1u);

    /* Copy bigger length sequence(srcALen) samples in scratch1 buffer */

    /* Copy (srcALen) samples in scratch buffer */
    riscv_copy_q15(pIn1, pScr1, srcALen);

    /* Update pointers */
    pScr1 += srcALen;

    /* Fill (srcBLen - 1u) zeros at end of scratch buffer */
    riscv_fill_q15(0, pScr1, (srcBLen - 1u));

    /* Update pointer */
    pScr1 += (srcBLen - 1u);

    /* Initialization of pIn2 pointer */
    pIn2 = py;

    pScratch1 += firstIndex;

    pOut = pDst + firstIndex;

    /* Actual convolution process starts here */
    blkCnt = (numPoints) >> 2;

    while(blkCnt > 0)
    {
      /* Initialze temporary scratch pointer as scratch1 */
      pScr1 = pScratch1;

      /* Clear Accumlators */
      acc0 = 0;
      acc1 = 0;
      acc2 = 0;
      acc3 = 0;

      VectInA= (shortV*)pScr1;
      pScr1+=2;
      VectInB= (shortV*)pScr1;
      pScr1+=2;
      tapCnt = (srcBLen) >> 2u;

      while(tapCnt > 0u)
      {

        /* Read four samples from smaller buffer */

        VectInC= (shortV*)pIn2;
        VectInC1= (shortV*)(pIn2 + 2 );      

	/* multiply and accumlate */
        acc0 += x_heep_dotp2(*VectInA, *VectInC);
        acc2 += x_heep_dotp2(*VectInB, *VectInC);

        VectInD[0] = (*VectInA)[1]; 
        VectInD[1] = (*VectInB)[0];



        acc1 += x_heep_dotp2(VectInD, *VectInC);
        VectInA= (shortV*)pScr1; 
        /* multiply and accumlate */
        acc0 += x_heep_dotp2(*VectInB, *VectInC1);
        acc2 += x_heep_dotp2(*VectInA, *VectInC1);
        VectInD[0] = (*VectInB)[1]; 
        VectInD[1] = (*VectInA)[0];
        acc3 += x_heep_dotp2(VectInD, *VectInC);
        acc1 += x_heep_dotp2(VectInD, *VectInC1);
        VectInB= (shortV*)(pScr1+2);
        VectInD[0] = (*VectInA)[1]; 
        VectInD[1] = (*VectInB)[0];
        acc3 += x_heep_dotp2(VectInD, *VectInC1);

        pIn2 += 4u;
        pScr1 += 4u;


        /* Decrement the loop counter */
        tapCnt--;
      }

      /* Update scratch pointer for remaining samples of smaller length sequence */
      pScr1 -= 4u;

      /* apply same above for remaining samples of smaller length sequence */
      tapCnt = (srcBLen) & 3u;

      while(tapCnt > 0u)
      {
        /* accumlate the results */
        acc0 += (*pScr1++ * *pIn2);
        acc1 += (*pScr1++ * *pIn2);
        acc2 += (*pScr1++ * *pIn2);
        acc3 += (*pScr1++ * *pIn2++);

        pScr1 -= 3u;

        /* Decrement the loop counter */
        tapCnt--;
      }

      blkCnt--;


      /* Store the results in the accumulators in the destination buffer. */
      *(shortV*)pOut = x_heep_pack2(__SSAT((acc0 >> 15), 16),__SSAT((acc1 >> 15), 16));
      pOut+=2;
      *(shortV*)pOut = x_heep_pack2(__SSAT((acc2 >> 15), 16),__SSAT((acc3 >> 15), 16));
      pOut+=2;
      /* Initialization of inputB pointer */
      pIn2 = py;

      pScratch1 += 4u;

    }


    blkCnt = numPoints & 0x3;

    /* Calculate convolution for remaining samples of Bigger length sequence */
    while(blkCnt > 0)
    {
      /* Initialze temporary scratch pointer as scratch1 */
      pScr1 = pScratch1;

      /* Clear Accumlators */
      acc0 = 0;

      tapCnt = (srcBLen) >> 1u;

      while(tapCnt > 0u)
      {

        /* Read next two samples from scratch1 buffer */
        acc0 += (*pScr1++ * *pIn2++);
        acc0 += (*pScr1++ * *pIn2++);

        /* Decrement the loop counter */
        tapCnt--;
      }

      tapCnt = (srcBLen) & 1u;

      /* apply same above for remaining samples of smaller length sequence */
      while(tapCnt > 0u)
      {

        /* accumlate the results */
        acc0 += (*pScr1++ * *pIn2++);

        /* Decrement the loop counter */
        tapCnt--;
      }

      blkCnt--;

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q15_t) (__SSAT((acc0 >> 15), 16));

      /* Initialization of inputB pointer */
      pIn2 = py;

      pScratch1 += 1u;

    }

    /* set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;

  }

  /* Return to application */
  return (status);
}


/**    
 * @} end of PartialConv group    
 */
