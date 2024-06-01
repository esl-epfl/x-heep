/*!***********************************************************************************
 * @file     fxp32.h
 * @author 	 Linus Crugnola	<linus.crugnola@epfl.ch>
 * @date     07.06.2024
 *
 * Description:
 *  This file contains definitions for fixed point arithmetic with 32 bits.
 *
 *************************************************************************************/

#ifndef FXP_H
#define FXP_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief the number of bits used for the fractional part
 */
#define FRACTIONAL_BITS 23

/**
 * @brief 32 bit and extended 64 bit fixed point number types
 */
typedef int32_t fxp32;
typedef int64_t fxpMul;

/**
 * @brief convert a float/integer to a fixed point number
 *
 * @param n the number to convert
 *
 * @return the fixed point number
 */
fxp32 fxp32_fromFloat(float n);
fxp32 fxp32_fromInt(int n);

/**
 * @brief convert a fixed point number to a float
 *
 * @param x the fixed point number to convert
 *
 * @return the float
 */
float fxp32_toFloat(fxp32 x);
float fxp32_fxpMulToFloat(fxpMul x);

/**
 * @brief multiply two fixed point numbers
 *
 * @param a the first number
 * @param b the second number
 *
 * @return the result of the multiplication in 32 or 64 bit format
 */
fxp32 fxp32_mul(fxp32 a, fxp32 b);
fxpMul fxp32_mul64(fxp32 a, fxp32 b);

/**
 * @brief divide two fixed point numbers
 *
 * @param a the dividend
 * @param b the divisor
 *
 * @return the result of the division in 32 bit format
 */
fxp32 fxp32_div(fxp32 a, fxp32 b);

/**
 * @brief compute the square of a fixed point number
 *
 * @param a the number to square
 *
 * @return the square of the number in 64 bit format
 */
fxpMul fxp32_pow2(fxp32 a);

/**
 * @brief compute the square root of a fixed point number
 *
 * @param a the number to compute the square root of
 *
 * @return the square root of the number in 32 bit format
 */
fxp32 fxp32_sqrt(fxp32 a);

/**
 * @brief check if two fixed point numbers are close
 *
 * @param a the first number
 * @param b the second number
 * @param prec the precision
 *
 * @return true if the numbers are less or equal than the precision apart
 */
bool fxp32_close(fxp32 a, fxp32 b, float prec);

#endif // FXP_H