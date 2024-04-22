#ifndef FXP_H
#define FXP_H

#include <stdbool.h>
#include <stdint.h>

#define FRACTIONAL_BITS 23
typedef int32_t fxp32;
typedef int64_t fxpMul;

fxp32 fxp32_fromFloat(float f);
float fxp32_toFloat(fxp32 x);
fxp32 fxp32_fromInt(int i);

fxp32 fxp32_mul(fxp32 a, fxp32 b);
fxpMul fxp32_mul64(fxp32 a, fxp32 b);

fxp32 fxp32_div(fxp32 a, fxp32 b);

fxp32 fxp32_sqrt(fxp32 a);

bool fxp32_close(fxp32 a, fxp32 b, float prec);

void assert_fxp32Close(fxp32 a, fxp32 b, float prec);

#endif // FXP_H