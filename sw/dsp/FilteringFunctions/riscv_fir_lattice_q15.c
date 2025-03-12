/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_fir_lattice_q15.c    
*    
* Description:	Q15 FIR lattice filter processing function.    
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
 * @addtogroup FIR_Lattice    
 * @{    
 */


/**    
 * @brief Processing function for the Q15 FIR lattice filter.    
 * @param[in]  *S        points to an instance of the Q15 FIR lattice structure.    
 * @param[in]  *pSrc     points to the block of input data.    
 * @param[out] *pDst     points to the block of output data    
 * @param[in]  blockSize number of samples to process.    
 * @return none.    
 */

void riscv_fir_lattice_q15(
  const riscv_fir_lattice_instance_q15 * S,
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{


  q15_t *pState;                                 /* State pointer */
  q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
  q15_t *px;                                     /* temporary state pointer */
  q15_t *pk;     

#if defined (USE_DSP_RISCV)


  q31_t fcurnt1, fnext1, gcurnt1 = 0, gnext1;    /* temporary variables for first sample in loop unrolling */
  q31_t fcurnt2, fnext2, gnext2;                 /* temporary variables for second sample in loop unrolling */
  q31_t fcurnt3, fnext3, gnext3;                 /* temporary variables for third sample in loop unrolling */
  q31_t fcurnt4, fnext4, gnext4;                 /* temporary variables for fourth sample in loop unrolling */
  uint32_t numStages = S->numStages;             /* Number of stages in the filter */
  uint32_t blkCnt, stageCnt;                     /* temporary variables for counts */

  pState = &S->pState[0];

  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.    
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {

    /* Read two samples from input buffer */
    /* f0(n) = x(n) */
    fcurnt1 = *pSrc++;
    fcurnt2 = *pSrc++;

    /* Initialize coeff pointer */
    pk = (pCoeffs);

    /* Initialize state pointer */
    px = pState;

    /* Read g0(n-1) from state */
    gcurnt1 = *px;

    /* Process first sample for first tap */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15u) + fcurnt1;
    fnext1 = x_heep_clip(fnext1, 15);

    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext1 = (q31_t) ((fcurnt1 * (*pk)) >> 15u) + gcurnt1;
    gnext1 = x_heep_clip(gnext1, 15);

    /* Process second sample for first tap */
    /* for sample 2 processing */
    fnext2 = (q31_t) ((fcurnt1 * (*pk)) >> 15u) + fcurnt2;
    fnext2 = x_heep_clip(fnext2, 15);

    gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15u) + fcurnt1;
    gnext2 = x_heep_clip(gnext2, 15);


    /* Read next two samples from input buffer */
    /* f0(n+2) = x(n+2) */
    fcurnt3 = *pSrc++;
    fcurnt4 = *pSrc++;

    /* Copy only last input samples into the state buffer    
       which is used for next four samples processing */
    *px++ = (q15_t) fcurnt4;

    /* Process third sample for first tap */
    fnext3 = (q31_t) ((fcurnt2 * (*pk)) >> 15u) + fcurnt3;
    fnext3 = x_heep_clip(fnext3, 15);
    gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15u) + fcurnt2;
    gnext3 = x_heep_clip(gnext3, 15);

    /* Process fourth sample for first tap */
    fnext4 = (q31_t) ((fcurnt3 * (*pk)) >> 15u) + fcurnt4;
    fnext4 = x_heep_clip(fnext4,15);
    gnext4 = (q31_t) ((fcurnt4 * (*pk++)) >> 15u) + fcurnt3;
    gnext4 = x_heep_clip(gnext4, 15);

    /* Update of f values for next coefficient set processing */
    fcurnt1 = fnext1;
    fcurnt2 = fnext2;
    fcurnt3 = fnext3;
    fcurnt4 = fnext4;


    /* Loop unrolling.  Process 4 taps at a time . */
    stageCnt = (numStages - 1u) >> 2;


    /* Loop over the number of taps.  Unroll by a factor of 4.    
     ** Repeat until we've computed numStages-3 coefficients. */

    /* Process 2nd, 3rd, 4th and 5th taps ... here */
    while(stageCnt > 0u)
    {
      /* Read g1(n-1), g3(n-1) .... from state */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Process first sample for 2nd, 6th .. tap */
      /* Sample processing for K2, K6.... */
      /* f1(n) = f0(n) +  K1 * g0(n-1) */
      fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15u) + fcurnt1;
      fnext1 = x_heep_clip(fnext1, 15);


      /* Process second sample for 2nd, 6th .. tap */
      /* for sample 2 processing */
      fnext2 = (q31_t) ((gnext1 * (*pk)) >> 15u) + fcurnt2;
      fnext2 = x_heep_clip(fnext2, 15);
      /* Process third sample for 2nd, 6th .. tap */
      fnext3 = (q31_t) ((gnext2 * (*pk)) >> 15u) + fcurnt3;
      fnext3 = x_heep_clip(fnext3, 15);
      /* Process fourth sample for 2nd, 6th .. tap */
      /* fnext4 = fcurnt4 + (*pk) * gnext3; */
      fnext4 = (q31_t) ((gnext3 * (*pk)) >> 15u) + fcurnt4;
      fnext4 = x_heep_clip(fnext4,15);

      /* g1(n) = f0(n) * K1  +  g0(n-1) */
      /* Calculation of state values for next stage */
      gnext4 = (q31_t) ((fcurnt4 * (*pk)) >> 15u) + gnext3;
      gnext4 = x_heep_clip(gnext4, 15);
      gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15u) + gnext2;
      gnext3 = x_heep_clip(gnext3, 15);

      gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15u) + gnext1;
      gnext2 = x_heep_clip(gnext2, 15);

      gnext1 = (q31_t) ((fcurnt1 * (*pk++)) >> 15u) + gcurnt1;
      gnext1 = x_heep_clip(gnext1, 15);


      /* Read g2(n-1), g4(n-1) .... from state */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Sample processing for K3, K7.... */
      /* Process first sample for 3rd, 7th .. tap */
      /* f3(n) = f2(n) +  K3 * g2(n-1) */
      fcurnt1 = (q31_t) ((gcurnt1 * (*pk)) >> 15u) + fnext1;
      fcurnt1 = x_heep_clip(fcurnt1, 15);

      /* Process second sample for 3rd, 7th .. tap */
      fcurnt2 = (q31_t) ((gnext1 * (*pk)) >> 15u) + fnext2;
      fcurnt2 = x_heep_clip(fcurnt2, 15);

      /* Process third sample for 3rd, 7th .. tap */
      fcurnt3 = (q31_t) ((gnext2 * (*pk)) >> 15u) + fnext3;
      fcurnt3 = x_heep_clip(fcurnt3, 15);

      /* Process fourth sample for 3rd, 7th .. tap */
      fcurnt4 = (q31_t) ((gnext3 * (*pk)) >> 15u) + fnext4;
      fcurnt4 = x_heep_clip(fcurnt4, 15);

      /* Calculation of state values for next stage */
      /* g3(n) = f2(n) * K3  +  g2(n-1) */
      gnext4 = (q31_t) ((fnext4 * (*pk)) >> 15u) + gnext3;
      gnext4 = x_heep_clip(gnext4, 15);

      gnext3 = (q31_t) ((fnext3 * (*pk)) >> 15u) + gnext2;
      gnext3 = x_heep_clip(gnext3, 15);

      gnext2 = (q31_t) ((fnext2 * (*pk)) >> 15u) + gnext1;
      gnext2 = x_heep_clip(gnext2, 15);

      gnext1 = (q31_t) ((fnext1 * (*pk++)) >> 15u) + gcurnt1;
      gnext1 = x_heep_clip(gnext1, 15);

      /* Read g1(n-1), g3(n-1) .... from state */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Sample processing for K4, K8.... */
      /* Process first sample for 4th, 8th .. tap */
      /* f4(n) = f3(n) +  K4 * g3(n-1) */
      fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15u) + fcurnt1;
      fnext1 = x_heep_clip(fnext1, 15);

      /* Process second sample for 4th, 8th .. tap */
      /* for sample 2 processing */
      fnext2 = (q31_t) ((gnext1 * (*pk)) >> 15u) + fcurnt2;
      fnext2 = x_heep_clip(fnext2, 15);

      /* Process third sample for 4th, 8th .. tap */
      fnext3 = (q31_t) ((gnext2 * (*pk)) >> 15u) + fcurnt3;
      fnext3 = x_heep_clip(fnext3, 15);

      /* Process fourth sample for 4th, 8th .. tap */
      fnext4 = (q31_t) ((gnext3 * (*pk)) >> 15u) + fcurnt4;
      fnext4 = x_heep_clip(fnext4, 15);

      /* g4(n) = f3(n) * K4  +  g3(n-1) */
      /* Calculation of state values for next stage */
      gnext4 = (q31_t) ((fcurnt4 * (*pk)) >> 15u) + gnext3;
      gnext4 = x_heep_clip(gnext4, 15);

      gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15u) + gnext2;
      gnext3 = x_heep_clip(gnext3, 15);

      gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15u) + gnext1;
      gnext2 = x_heep_clip(gnext2, 15);
      gnext1 = (q31_t) ((fcurnt1 * (*pk++)) >> 15u) + gcurnt1;
      gnext1 = x_heep_clip(gnext1, 15);


      /* Read g2(n-1), g4(n-1) .... from state */
      gcurnt1 = *px;

      /* save g4(n) in state buffer */
      *px++ = (q15_t) gnext4;

      /* Sample processing for K5, K9.... */
      /* Process first sample for 5th, 9th .. tap */
      /* f5(n) = f4(n) +  K5 * g4(n-1) */
      fcurnt1 = (q31_t) ((gcurnt1 * (*pk)) >> 15u) + fnext1;
      fcurnt1 = x_heep_clip(fcurnt1, 15);

      /* Process second sample for 5th, 9th .. tap */
      fcurnt2 = (q31_t) ((gnext1 * (*pk)) >> 15u) + fnext2;
      fcurnt2 = x_heep_clip(fcurnt2, 15);

      /* Process third sample for 5th, 9th .. tap */
      fcurnt3 = (q31_t) ((gnext2 * (*pk)) >> 15u) + fnext3;
      fcurnt3 = x_heep_clip(fcurnt3, 15);

      /* Process fourth sample for 5th, 9th .. tap */
      fcurnt4 = (q31_t) ((gnext3 * (*pk)) >> 15u) + fnext4;
      fcurnt4 = x_heep_clip(fcurnt4, 15);

      /* Calculation of state values for next stage */
      /* g5(n) = f4(n) * K5  +  g4(n-1) */
      gnext4 = (q31_t) ((fnext4 * (*pk)) >> 15u) + gnext3;
      gnext4 = x_heep_clip(gnext4, 15);
      gnext3 = (q31_t) ((fnext3 * (*pk)) >> 15u) + gnext2;
      gnext3 = x_heep_clip(gnext3, 15);
      gnext2 = (q31_t) ((fnext2 * (*pk)) >> 15u) + gnext1;
      gnext2 = x_heep_clip(gnext2, 15);
      gnext1 = (q31_t) ((fnext1 * (*pk++)) >> 15u) + gcurnt1;
      gnext1 = x_heep_clip(gnext1, 15);

      stageCnt--;
    }

    /* If the (filter length -1) is not a multiple of 4, compute the remaining filter taps */
    stageCnt = (numStages - 1u) % 0x4u;

    while(stageCnt > 0u)
    {
      gcurnt1 = *px;

      /* save g value in state buffer */
      *px++ = (q15_t) gnext4;

      /* Process four samples for last three taps here */
      fnext1 = (q31_t) ((gcurnt1 * (*pk)) >> 15u) + fcurnt1;
      fnext1 = x_heep_clip(fnext1, 15);
      fnext2 = (q31_t) ((gnext1 * (*pk)) >> 15u) + fcurnt2;
      fnext2 = x_heep_clip(fnext2, 15);

      fnext3 = (q31_t) ((gnext2 * (*pk)) >> 15u) + fcurnt3;
      fnext3 = x_heep_clip(fnext3, 15);

      fnext4 = (q31_t) ((gnext3 * (*pk)) >> 15u) + fcurnt4;
      fnext4 = x_heep_clip(fnext4, 15);

      /* g1(n) = f0(n) * K1  +  g0(n-1) */
      gnext4 = (q31_t) ((fcurnt4 * (*pk)) >> 15u) + gnext3;
      gnext4 = x_heep_clip(gnext4, 15);
      gnext3 = (q31_t) ((fcurnt3 * (*pk)) >> 15u) + gnext2;
      gnext3 = x_heep_clip(gnext3, 15);
      gnext2 = (q31_t) ((fcurnt2 * (*pk)) >> 15u) + gnext1;
      gnext2 = x_heep_clip(gnext2, 15);
      gnext1 = (q31_t) ((fcurnt1 * (*pk++)) >> 15u) + gcurnt1;
      gnext1 = x_heep_clip(gnext1, 15);

      /* Update of f values for next coefficient set processing */
      fcurnt1 = fnext1;
      fcurnt2 = fnext2;
      fcurnt3 = fnext3;
      fcurnt4 = fnext4;

      stageCnt--;

    }

    /* The results in the 4 accumulators, store in the destination buffer. */
    /* y(n) = fN(n) */

    *(shortV*)pDst = x_heep_pack2(fcurnt1,fcurnt2);
    pDst+=2;
    *(shortV*)pDst = x_heep_pack2(fcurnt3,fcurnt4);
    pDst+=2;


    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* f0(n) = x(n) */
    fcurnt1 = *pSrc++;

    /* Initialize coeff pointer */
    pk = (pCoeffs);

    /* Initialize state pointer */
    px = pState;

    /* read g2(n) from state buffer */
    gcurnt1 = *px;

    /* for sample 1 processing */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext1 = (((q31_t) gcurnt1 * (*pk)) >> 15u) + fcurnt1;
    fnext1 = x_heep_clip(fnext1, 15);


    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext1 = (((q31_t) fcurnt1 * (*pk++)) >> 15u) + gcurnt1;
    gnext1 = x_heep_clip(gnext1, 15);

    /* save g1(n) in state buffer */
    *px++ = (q15_t) fcurnt1;

    /* f1(n) is saved in fcurnt1    
       for next stage processing */
    fcurnt1 = fnext1;

    stageCnt = (numStages - 1u);

    /* stage loop */
    while(stageCnt > 0u)
    {
      /* read g2(n) from state buffer */
      gcurnt1 = *px;

      /* save g1(n) in state buffer */
      *px++ = (q15_t) gnext1;

      /* Sample processing for K2, K3.... */
      /* f2(n) = f1(n) +  K2 * g1(n-1) */
      fnext1 = (((q31_t) gcurnt1 * (*pk)) >> 15u) + fcurnt1;
      fnext1 = x_heep_clip(fnext1, 15);

      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext1 = (((q31_t) fcurnt1 * (*pk++)) >> 15u) + gcurnt1;
      gnext1 = x_heep_clip(gnext1, 15);


      /* f1(n) is saved in fcurnt1    
         for next stage processing */
      fcurnt1 = fnext1;

      stageCnt--;

    }

    /* y(n) = fN(n) */
    *pDst++ = x_heep_clip(fcurnt1, 15);


    blkCnt--;

  }

#else

  q31_t fcurnt, fnext, gcurnt, gnext;            /* temporary variables */
  uint32_t numStages = S->numStages;             /* Length of the filter */
  uint32_t blkCnt, stageCnt;                     /* temporary variables for counts */

  pState = &S->pState[0];

  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* f0(n) = x(n) */
    fcurnt = *pSrc++;

    /* Initialize coeff pointer */
    pk = (pCoeffs);

    /* Initialize state pointer */
    px = pState;

    /* read g0(n-1) from state buffer */
    gcurnt = *px;

    /* for sample 1 processing */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext = ((gcurnt * (*pk)) >> 15u) + fcurnt;
    fnext = __SSAT(fnext, 16);


    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext = ((fcurnt * (*pk++)) >> 15u) + gcurnt;
    gnext = __SSAT(gnext, 16);

    /* save f0(n) in state buffer */
    *px++ = (q15_t) fcurnt;

    /* f1(n) is saved in fcurnt            
       for next stage processing */
    fcurnt = fnext;

    stageCnt = (numStages - 1u);

    /* stage loop */
    while(stageCnt > 0u)
    {
      /* read g1(n-1) from state buffer */
      gcurnt = *px;

      /* save g0(n-1) in state buffer */
      *px++ = (q15_t) gnext;

      /* Sample processing for K2, K3.... */
      /* f2(n) = f1(n) +  K2 * g1(n-1) */
      fnext = ((gcurnt * (*pk)) >> 15u) + fcurnt;
      fnext = __SSAT(fnext, 16);

      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext = ((fcurnt * (*pk++)) >> 15u) + gcurnt;
      gnext = __SSAT(gnext, 16);


      /* f1(n) is saved in fcurnt            
         for next stage processing */
      fcurnt = fnext;

      stageCnt--;

    }

    /* y(n) = fN(n) */
    *pDst++ = __SSAT(fcurnt, 16);


    blkCnt--;

  }
#endif

}

/**    
 * @} end of FIR_Lattice group    
 */
