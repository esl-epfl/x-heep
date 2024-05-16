#include "riscv_math.h"
#include "x_heep_emul.h"

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