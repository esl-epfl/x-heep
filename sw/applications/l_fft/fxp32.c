#include "fxp32.h"

fxp32 fxp32_fromFloat(float f) {
    return (fxp32)(f * (1 << FRACTIONAL_BITS));
}

float fxp32_toFloat(fxp32 x) {
    return (float)x / (1 << FRACTIONAL_BITS);
}

float fxp32_fxpMulToFloat(fxpMul x) {
    return (float)x / ((fxpMul)1 << FRACTIONAL_BITS);
}

fxp32 fxp32_fromInt(int i) {
    return i << FRACTIONAL_BITS;
}

fxp32 fxp32_mul(fxp32 a, fxp32 b) {
    return (fxp32)(((fxpMul)a * (fxpMul)b) >> FRACTIONAL_BITS);
}

fxpMul fxp32_mul64(fxp32 a, fxp32 b) {
    return ((fxpMul)a * (fxpMul)b) >> FRACTIONAL_BITS;
}

fxpMul fxp32_pow2(fxp32 a) {
    return ((fxpMul)a * (fxpMul)a) >> FRACTIONAL_BITS;
}

fxp32 fxp32_div(fxp32 a, fxp32 b) {
    return (fxp32)(((fxpMul)a << FRACTIONAL_BITS) / b);
}

bool fxp32_closefxp(fxp32 a, fxp32 b, fxp32 prec) {
    fxp32 diff = a - b;
    if (diff < 0)
        diff = -diff;
    return diff <= prec;
}

fxp32 fxp32_sqrt(fxp32 a) {
    fxp32 guess = a >> 1;
    fxp32 eps = fxp32_fromFloat(0.0001);
    while (fxp32_closefxp(guess, fxp32_div(a, guess), eps) == false) {
        guess = (guess + fxp32_div(a, guess)) >> 1;
    }
    return guess;
}

bool fxp32_close(fxp32 a, fxp32 b, float prec) {
    fxp32 diff = a - b;
    if (diff < 0)
        diff = -diff;
    return diff <= fxp32_fromFloat(prec);
}
