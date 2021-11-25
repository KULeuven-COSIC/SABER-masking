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

#include "verify_masking.h"

static uint8_t verify(const uint8_t *a, const uint8_t *b, size_t len)
{
    uint32_t r;
    size_t i;

    r = 0;
    for (i = 0; i < len; i++)
    {
        r |= a[i] ^ b[i];
    }

    r = (~r + 1); // Two's complement
    r >>= 31;
    return (uint8_t)r;
}

// Section 4.1.5
uint8_t verify_masked(shake128incctx shake_masked_compare[2])
{
    uint8_t hash[2][32];

    shake128_inc_finalize(&shake_masked_compare[0]);
    shake128_inc_finalize(&shake_masked_compare[1]);

    shake128_inc_squeeze(hash[0], 32, &shake_masked_compare[0]);
    shake128_inc_squeeze(hash[1], 32, &shake_masked_compare[1]);

    return verify(hash[0], hash[1], 32);
}

/* b = 1 means mov, b = 0 means don't mov and unmask*/
void cmov_masked(uint8_t r[SABER_SHARES][64], const uint8_t *x, size_t len, uint8_t b)
{
    size_t i;

    b = -b;
    for (i = 0; i < len; i++)
    {
        r[1][i] &= ~b;
    }

    for (i = 0; i < len; i++)
    {
        r[0][i] ^= (b & (x[i] ^ r[0][i])) | r[1][i];
    }
}
