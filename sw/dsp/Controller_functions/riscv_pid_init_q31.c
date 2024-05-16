#include "riscv_math.h"

void riscv_pid_init_q31(
  riscv_pid_instance_q31 * S,
  int32_t resetStateFlag)
{
  q31_t temp;

  /* Derived coefficient A0 */
  temp = clip_q63_to_q31((q63_t) S->Kp + S->Ki);
  S->A0 = clip_q63_to_q31((q63_t) temp + S->Kd);

  /* Derived coefficient A1 */
  temp = clip_q63_to_q31((q63_t) S->Kd + S->Kd);
  S->A1 = -clip_q63_to_q31((q63_t) temp + S->Kp);


  /* Derived coefficient A2 */
  S->A2 = S->Kd;

  /* Check whether state needs reset or not */
  if(resetStateFlag)
  {
    /* Clear the state buffer.  The size will be always 3 samples */
    memset(S->state, 0, 3u * sizeof(q31_t));
  }

}
