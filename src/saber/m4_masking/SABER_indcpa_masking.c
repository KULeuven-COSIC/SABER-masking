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
#include "SABER_indcpa_masking.h"
// #include "randombytes.h"
#include "fips202.h"
#include "poly_masking.h"
#include "verify_masking.h"
#include <string.h>
#include <stdlib.h>

uint8_t indcpa_kem_enc_cmp_masked(const uint8_t m[SABER_SHARES][SABER_KEYBYTES], const uint8_t seed_sp[SABER_SHARES][SABER_NOISE_SEEDBYTES], const uint8_t pk[SABER_INDCPA_PUBLICKEYBYTES], const uint8_t ciphertext[SABER_BYTES_CCA_DEC])
{
    const uint8_t *seed_A = pk + SABER_POLYVECCOMPRESSEDBYTES;
    const uint8_t *ct0 = ciphertext;
    const uint8_t *ct1 = ciphertext + SABER_POLYVECCOMPRESSEDBYTES;
    uint16_t sp[SABER_SHARES][SABER_L][SABER_N];

    shake128incctx shake_masked_compare[2];
    shake128_inc_init(&shake_masked_compare[0]);
    shake128_inc_init(&shake_masked_compare[1]);

    GenSecret_masked(sp, seed_sp);
    MatrixVectorMulEnc_masked(ct0, seed_A, sp, shake_masked_compare); // verify(ct[0], Pack(Round(A*s')))
    InnerProdEnc_masked(ct1, pk, sp, m, shake_masked_compare);        // verify(ct[1], Pack(Round(b*s' + m')))

    return verify_masked(shake_masked_compare);
}

void indcpa_kem_dec_masked(uint16_t s[SABER_SHARES][SABER_L][SABER_N], const uint8_t ciphertext[SABER_BYTES_CCA_DEC], uint8_t m[SABER_SHARES][SABER_KEYBYTES])
{
    InnerProdDec_masked(m, ciphertext, s); // m <- Pack(Round(b'*s - cm))
}