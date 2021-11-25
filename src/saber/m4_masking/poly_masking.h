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
#ifndef POLY_MASKING_H
#define POLY_MASKING_H

#include "SABER_params_masking.h"
#include "fips202.h"
#include <stdint.h>

void GenSecret_masked(uint16_t s[SABER_SHARES][SABER_L][SABER_N], const uint8_t seed_s[SABER_SHARES][SABER_NOISE_SEEDBYTES]);
void MatrixVectorMulEnc_masked(const uint8_t ct0[SABER_POLYVECCOMPRESSEDBYTES], const uint8_t seed_A[SABER_SEEDBYTES], uint16_t sp[SABER_SHARES][SABER_L][SABER_N], shake128incctx shake_masking_ctx[2]);
void InnerProdEnc_masked(const uint8_t ct1[SABER_SCALEBYTES_KEM], const uint8_t pk[SABER_INDCPA_PUBLICKEYBYTES], uint16_t sp[SABER_SHARES][SABER_L][SABER_N], const uint8_t m[SABER_SHARES][SABER_KEYBYTES], shake128incctx shake_masking_ctx[2]);
void InnerProdDec_masked(uint8_t m[SABER_SHARES][SABER_KEYBYTES], const uint8_t ciphertext[SABER_BYTES_CCA_DEC], uint16_t s[SABER_SHARES][SABER_L][SABER_N]);

#endif
