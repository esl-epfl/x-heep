/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_pid_init_q15.c    
*    
* Description:	Q15 PID Control initialization function    
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
 * @addtogroup PID    
 * @{    
 */

/**    
 * @details    
 * @param[in,out] *S points to an instance of the Q15 PID structure.    
 * @param[in]     resetStateFlag  flag to reset the state. 0 = no change in state 1 = reset the state.    
 * @return none.    
 * \par Description:   
 * \par    
 * The <code>resetStateFlag</code> specifies whether to set state to zero or not. \n   
 * The function computes the structure fields: <code>A0</code>, <code>A1</code> <code>A2</code>    
 * using the proportional gain( \c Kp), integral gain( \c Ki) and derivative gain( \c Kd)    
 * also sets the state variables to all zeros.    
 */

void riscv_pid_init_q15(
  riscv_pid_instance_q15 * S,
  int32_t resetStateFlag)
{
  q31_t temp;                                    /*to store the sum */
  /* Derived coefficient A0 */
#if defined (USE_DSP_RISCV)
  temp = S->Kp + S->Ki + S->Kd;
  S->A0 = (q15_t) x_heep_clip(temp, 15);
  temp = -(S->Kd + S->Kd + S->Kp);
  S->A1 = x_heep_pack2(x_heep_clip(temp,15),S->Kd);
#else

  temp = S->Kp + S->Ki + S->Kd;
  S->A0 = (q15_t) __SSAT(temp, 16);

  /* Derived coefficients and pack into A1 */
  temp = -(S->Kd + S->Kd + S->Kp);
  S->A1 = (q15_t) __SSAT(temp, 16);
  S->A2 = S->Kd;

#endif

  /* Check whether state needs reset or not */
  if(resetStateFlag)
  {
    /* Clear the state buffer.  The size will be always 3 samples */
    memset(S->state, 0, 3u * sizeof(q15_t));
  }


}

/**    
 * @} end of PID group    
 */










