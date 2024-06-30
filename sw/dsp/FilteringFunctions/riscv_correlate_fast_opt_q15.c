/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_correlate_fast_opt_q15.c    
*    
* Description:	Fast Q15 Correlation.    
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
 * @addtogroup Corr    
 * @{    
 */

/**    
 * @brief Correlation of Q15 sequences (fast version) for Cortex-M3 and Cortex-M4.    
 * @param[in] *pSrcA points to the first input sequence.    
 * @param[in] srcALen length of the first input sequence.    
 * @param[in] *pSrcB points to the second input sequence.    
 * @param[in] srcBLen length of the second input sequence.    
 * @param[out] *pDst points to the location where the output result is written.  Length 2 * max(srcALen, srcBLen) - 1.    
 * @param[in]  *pScratch points to scratch buffer of size max(srcALen, srcBLen) + 2*min(srcALen, srcBLen) - 2.   
 * @return none.    
 *    
 *    
 * \par Restrictions    
 *  If the silicon does not support unaligned memory access enable the macro UNALIGNED_SUPPORT_DISABLE    
 *	In this case input, output, scratch buffers should be aligned by 32-bit    
 *    
 *     
 * <b>Scaling and Overflow Behavior:</b>    
 *    
 * \par    
 * This fast version uses a 32-bit accumulator with 2.30 format.    
 * The accumulator maintains full precision of the intermediate multiplication results but provides only a single guard bit.    
 * There is no saturation on intermediate additions.    
 * Thus, if the accumulator overflows it wraps around and distorts the result.    
 * The input signals should be scaled down to avoid intermediate overflows.    
 * Scale down one of the inputs by 1/min(srcALen, srcBLen) to avoid overflow since a    
 * maximum of min(srcALen, srcBLen) number of additions is carried internally.    
 * The 2.30 accumulator is right shifted by 15 bits and then saturated to 1.15 format to yield the final result.    
 *    
 * \par    
 * See <code>riscv_correlate_q15()</code> for a slower implementation of this function which uses a 64-bit accumulator to avoid wrap around distortion.    
 */

void riscv_correlate_fast_opt_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  q15_t * pScratch)
{
  q15_t *pIn1;                                   /* inputA pointer               */
  q15_t *pIn2;                                   /* inputB pointer               */
  q31_t acc0, acc1, acc2, acc3;                  /* Accumulators                  */
  q15_t *py;                                     /* Intermediate inputB pointer  */
  q31_t x1, x2, x3;                              /* temporary variables for holding input and coefficient values */
  uint32_t j, blkCnt, outBlockSize;              /* loop counter                 */
  int32_t inc = 1;                               /* Destination address modifier */
  uint32_t tapCnt;
  q31_t y1, y2;
  q15_t *pScr;                                   /* Intermediate pointers        */
  q15_t *pOut = pDst;                            /* output pointer               */

  shortV *VectInA;
  shortV *VectInB;
  shortV *VectInC;
  shortV VectInD;
  shortV *VectInC1;
  q15_t a, b;


  /* The algorithm implementation is based on the lengths of the inputs. */
  /* srcB is always made to slide across srcA. */
  /* So srcBLen is always considered as shorter or equal to srcALen */
  /* But CORR(x, y) is reverse of CORR(y, x) */
  /* So, when srcBLen > srcALen, output pointer is made to point to the end of the output buffer */
  /* and the destination pointer modifier, inc is set to -1 */
  /* If srcALen > srcBLen, zero pad has to be done to srcB to make the two inputs of same length */
  /* But to improve the performance,        
   * we include zeroes in the output instead of zero padding either of the the inputs*/
  /* If srcALen > srcBLen,        
   * (srcALen - srcBLen) zeroes has to included in the starting of the output buffer */
  /* If srcALen < srcBLen,        
   * (srcALen - srcBLen) zeroes has to included in the ending of the output buffer */
  if(srcALen >= srcBLen)
  {
    /* Initialization of inputA pointer */
    pIn1 = (pSrcA);

    /* Initialization of inputB pointer */
    pIn2 = (pSrcB);

    /* Number of output samples is calculated */
    outBlockSize = (2u * srcALen) - 1u;

    /* When srcALen > srcBLen, zero padding is done to srcB        
     * to make their lengths equal.        
     * Instead, (outBlockSize - (srcALen + srcBLen - 1))        
     * number of output samples are made zero */
    j = outBlockSize - (srcALen + (srcBLen - 1u));

    /* Updating the pointer position to non zero value */
    pOut += j;

  }
  else
  {
    /* Initialization of inputA pointer */
    pIn1 = (pSrcB);

    /* Initialization of inputB pointer */
    pIn2 = (pSrcA);

    /* srcBLen is always considered as shorter or equal to srcALen */
    j = srcBLen;
    srcBLen = srcALen;
    srcALen = j;

    /* CORR(x, y) = Reverse order(CORR(y, x)) */
    /* Hence set the destination pointer to point to the last output sample */
    pOut = pDst + ((srcALen + srcBLen) - 2u);

    /* Destination address modifier is set to -1 */
    inc = -1;

  }

  pScr = pScratch;

  /* Fill (srcBLen - 1u) zeros in scratch buffer */
  riscv_fill_q15(0, pScr, (srcBLen - 1u));

  /* Update temporary scratch pointer */
  pScr += (srcBLen - 1u);



  /* Copy (srcALen) samples in scratch buffer */
  riscv_copy_q15(pIn1, pScr, srcALen);

  /* Update pointers */
  pScr += srcALen;




  /* Fill (srcBLen - 1u) zeros at end of scratch buffer */
  riscv_fill_q15(0, pScr, (srcBLen - 1u));

  /* Update pointer */
  pScr += (srcBLen - 1u);


  /* Temporary pointer for scratch2 */
  py = pIn2;


  /* Actual correlation process starts here */
  blkCnt = (srcALen + srcBLen - 1u) >> 2;

  while(blkCnt > 0)
  {
    /* Initialze temporary scratch pointer as scratch1 */
    pScr = pScratch;

    /* Clear Accumlators */
    acc0 = 0;
    acc1 = 0;
    acc2 = 0;
    acc3 = 0;

    VectInA= (shortV*)pScr;
    pScr+=2;
    VectInB= (shortV*)pScr;
    pScr+=2;


    tapCnt = (srcBLen) >> 2u;

    while(tapCnt > 0u)
    {
	/* Read four samples from smaller buffer */

        VectInC= (shortV*)pIn2;
        VectInC1= (shortV*)(pIn2 + 2 );		
        acc0 = x_heep_sumdotp2(*VectInA, *VectInC,acc0);
        acc2 = x_heep_sumdotp2(*VectInB, *VectInC,acc2);	
        VectInD[0] = (*VectInA)[1]; 
        VectInD[1] = (*VectInB)[0];
	      acc1 = x_heep_sumdotp2(VectInD, *VectInC,acc1);
        VectInA= (shortV*)pScr;
        acc0 = x_heep_sumdotp2(*VectInB, *VectInC1,acc0);
        acc2 = x_heep_sumdotp2(*VectInA, *VectInC1,acc2);
        VectInD[0] = (*VectInB)[1]; 
        VectInD[1] = (*VectInA)[0];
        acc3 = x_heep_sumdotp2(VectInD, *VectInC, acc3);
        acc1 = x_heep_sumdotp2(VectInD, *VectInC1, acc1);
        VectInB= (shortV*)(pScr+2);
        VectInD[0] = (*VectInA)[1]; 
        VectInD[1] = (*VectInB)[0];
        acc3 = x_heep_sumdotp2(VectInD, *VectInC1, acc3);
	pIn2 += 4u;

	pScr += 4u;


	/* Decrement the loop counter */
	tapCnt--;
    }



    /* Update scratch pointer for remaining samples of smaller length sequence */
    pScr -= 4u;


    /* apply same above for remaining samples of smaller length sequence */
    tapCnt = (srcBLen) & 3u;

    while(tapCnt > 0u)
    {

      /* accumlate the results */
      acc0 = x_heep_macu(acc0, *pScr++ , *pIn2);
      acc1 = x_heep_macu(acc1,*pScr++ , *pIn2);
      acc2 = x_heep_macu(acc2, *pScr++ , *pIn2);
      acc3 = x_heep_macu(acc3, *pScr++ , *pIn2++);
      pScr -= 3u;

      /* Decrement the loop counter */
      tapCnt--;
    }

    blkCnt--;


    /* Store the results in the accumulators in the destination buffer. */
    *pOut = (q15_t) (x_heep_clip((acc0 >> 15),15));
    pOut += inc;
    *pOut = (q15_t) (x_heep_clip((acc1 >> 15), 15));
    pOut += inc;
    *pOut = (q15_t) (x_heep_clip((acc2 >> 15), 15));
    pOut += inc;
    *pOut = (q15_t) (x_heep_clip((acc3 >> 15), 15));
    pOut += inc;

    /* Initialization of inputB pointer */
    pIn2 = py;

    pScratch += 4u;

  }


  blkCnt = (srcALen + srcBLen - 1u) & 0x3;

  /* Calculate correlation for remaining samples of Bigger length sequence */
  while(blkCnt > 0)
  {
    /* Initialze temporary scratch pointer as scratch1 */
    pScr = pScratch;

    /* Clear Accumlators */
    acc0 = 0;

    tapCnt = (srcBLen) >> 1u;

    while(tapCnt > 0u)
    {

      acc0 = x_heep_macu(acc0,*pScr++ , *pIn2++ );
      acc0 = x_heep_macu(acc0,*pScr++ , *pIn2++);

      /* Decrement the loop counter */
      tapCnt--;
    }

    tapCnt = (srcBLen) & 1u;

    /* apply same above for remaining samples of smaller length sequence */
    while(tapCnt > 0u)
    {

      /* accumlate the results */
      acc0 = x_heep_macu(acc0,*pScr++ , *pIn2++);
      /* Decrement the loop counter */
      tapCnt--;
    }

    blkCnt--;

    /* Store the result in the accumulator in the destination buffer. */
    *pOut = (q15_t) (x_heep_clip((acc0 >> 15), 15));
    pOut += inc;

    /* Initialization of inputB pointer */
    pIn2 = py;

    pScratch += 1u;

  }
}

/**    
 * @} end of Corr group    
 */
