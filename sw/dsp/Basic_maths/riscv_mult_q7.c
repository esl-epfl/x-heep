/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mult_q7.c
 * Description:  Q7 vector multiplication
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 Modifications 2017  Mostafa Saleh       (Ported to RISC-V PULPino)
 Modifications 2024  ESL
* -------------------------------------------------------------------- */

#include "riscv_math.h"
#include "x_heep_emul.h"
/**
 * @ingroup groupMath
 */

/**
 * @addtogroup BasicMult
 * @{
 */

/**
 * @brief           Q7 vector multiplication
 * @param[in]       *pSrcA points to the first input vector
 * @param[in]       *pSrcB points to the second input vector
 * @param[out]      *pDst points to the output vector
 * @param[in]       blockSize number of samples in each vector
 * @return none.
 *
 * <b>Scaling and Overflow Behavior:</b>
 * \par
 * The function uses saturating arithmetic.
 * Results outside of the allowable Q7 range [0x80 0x7F] will be saturated.
 */

void riscv_mult_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  q7_t * pDst,
  uint32_t blockSize)
{
    uint32_t blkCnt;                               /* loop counters */

    #if defined (USE_DSP_RISCV)

    q15_t out1;                   /* Temporary variables to store the product */
    blkCnt = blockSize;
    while (blkCnt > 0u)
    {
        /* C = A * B */
        /*multiply and normailze then accumulate*/
    // out1 =  mulsN(*pSrcA++, *pSrcB++,7);// (((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7)
        q15_t x=(q15_t)(*pSrcA++) * (*pSrcB++)>>7;
        int precision=8;
        *pDst++ =  (q7_t)x_heep_clip(x,precision);
        /* Decrement the blockSize loop counter */
        blkCnt--;
    }
    #else

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;




    while (blkCnt > 0u)
    {
        /* C = A * B */
        /* Multiply the inputs and store the result in the destination buffer */
        *pDst++ = (q7_t) __SSAT((((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7), 8);

        /* Decrement the blockSize loop counter */
        blkCnt--;
    }
    #endif
}

/**
 * @} end of BasicMult group
 */

