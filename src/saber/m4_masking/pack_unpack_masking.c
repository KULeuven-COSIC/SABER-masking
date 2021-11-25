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

#include "pack_unpack_masking.h"
#include <stddef.h>

/* This function reduces its input mod p */
void POLp2BS_sub(uint8_t cmp[SABER_POLYCOMPRESSEDBYTES], const uint8_t bytes[SABER_POLYCOMPRESSEDBYTES], const uint16_t data[SABER_N])
{
    size_t j;
    const uint16_t *indata = data;
    const uint8_t *inbytes = bytes;
    uint8_t *out = cmp;
    uint16_t tmp[4];

    for (j = 0; j < SABER_N / 4; j++)
    {
        tmp[0] = ((inbytes[0]) | (inbytes[1] << 8));
        tmp[1] = ((inbytes[1] >> 2) | (inbytes[2] << 6));
        tmp[2] = ((inbytes[2] >> 4) | (inbytes[3] << 4));
        tmp[3] = ((inbytes[3] >> 6) | (inbytes[4] << 2));

#ifdef SABER_MASKING_A2A
        tmp[0] -= indata[0];
        tmp[1] -= indata[1];
        tmp[2] -= indata[2];
        tmp[3] -= indata[3];
#else
        tmp[0] ^= indata[0];
        tmp[1] ^= indata[1];
        tmp[2] ^= indata[2];
        tmp[3] ^= indata[3];
#endif

        out[0] = (uint8_t)(tmp[0]);
        out[1] = (uint8_t)(((tmp[0] >> 8) & 0x03) | (tmp[1] << 2));
        out[2] = (uint8_t)(((tmp[1] >> 6) & 0x0f) | (tmp[2] << 4));
        out[3] = (uint8_t)(((tmp[2] >> 4) & 0x3f) | (tmp[3] << 6));
        out[4] = (uint8_t)(tmp[3] >> 2);

        indata += 4;
        inbytes += 5;
        out += 5;
    }
}

/* This function reduces its input mod p */
void POLT2BS_sub(uint8_t cmp[SABER_SCALEBYTES_KEM], const uint8_t bytes[SABER_SCALEBYTES_KEM], const uint16_t data[SABER_N])
{
    size_t j;
    const uint16_t *indata = data;
    const uint8_t *inbytes = bytes;
    uint8_t *out = cmp;
    uint16_t tmp[2];

    for (j = 0; j < SABER_N / 2; j++)
    {

        tmp[0] = (inbytes[0]);
        tmp[1] = (inbytes[0] >> 4);

#ifdef SABER_MASKING_A2A
        tmp[0] -= indata[0];
        tmp[1] -= indata[1];
#else
        tmp[0] ^= indata[0];
        tmp[1] ^= indata[1];
#endif

        out[0] = (uint8_t)((tmp[0] & 0x0f) | (tmp[1] << 4));

        indata += 2;
        inbytes += 1;
        out += 1;
    }
}