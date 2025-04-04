/* ---------------------------------------------------------------------- 
* Copyright (C) 2010-2014 ARM Limited. All rights reserved. 
* 
* $Date:        19. March 2015 
* $Revision: 	V.1.4.5
* 
* Project: 	    CMSIS DSP Library 
* Title:	    arm_const_structs.c 
* 
* Description:	This file has constant structs that are initialized for
*              user convenience.  For example, some can be given as 
*              arguments to the arm_cfft_f32() function.
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
* -------------------------------------------------------------------- */

#include "riscv_const_structs.h"

//Floating-point structs

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len16 = {
	16, twiddleCoef_16, riscvBitRevIndexTable16, RISCVBITREVINDEXTABLE__16_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len32 = {
	32, twiddleCoef_32, riscvBitRevIndexTable32, RISCVBITREVINDEXTABLE__32_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len64 = {
	64, twiddleCoef_64, riscvBitRevIndexTable64, RISCVBITREVINDEXTABLE__64_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len128 = {
	128, twiddleCoef_128, riscvBitRevIndexTable128, RISCVBITREVINDEXTABLE_128_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len256 = {
	256, twiddleCoef_256, riscvBitRevIndexTable256, RISCVBITREVINDEXTABLE_256_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len512 = {
	512, twiddleCoef_512, riscvBitRevIndexTable512, RISCVBITREVINDEXTABLE_512_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len1024 = {
	1024, twiddleCoef_1024, riscvBitRevIndexTable1024, RISCVBITREVINDEXTABLE1024_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len2048 = {
	2048, twiddleCoef_2048, riscvBitRevIndexTable2048, RISCVBITREVINDEXTABLE2048_TABLE_LENGTH
};

const riscv_cfft_instance_f32 riscv_cfft_sR_f32_len4096 = {
	4096, twiddleCoef_4096, riscvBitRevIndexTable4096, RISCVBITREVINDEXTABLE4096_TABLE_LENGTH
};

//Fixed-point structs

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len16 = {
	16, twiddleCoef_16_q31, riscvBitRevIndexTable_fixed_16, RISCVBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len32 = {
	32, twiddleCoef_32_q31, riscvBitRevIndexTable_fixed_32, RISCVBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len64 = {
	64, twiddleCoef_64_q31, riscvBitRevIndexTable_fixed_64, RISCVBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len128 = {
	128, twiddleCoef_128_q31, riscvBitRevIndexTable_fixed_128, RISCVBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len256 = {
	256, twiddleCoef_256_q31, riscvBitRevIndexTable_fixed_256, RISCVBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len512 = {
	512, twiddleCoef_512_q31, riscvBitRevIndexTable_fixed_512, RISCVBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len1024 = {
	1024, twiddleCoef_1024_q31, riscvBitRevIndexTable_fixed_1024, RISCVBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len2048 = {
	2048, twiddleCoef_2048_q31, riscvBitRevIndexTable_fixed_2048, RISCVBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const riscv_cfft_instance_q31 riscv_cfft_sR_q31_len4096 = {
	4096, twiddleCoef_4096_q31, riscvBitRevIndexTable_fixed_4096, RISCVBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};


const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len16 = {
	16, twiddleCoef_16_q15, riscvBitRevIndexTable_fixed_16, RISCVBITREVINDEXTABLE_FIXED___16_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len32 = {
	32, twiddleCoef_32_q15, riscvBitRevIndexTable_fixed_32, RISCVBITREVINDEXTABLE_FIXED___32_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len64 = {
	64, twiddleCoef_64_q15, riscvBitRevIndexTable_fixed_64, RISCVBITREVINDEXTABLE_FIXED___64_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len128 = {
	128, twiddleCoef_128_q15, riscvBitRevIndexTable_fixed_128, RISCVBITREVINDEXTABLE_FIXED__128_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len256 = {
	256, twiddleCoef_256_q15, riscvBitRevIndexTable_fixed_256, RISCVBITREVINDEXTABLE_FIXED__256_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len512 = {
	512, twiddleCoef_512_q15, riscvBitRevIndexTable_fixed_512, RISCVBITREVINDEXTABLE_FIXED__512_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len1024 = {
	1024, twiddleCoef_1024_q15, riscvBitRevIndexTable_fixed_1024, RISCVBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len2048 = {
	2048, twiddleCoef_2048_q15, riscvBitRevIndexTable_fixed_2048, RISCVBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

const riscv_cfft_instance_q15 riscv_cfft_sR_q15_len4096 = {
	4096, twiddleCoef_4096_q15, riscvBitRevIndexTable_fixed_4096, RISCVBITREVINDEXTABLE_FIXED_4096_TABLE_LENGTH
};
