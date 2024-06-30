/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_fir_lattice_f32.c    
*    
* Description:	Processing function for the floating-point FIR Lattice filter.    
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
 * @ingroup groupFilters    
 */

/**    
 * @defgroup FIR_Lattice Finite Impulse Response (FIR) Lattice Filters    
 *    
 * This set of functions implements Finite Impulse Response (FIR) lattice filters    
 * for Q15, Q31 and floating-point data types.  Lattice filters are used in a     
 * variety of adaptive filter applications.  The filter structure is feedforward and    
 * the net impulse response is finite length.    
 * The functions operate on blocks    
 * of input and output data and each call to the function processes    
 * <code>blockSize</code> samples through the filter.  <code>pSrc</code> and    
 * <code>pDst</code> point to input and output arrays containing <code>blockSize</code> values.    
 *    
 * \par Algorithm:    
 * \image html FIRLattice.gif "Finite Impulse Response Lattice filter"    
 * The following difference equation is implemented:    
 * <pre>    
 *    f0[n] = g0[n] = x[n]    
 *    fm[n] = fm-1[n] + km * gm-1[n-1] for m = 1, 2, ...M    
 *    gm[n] = km * fm-1[n] + gm-1[n-1] for m = 1, 2, ...M    
 *    y[n] = fM[n]    
 * </pre>    
 * \par    
 * <code>pCoeffs</code> points to tha array of reflection coefficients of size <code>numStages</code>.    
 * Reflection Coefficients are stored in the following order.    
 * \par    
 * <pre>    
 *    {k1, k2, ..., kM}    
 * </pre>    
 * where M is number of stages    
 * \par    
 * <code>pState</code> points to a state array of size <code>numStages</code>.    
 * The state variables (g values) hold previous inputs and are stored in the following order.    
 * <pre>    
 *    {g0[n], g1[n], g2[n] ...gM-1[n]}    
 * </pre>    
 * The state variables are updated after each block of data is processed; the coefficients are untouched.    
 * \par Instance Structure    
 * The coefficients and state variables for a filter are stored together in an instance data structure.    
 * A separate instance structure must be defined for each filter.    
 * Coefficient arrays may be shared among several instances while state variable arrays cannot be shared.    
 * There are separate instance structure declarations for each of the 3 supported data types.    
 *    
 * \par Initialization Functions    
 * There is also an associated initialization function for each data type.    
 * The initialization function performs the following operations:    
 * - Sets the values of the internal structure fields.    
 * - Zeros out the values in the state buffer.    
 * To do this manually without calling the init function, assign the follow subfields of the instance structure:
 * numStages, pCoeffs, pState. Also set all of the values in pState to zero. 
 *    
 * \par    
 * Use of the initialization function is optional.    
 * However, if the initialization function is used, then the instance structure cannot be placed into a const data section.    
 * To place an instance structure into a const data section, the instance structure must be manually initialized.    
 * Set the values in the state buffer to zeros and then manually initialize the instance structure as follows:    
 * <pre>    
 *riscv_fir_lattice_instance_f32 S = {numStages, pState, pCoeffs};    
 *riscv_fir_lattice_instance_q31 S = {numStages, pState, pCoeffs};    
 *riscv_fir_lattice_instance_q15 S = {numStages, pState, pCoeffs};    
 * </pre>    
 * \par    
 * where <code>numStages</code> is the number of stages in the filter; <code>pState</code> is the address of the state buffer;    
 * <code>pCoeffs</code> is the address of the coefficient buffer.    
 * \par Fixed-Point Behavior    
 * Care must be taken when using the fixed-point versions of the FIR Lattice filter functions.    
 * In particular, the overflow and saturation behavior of the accumulator used in each function must be considered.    
 * Refer to the function specific documentation below for usage guidelines.    
 */

/**    
 * @addtogroup FIR_Lattice    
 * @{    
 */


  /**    
   * @brief Processing function for the floating-point FIR lattice filter.    
   * @param[in]  *S        points to an instance of the floating-point FIR lattice structure.    
   * @param[in]  *pSrc     points to the block of input data.    
   * @param[out] *pDst     points to the block of output data    
   * @param[in]  blockSize number of samples to process.    
   * @return none.    
   */

void riscv_fir_lattice_f32(
  const riscv_fir_lattice_instance_f32 * S,
  float32_t * pSrc,
  float32_t * pDst,
  uint32_t blockSize)
{
  float32_t *pState;                             /* State pointer */
  float32_t *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
  float32_t *px;                                 /* temporary state pointer */
  float32_t *pk;                                 /* temporary coefficient pointer */


  float32_t fcurr, fnext, gcurr, gnext;          /* temporary variables */
  uint32_t numStages = S->numStages;             /* Length of the filter */
  uint32_t blkCnt, stageCnt;                     /* temporary variables for counts */

  pState = &S->pState[0];

  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* f0(n) = x(n) */
    fcurr = *pSrc++;

    /* Initialize coeff pointer */
    pk = pCoeffs;

    /* Initialize state pointer */
    px = pState;

    /* read g0(n-1) from state buffer */
    gcurr = *px;

    /* for sample 1 processing */
    /* f1(n) = f0(n) +  K1 * g0(n-1) */
    fnext = fcurr + ((*pk) * gcurr);
    /* g1(n) = f0(n) * K1  +  g0(n-1) */
    gnext = (fcurr * (*pk++)) + gcurr;

    /* save f0(n) in state buffer */
    *px++ = fcurr;

    /* f1(n) is saved in fcurr            
       for next stage processing */
    fcurr = fnext;

    stageCnt = (numStages - 1u);

    /* stage loop */
    while(stageCnt > 0u)
    {
      /* read g2(n) from state buffer */
      gcurr = *px;

      /* save g1(n) in state buffer */
      *px++ = gnext;

      /* Sample processing for K2, K3.... */
      /* f2(n) = f1(n) +  K2 * g1(n-1) */
      fnext = fcurr + ((*pk) * gcurr);
      /* g2(n) = f1(n) * K2  +  g1(n-1) */
      gnext = (fcurr * (*pk++)) + gcurr;

      /* f1(n) is saved in fcurr1            
         for next stage processing */
      fcurr = fnext;

      stageCnt--;

    }

    /* y(n) = fN(n) */
    *pDst++ = fcurr;

    blkCnt--;

  }


}

/**    
 * @} end of FIR_Lattice group    
 */
