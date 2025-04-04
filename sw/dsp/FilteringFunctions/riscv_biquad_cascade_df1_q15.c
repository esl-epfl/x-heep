/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_biquad_cascade_df1_q15.c    
*    
* Description:	Processing function for the    
*				Q15 Biquad cascade DirectFormI(DF1) filter.    
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
 * @ingroup groupFilters    
 */

/**    
 * @addtogroup BiquadCascadeDF1    
 * @{    
 */

/**    
 * @brief Processing function for the Q15 Biquad cascade filter.    
 * @param[in]  *S points to an instance of the Q15 Biquad cascade structure.    
 * @param[in]  *pSrc points to the block of input data.    
 * @param[out] *pDst points to the location where the output result is written.    
 * @param[in]  blockSize number of samples to process per call.    
 * @return none.    
 *    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function is implemented using a 64-bit internal accumulator.    
 * Both coefficients and state variables are represented in 1.15 format and multiplications yield a 2.30 result.    
 * The 2.30 intermediate results are accumulated in a 64-bit accumulator in 34.30 format.    
 * There is no risk of internal overflow with this approach and the full precision of intermediate multiplications is preserved.    
 * The accumulator is then shifted by <code>postShift</code> bits to truncate the result to 1.15 format by discarding the low 16 bits.    
 * Finally, the result is saturated to 1.15 format.    
 *    
 * \par    
 * Refer to the function <code>riscv_biquad_cascade_df1_fast_q15()</code> for a faster but less precise implementation of this filter for Cortex-M3 and Cortex-M4.    
 */

void riscv_biquad_cascade_df1_q15(
  const riscv_biquad_casd_df1_inst_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  q15_t *pIn = pSrc;                             /*  Source pointer                               */
  q15_t *pOut = pDst;                            /*  Destination pointer                          */
  q15_t b0, b1, b2, a1, a2;                      /*  Filter coefficients           */
  q15_t Xn1, Xn2, Yn1, Yn2;                      /*  Filter state variables        */
  q15_t Xn;                                      /*  temporary input               */
  q63_t acc;                                     /*  Accumulator                                  */
  int32_t shift = (15 - (int32_t)S->postShift); /*  Post shift                                   */
  q15_t *pState = S->pState;                     /*  State pointer                                */
  q15_t *pCoeffs = S->pCoeffs;                   /*  Coefficient pointer                          */
  uint32_t sample, stage = (uint32_t)S->numStages;     /*  Stage loop counter                           */
#if defined (USE_DSP_RISCV)

  shortV *VectInA;
  shortV *VectInB;
  shortV *VectInC;
  shortV *VectInD;
  shortV *VectInb0;
  do
  {
    /* Reading the coefficients */

    b0 = *pCoeffs++;
    pCoeffs++;  // skip the 0 coefficient
    VectInA = (shortV*)pCoeffs; /*b1 b2*/
    pCoeffs+=2;
    VectInB = (shortV*)pCoeffs; /*a1 a2*/
    pCoeffs+=2;
    /* Reading the state values */
    VectInC = (shortV*)pState; /*Xn1 Xn2*/
    VectInD = (shortV*)(pState+2); /*Yn1 Yn2*/

    /*      The variables acc holds the output value that is computed:         
     *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]         
     */

    sample = blockSize;

    while(sample > 0u)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      /* acc =  b0 * x[n] */
      acc = (q31_t) b0 *Xn;
      acc += x_heep_dotp2(*VectInA,*VectInC);
      acc += x_heep_dotp2(*VectInB,*VectInD);
      acc =x_heep_clip((acc >> shift), 15);

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc    */
      (*VectInC) = x_heep_pack2(Xn,(*VectInC)[0]);
      (*VectInD) = x_heep_pack2(acc,(*VectInD)[0]);
      /* Store the output in the destination buffer. */
      *pOut++ = (q15_t) acc;

      /* decrement the loop counter */
      sample--;
    }

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent stages occur in-place in the output buffer */
    pIn = pDst;

    /* Reset to destination pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    *(shortV*)pState = *VectInC;
    pState+=2;
    *(shortV*)pState = *VectInD;
    pState+=2;

  } while(--stage);
#else
  do
  {
    /* Reading the coefficients */
    b0 = *pCoeffs++;
    pCoeffs++;  // skip the 0 coefficient
    b1 = *pCoeffs++;
    b2 = *pCoeffs++;
    a1 = *pCoeffs++;
    a2 = *pCoeffs++;

    /* Reading the state values */
    Xn1 = pState[0];
    Xn2 = pState[1];
    Yn1 = pState[2];
    Yn2 = pState[3];

    /*      The variables acc holds the output value that is computed:         
     *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2]         
     */

    sample = blockSize;

    while(sample > 0u)
    {
      /* Read the input */
      Xn = *pIn++;

      /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
      /* acc =  b0 * x[n] */
      acc = (q31_t) b0 *Xn;

      /* acc +=  b1 * x[n-1] */
      acc += (q31_t) b1 *Xn1;
      /* acc +=  b[2] * x[n-2] */
      acc += (q31_t) b2 *Xn2;
      /* acc +=  a1 * y[n-1] */
      acc += (q31_t) a1 *Yn1;
      /* acc +=  a2 * y[n-2] */
      acc += (q31_t) a2 *Yn2;

      /* The result is converted to 1.31  */
      acc = __SSAT((acc >> shift), 16);

      /* Every time after the output is computed state should be updated. */
      /* The states should be updated as:  */
      /* Xn2 = Xn1    */
      /* Xn1 = Xn     */
      /* Yn2 = Yn1    */
      /* Yn1 = acc    */
      Xn2 = Xn1;
      Xn1 = Xn;
      Yn2 = Yn1;
      Yn1 = (q15_t) acc;

      /* Store the output in the destination buffer. */
      *pOut++ = (q15_t) acc;

      /* decrement the loop counter */
      sample--;
    }

    /*  The first stage goes from the input buffer to the output buffer. */
    /*  Subsequent stages occur in-place in the output buffer */
    pIn = pDst;

    /* Reset to destination pointer */
    pOut = pDst;

    /*  Store the updated state variables back into the pState array */
    *pState++ = Xn1;
    *pState++ = Xn2;
    *pState++ = Yn1;
    *pState++ = Yn2;

  } while(--stage);
#endif
}


/**    
 * @} end of BiquadCascadeDF1 group    
 */
