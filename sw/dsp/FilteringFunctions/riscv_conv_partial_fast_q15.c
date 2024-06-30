/* ----------------------------------------------------------------------   
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.   
*   
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*   
* Project: 	    CMSIS DSP Library   
* Title:		arm_conv_partial_fast_q15.c   
*   
* Description:	Fast Q15 Partial convolution.   
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
 * @brief Partial convolution of Q15 sequences (fast version) for Cortex-M3 and Cortex-M4.   
 * @param[in]       *pSrcA points to the first input sequence.   
 * @param[in]       srcALen length of the first input sequence.   
 * @param[in]       *pSrcB points to the second input sequence.   
 * @param[in]       srcBLen length of the second input sequence.   
 * @param[out]      *pDst points to the location where the output result is written.   
 * @param[in]       firstIndex is the first output sample to start with.   
 * @param[in]       numPoints is the number of output points to be computed.   
 * @return Returns either RISCV_MATH_SUCCESS if the function completed correctly or RISCV_MATH_ARGUMENT_ERROR if the requested subset is not in the range [0 srcALen+srcBLen-2].   
 *   
 * See <code>riscv_conv_partial_q15()</code> for a slower implementation of this function which uses a 64-bit accumulator to avoid wrap around distortion.   
 */


riscv_status riscv_conv_partial_fast_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst,
  uint32_t firstIndex,
  uint32_t numPoints)
{

  q15_t *pIn1;                                   /* inputA pointer               */
  q15_t *pIn2;                                   /* inputB pointer               */
  q15_t *pOut = pDst;                            /* output pointer               */
  q31_t sum, acc0, acc1, acc2, acc3;             /* Accumulator                  */
  q15_t *px;                                     /* Intermediate inputA pointer  */
  q15_t *py;                                     /* Intermediate inputB pointer  */
  q15_t *pSrc1, *pSrc2;                          /* Intermediate pointers        */
  q31_t x0, x1, x2, x3, c0;
  uint32_t j, k, count, check, blkCnt;
  int32_t blockSize1, blockSize2, blockSize3;    /* loop counters                 */
  riscv_status status;                             /* status of Partial convolution */
  q15_t a, b;
  shortV VectInA;
  shortV VectInB;
  shortV VectInC;
  shortV VectInD;
  shortV VectInE;
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
    if(srcALen >=srcBLen)
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

    /* Conditions to check which loopCounter holds   
     * the first and last indices of the output samples to be calculated. */
    check = firstIndex + numPoints;
    blockSize3 = ((int32_t)check > (int32_t)srcALen) ? (int32_t)check - (int32_t)srcALen : 0;
    blockSize3 = ((int32_t)firstIndex > (int32_t)srcALen - 1) ? blockSize3 - (int32_t)firstIndex + (int32_t)srcALen : blockSize3;
    blockSize1 = ((int32_t) srcBLen - 1) - (int32_t) firstIndex;
    blockSize1 = (blockSize1 > 0) ? ((check > (srcBLen - 1u)) ? blockSize1 :
                                     (int32_t) numPoints) : 0;
    blockSize2 = ((int32_t) check - blockSize3) -
      (blockSize1 + (int32_t) firstIndex);
    blockSize2 = (blockSize2 > 0) ? blockSize2 : 0;

    /* conv(x,y) at n = x[n] * y[0] + x[n-1] * y[1] + x[n-2] * y[2] + ...+ x[n-N+1] * y[N -1] */
    /* The function is internally   
     * divided into three stages according to the number of multiplications that has to be   
     * taken place between inputA samples and inputB samples. In the first stage of the   
     * algorithm, the multiplications increase by one for every iteration.   
     * In the second stage of the algorithm, srcBLen number of multiplications are done.   
     * In the third stage of the algorithm, the multiplications decrease by one   
     * for every iteration. */

    /* Set the output pointer to point to the firstIndex   
     * of the output sample to be calculated. */
    pOut = pDst + firstIndex;

    /* --------------------------   
     * Initializations of stage1   
     * -------------------------*/

    /* sum = x[0] * y[0]   
     * sum = x[0] * y[1] + x[1] * y[0]   
     * ....   
     * sum = x[0] * y[srcBlen - 1] + x[1] * y[srcBlen - 2] +...+ x[srcBLen - 1] * y[0]   
     */

    /* In this stage the MAC operations are increased by 1 for every iteration.   
       The count variable holds the number of MAC operations performed.   
       Since the partial convolution starts from firstIndex   
       Number of Macs to be performed is firstIndex + 1 */
    count = 1u + firstIndex;

    /* Working pointer of inputA */
    px = pIn1;

    /* Working pointer of inputB */
    pSrc2 = pIn2 + firstIndex;
    py = pSrc2;

    /* ------------------------   
     * Stage1 process   
     * ----------------------*/

    /* For loop unrolling by 4, this stage is divided into two. */
    /* First part of this stage computes the MAC operations less than 4 */
    /* Second part of this stage computes the MAC operations greater than or equal to 4 */

    /* The first part of the stage starts here */
  while((count < 4u) && (blockSize1 > 0))
    {
      /* Accumulator is made zero for every iteration */
      sum = 0;

      /* Loop over number of MAC operations between   
       * inputA samples and inputB samples */
      k = count;

      while(k > 0u)
      {
        /* Perform the multiply-accumulates */
      sum += ((q31_t) * px++ * *py--);

        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q15_t) (sum >> 15);

      /* Update the inputA and inputB pointers for next MAC calculation */
      py = ++pSrc2;
      px = pIn1;

      /* Increment the MAC count */
      count++;

      /* Decrement the loop counter */
      blockSize1--;
    }

    /* The second part of the stage starts here */
    /* The internal loop, over count, is unrolled by 4 */
    /* To, read the last two inputB samples using SIMD:   
     * y[srcBLen] and y[srcBLen-1] coefficients, py is decremented by 1 */
    py = py - 1;

  while(blockSize1 > 0)
    {
      /* Accumulator is made zero for every iteration */
      sum = 0;

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = count >> 2u;

      /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.   
       ** a second loop below computes MACs for the remaining 1 to 3 samples. */
	py++;

    while(k > 0u)
    {
      /* Perform the multiply-accumulates */
        sum += ((q31_t) * px++ * *py--);
        sum += ((q31_t) * px++ * *py--);
        sum += ((q31_t) * px++ * *py--);
        sum += ((q31_t) * px++ * *py--);

      /* Decrement the loop counter */
      k--;
    }

      /* If the count is not a multiple of 4, compute any remaining MACs here.   
       ** No loop unrolling is used. */
      k = count % 0x4u;

      while(k > 0u)
      {
        /* Perform the multiply-accumulates */
      sum += ((q31_t) * px++ * *py--);

        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q15_t) (sum >> 15);

      /* Update the inputA and inputB pointers for next MAC calculation */
      py = ++pSrc2 - 1u;
      px = pIn1;

      /* Increment the MAC count */
      count++;

      /* Decrement the loop counter */
      blockSize1--;
    }

    /* --------------------------   
     * Initializations of stage2   
     * ------------------------*/

    /* sum = x[0] * y[srcBLen-1] + x[1] * y[srcBLen-2] +...+ x[srcBLen-1] * y[0]   
     * sum = x[1] * y[srcBLen-1] + x[2] * y[srcBLen-2] +...+ x[srcBLen] * y[0]   
     * ....   
     * sum = x[srcALen-srcBLen-2] * y[srcBLen-1] + x[srcALen] * y[srcBLen-2] +...+ x[srcALen-1] * y[0]   
     */

    /* Working pointer of inputA */
    if((int32_t)firstIndex - (int32_t)srcBLen + 1 > 0)
    {
      px = pIn1 + firstIndex - srcBLen + 1;
    }
    else
    {
      px = pIn1;
    }

    /* Working pointer of inputB */
    pSrc2 = pIn2 + (srcBLen - 1u);
    py = pSrc2;

    /* count is the index by which the pointer pIn1 to be incremented */
    count = 0u;


    /* --------------------   
     * Stage2 process   
     * -------------------*/

    /* Stage2 depends on srcBLen as in this stage srcBLen number of MACS are performed.   
     * So, to loop unroll over blockSize2,   
     * srcBLen should be greater than or equal to 4 */
    if(srcBLen >= 4u)
    {
      /* Loop unroll over blockSize2, by 4 */
      blkCnt = ((uint32_t) blockSize2 >> 2u);

      while(blkCnt > 0u)
      {
      py = py - 1u;

        /* Set all accumulators to zero */
        acc0 = 0;
        acc1 = 0;
        acc2 = 0;
        acc3 = 0;

      /* read x[0], x[1] samples */
        a = *px++;
        b = *px++;

	VectInA[0] = a; /*x0*/
	VectInA[1] = b; /*x1*/
	a = *px;
	VectInB[0] = b; /*x1*/
	VectInB[1] = a; /*x2*/

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = srcBLen >> 2u;

      /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.   
       ** a second loop below computes MACs for the remaining 1 to 3 samples. */
      do
      {
        /* Read the last two inputB samples using SIMD:   
         * y[srcBLen - 1] and y[srcBLen - 2] */
        a = *py;
        b = *(py+1);
        py -= 2;
        VectInC[0] = a; /*y[srcBLen - 1]*/
        VectInC[1] = b; /*y[srcBLen - 2] */

        /* acc0 +=  x[0] * y[srcBLen - 1] + x[1] * y[srcBLen - 2] */
        acc0 = x_heep_sumdotp2(VectInA, VectInC, acc0);

        /* acc1 +=  x[1] * y[srcBLen - 1] + x[2] * y[srcBLen - 2] */
        acc1 = x_heep_sumdotp2(VectInB, VectInC, acc1);

        a = *px;
        b = *(px + 1);

        VectInD[0] = a; /*x2*/
        VectInD[1] = b; /*x3*/
        a = *(px + 2);
        VectInE[0] = b; /*x3*/
        VectInE[1] = a; /*x4*/
        /* acc2 +=  x[2] * y[srcBLen - 1] + x[3] * y[srcBLen - 2] */
        acc2 = x_heep_sumdotp2(VectInD, VectInC, acc2);

        /* acc3 +=  x[3] * y[srcBLen - 1] + x[4] * y[srcBLen - 2] */
        acc3 = x_heep_sumdotp2(VectInE, VectInC, acc3);

        /* Read y[srcBLen - 3] and y[srcBLen - 4] */
        a = *py;
        b = *(py+1);
        py -= 2;
        VectInC[0] = a; /*y[srcBLen - 3]*/
        VectInC[1] = b; /*y[srcBLen - 4] */

        /* acc0 +=  x[2] * y[srcBLen - 3] + x[3] * y[srcBLen - 4] */
        acc0 = x_heep_sumdotp2(VectInD, VectInC, acc0);

        /* acc1 +=  x[3] * y[srcBLen - 3] + x[4] * y[srcBLen - 4] */
        acc1 = x_heep_sumdotp2(VectInE, VectInC, acc1);

        /* Read x[4], x[5], x[6] */
        a = *(px + 2);
        b = *(px + 3);

        VectInA[0] = a; /*x4*/
        VectInA[1] = b; /*x5*/
        a = *(px + 4);
        VectInB[0] = b; /*x5*/
        VectInB[1] = a; /*x6*/

        px += 4u;

        /* acc2 +=  x[4] * y[srcBLen - 3] + x[5] * y[srcBLen - 4] */
        acc2 = x_heep_sumdotp2(VectInA, VectInC, acc2);

        /* acc3 +=  x[5] * y[srcBLen - 3] + x[6] * y[srcBLen - 4] */
        acc3 = x_heep_sumdotp2(VectInB, VectInC, acc3);

      } while(--k);

      /* For the next MAC operations, SIMD is not used   
       * So, the 16 bit pointer if inputB, py is updated */

      /* If the srcBLen is not a multiple of 4, compute any remaining MACs here.   
       ** No loop unrolling is used. */
      k = srcBLen % 0x4u;

      if(k == 1u)
      {
        /* Read y[srcBLen - 5] */
     
        VectInC[0] = VectInC[1]; 
        VectInC[1] = 0;
        /* Read x[7] */
        a = *px;
        b = *(px+1);
        px++;

        VectInD[0] = a; /*x6*/
        VectInD[1] = b; /*7*/

        acc0 = x_heep_sumdotp2(VectInA, VectInC, acc0);
        acc1 = x_heep_sumdotp2(VectInB, VectInC, acc1);
        acc2 = x_heep_sumdotp2(VectInB, VectInC, acc2);
        acc3 = x_heep_sumdotp2(VectInD, VectInC, acc3);
      }

      if(k == 2u)
      {
        /* Read y[srcBLen - 5], y[srcBLen - 6] */
        a = *py;
        b = *(py+1);
        VectInC[0] = a; 
        VectInC[1] = b;

        /* Read x[7], x[8], x[9] */
        a = *px;
        b = *(px + 1);
        VectInE[0] = a; /*x7*/
        VectInE[1] = b; /*x8*/
        a = *(px + 2);
        VectInD[0] = b; /*x8*/
        VectInD[1] = a; /*x9*/
        px += 2u;

        /* Perform the multiply-accumulates */
        acc0 = x_heep_sumdotp2(VectInA, VectInC, acc0);
        acc1 = x_heep_sumdotp2(VectInB, VectInC, acc1);
        acc2 = x_heep_sumdotp2(VectInE, VectInC, acc2);
        acc3 = x_heep_sumdotp2(VectInD, VectInC, acc3);
      }

      if(k == 3u)
      {
        /* Read y[srcBLen - 5], y[srcBLen - 6] */
        a = *py;
        b = *(py+1);
        VectInC[0] = a; 
        VectInC[1] = b;

        /* Read x[7], x[8], x[9] */
        a = *px;
        b = *(px + 1);

        VectInE[0] = a; /*x7*/
        VectInE[1] = b; /*x8*/
        a = *(px + 2);
        VectInD[0] = b; /*x8*/
        VectInD[1] = a; /*x9*/

        /* Perform the multiply-accumulates */
        acc0 = x_heep_sumdotp2(VectInA, VectInC, acc0);
        acc1 = x_heep_sumdotp2(VectInB, VectInC, acc1);
        acc2 = x_heep_sumdotp2(VectInE, VectInC, acc2);
        acc3 = x_heep_sumdotp2(VectInD, VectInC, acc3);
                VectInC[0] = VectInC[1]; 
                VectInC[1] = 0;

        /* Read x[10] */
        a = *(px+2);
        b = *(px+3);

        VectInE[0] = a; /*x9*/
        VectInE[1] = b; /*x10*/

        px += 3u;

        /* Perform the multiply-accumulates */
        acc0 = x_heep_sumdotp2(VectInB, VectInC, acc0);
        acc1 = x_heep_sumdotp2(VectInD, VectInC, acc1);
        acc2 = x_heep_sumdotp2(VectInD, VectInC, acc2);
        acc3 = x_heep_sumdotp2(VectInE, VectInC, acc3);
      }

      /* Store the results in the accumulators in the destination buffer. */
	/**pOut++ = (q15_t)(acc0 >> 15);
	*pOut++ = (q15_t)(acc1 >> 15);
	*pOut++ = (q15_t)(acc2 >> 15);
	*pOut++ = (q15_t)(acc3 >> 15);*/
        *(shortV*)pOut = x_heep_pack2(x_heep_clip((acc0 >> 15),15),x_heep_clip((acc1 >> 15), 15 ));
        pOut+=2;
        *(shortV*)pOut = x_heep_pack2(x_heep_clip((acc2 >> 15), 15),x_heep_clip((acc3 >> 15), 15 ));
        pOut+=2;
        /* Increment the pointer pIn1 index, count by 4 */
        count += 4u;

        /* Update the inputA and inputB pointers for next MAC calculation */
        px = pIn1 + count;
        py = pSrc2;

        /* Decrement the loop counter */
        blkCnt--;
      }

      /* If the blockSize2 is not a multiple of 4, compute any remaining output samples here.   
       ** No loop unrolling is used. */
      blkCnt = (uint32_t) blockSize2 % 0x4u;

      while(blkCnt > 0u)
      {
        /* Accumulator is made zero for every iteration */
        sum = 0;

        /* Apply loop unrolling and compute 4 MACs simultaneously. */
        k = srcBLen >> 2u;

        /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.   
         ** a second loop below computes MACs for the remaining 1 to 3 samples. */
        while(k > 0u)
        {
          /* Perform the multiply-accumulates */
          sum = x_heep_macu(sum, * px++  ,  *py--);
          sum = x_heep_macu(sum, * px++  ,  *py--);
          sum = x_heep_macu(sum, * px++  ,  *py--);
          sum = x_heep_macu(sum, * px++  ,  *py--);
          /* Decrement the loop counter */
          k--;
        }

        /* If the srcBLen is not a multiple of 4, compute any remaining MACs here.   
         ** No loop unrolling is used. */
        k = srcBLen % 0x4u;

        while(k > 0u)
        {
          /* Perform the multiply-accumulates */
          sum = x_heep_macu(sum, * px++  ,  *py--);
          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = (q15_t) (sum >> 15);

        /* Increment the pointer pIn1 index, count by 1 */
        count++;

        /* Update the inputA and inputB pointers for next MAC calculation */
        px = pIn1 + count;
        py = pSrc2;

        /* Decrement the loop counter */
        blkCnt--;
      }
    }
    else
    {
      /* If the srcBLen is not a multiple of 4,   
       * the blockSize2 loop cannot be unrolled by 4 */
      blkCnt = (uint32_t) blockSize2;

      while(blkCnt > 0u)
      {
        /* Accumulator is made zero for every iteration */
        sum = 0;

        /* srcBLen number of MACS should be performed */
        k = srcBLen;

        while(k > 0u)
        {
          /* Perform the multiply-accumulate */
          sum = x_heep_macu(sum, * px++  ,  *py--);
          /* Decrement the loop counter */
          k--;
        }

        /* Store the result in the accumulator in the destination buffer. */
        *pOut++ = (q15_t) (sum >> 15);

        /* Increment the MAC count */
        count++;

        /* Update the inputA and inputB pointers for next MAC calculation */
        px = pIn1 + count;
        py = pSrc2;

        /* Decrement the loop counter */
        blkCnt--;
      }
    }


    /* --------------------------   
     * Initializations of stage3   
     * -------------------------*/

    /* sum += x[srcALen-srcBLen+1] * y[srcBLen-1] + x[srcALen-srcBLen+2] * y[srcBLen-2] +...+ x[srcALen-1] * y[1]   
     * sum += x[srcALen-srcBLen+2] * y[srcBLen-1] + x[srcALen-srcBLen+3] * y[srcBLen-2] +...+ x[srcALen-1] * y[2]   
     * ....   
     * sum +=  x[srcALen-2] * y[srcBLen-1] + x[srcALen-1] * y[srcBLen-2]   
     * sum +=  x[srcALen-1] * y[srcBLen-1]   
     */

    /* In this stage the MAC operations are decreased by 1 for every iteration.   
       The count variable holds the number of MAC operations performed */
    count = srcBLen - 1u;

    /* Working pointer of inputA */
    pSrc1 = (pIn1 + srcALen) - (srcBLen - 1u);
    px = pSrc1;

    /* Working pointer of inputB */
    pSrc2 = pIn2 + (srcBLen - 1u);
    pIn2 = pSrc2 - 1u;
    py = pIn2;

    /* -------------------   
     * Stage3 process   
     * ------------------*/

    /* For loop unrolling by 4, this stage is divided into two. */
    /* First part of this stage computes the MAC operations greater than 4 */
    /* Second part of this stage computes the MAC operations less than or equal to 4 */

    /* The first part of the stage starts here */
    j = count >> 2u;

    while((j > 0u) && (blockSize3 > 0))
    {
      /* Accumulator is made zero for every iteration */
      sum = 0;

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = count >> 2u;

      /* First part of the processing with loop unrolling.  Compute 4 MACs at a time.   
       ** a second loop below computes MACs for the remaining 1 to 3 samples. */
	py++;

    while(k > 0u)
    {	
      /* Perform the multiply-accumulates */
        sum = x_heep_macu(sum, * px++  ,  *py--);
        sum = x_heep_macu(sum, * px++  ,  *py--);
        sum = x_heep_macu(sum, * px++  ,  *py--);
        sum = x_heep_macu(sum, * px++  ,  *py--);
      /* Decrement the loop counter */
      k--;
    }


      /* If the count is not a multiple of 4, compute any remaining MACs here.   
       ** No loop unrolling is used. */
      k = count % 0x4u;

      while(k > 0u)
      {
      /* Perform the multiply-accumulates */
        sum = x_heep_macu(sum, * px++  ,  *py--);
        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q15_t) (sum >> 15);

      /* Update the inputA and inputB pointers for next MAC calculation */
      px = ++pSrc1;
      py = pIn2;

      /* Decrement the MAC count */
      count--;

      /* Decrement the loop counter */
      blockSize3--;

      j--;
    }

    /* The second part of the stage starts here */
    /* SIMD is not used for the next MAC operations,   
     * so pointer py is updated to read only one sample at a time */
    py = py + 1u;

  while(blockSize3 > 0)
    {
      /* Accumulator is made zero for every iteration */
      sum = 0;

      /* Apply loop unrolling and compute 4 MACs simultaneously. */
      k = count;

      while(k > 0u)
      {
        /* Perform the multiply-accumulates */
        /* sum +=  x[srcALen-1] * y[srcBLen-1] */
        sum = x_heep_macu(sum, * px++  ,  *py--);
        /* Decrement the loop counter */
        k--;
      }

      /* Store the result in the accumulator in the destination buffer. */
      *pOut++ = (q15_t) (sum >> 15);

      /* Update the inputA and inputB pointers for next MAC calculation */
      px = ++pSrc1;
      py = pSrc2;

      /* Decrement the MAC count */
      count--;

      /* Decrement the loop counter */
      blockSize3--;
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
