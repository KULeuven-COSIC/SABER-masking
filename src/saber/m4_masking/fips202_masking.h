/*
 * MIT License
 *
 * Copyright (c) 2021: imec-COSIC KU Leuven, 3001 Leuven, Belgium 
 * Author: Michiel Van Beirendonck <michiel.vanbeirendonck@esat.kuleuven.be>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef FIPS202_MASKING_H
#define FIPS202_MASKING_H

#define NROUNDS 24

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136
#define SHA3_256_RATE 136
#define SHA3_512_RATE 72

#define ROL(a, offset) ((a << offset) ^ (a >> (64 - offset)))

#include "SABER_params_masking.h"
#include <stdint.h>
#include <stddef.h>

void shake128_masked(size_t outlen, uint8_t output[SABER_SHARES][outlen], size_t inlen, const uint8_t input[SABER_SHARES][inlen]);
void sha3_512_masked(uint8_t output[SABER_SHARES][64], size_t inlen, const uint8_t input[SABER_SHARES][inlen]);

#endif