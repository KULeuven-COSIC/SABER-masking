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

#include "cbd_masking.h"
#include "masksONOFF.h"

#include <string.h>

#define kappa 4
#define lambda 4

#ifdef SABER_MASKING_ASM
extern void SecBitAddBitSubConsAdd(uint32_t x_bit[2][kappa], uint32_t y_bit[2][kappa], uint32_t z_bit[2][lambda]);
#else

static void SecAnd_first_order(uint32_t *x0Ptr, uint32_t y0, uint32_t *x1Ptr, uint32_t y1)
{
    uint32_t r_01, r_10, x0, x1;

    x0 = *x0Ptr;
    x1 = *x1Ptr;

    r_01 = random_uint32();

    r_10 = (r_01 ^ (x0 & y1)) ^ (x1 & y0);
    x0 = x0 & y0;
    x0 = x0 ^ r_01;
    x1 = x1 & y1;
    x1 = x1 ^ r_10;

    *x0Ptr = x0;
    *x1Ptr = x1;
}

static void SecBitAdd(uint32_t x_bit[2][kappa], uint32_t z[2][lambda])
{

    size_t j, k, l;
    uint32_t w[2], u[2];

    //initialize
    memset(z, 0, 2 * lambda * sizeof(uint32_t));

    for (j = 0; j < kappa; j++)
    {

        u[0] = z[0][0];
        u[1] = z[1][0];

        w[0] = x_bit[0][j];
        w[1] = x_bit[1][j];

        z[0][0] = u[0] ^ w[0];
        z[1][0] = u[1] ^ w[1];

        l = 1;
        k = j + 1;

        while (k >>= 1)
        {

            SecAnd_first_order(&w[0], u[0], &w[1], u[1]);

            u[0] = z[0][l];
            u[1] = z[1][l];

            z[0][l] = z[0][l] ^ w[0];
            z[1][l] = z[1][l] ^ w[1];

            l++;
        }
    }
}

static void SecBitSub(uint32_t y_bit[2][kappa], uint32_t z[2][lambda])
{

    int32_t j, l;
    uint32_t w[2], u[2];

    for (j = 0; j < kappa; j++)
    {

        w[0] = y_bit[0][j];
        w[1] = y_bit[1][j];

        for (l = 0; l < lambda; l++)
        {

            u[0] = z[0][l];
            z[0][l] = u[0] ^ w[0];

            u[1] = z[1][l];
            z[1][l] = u[1] ^ w[1];
            u[1] = ~u[1];

            SecAnd_first_order(&w[0], u[0], &w[1], u[1]);
        }
    }
}

static void SecConsAdd(uint32_t y_bit[2][lambda]) //adds kappa //optimized for kappa=4
{

    y_bit[0][3] = y_bit[0][3] ^ y_bit[0][2];
    y_bit[0][2] = y_bit[0][2] ^ 0xffffffff;
    y_bit[1][3] = y_bit[1][3] ^ y_bit[1][2];
}

static void __attribute__((unused)) SecBitAdd_unrolled(uint32_t x_bit[2][kappa], uint32_t z[2][lambda])
{

    uint32_t w0, w1;
    uint32_t t0, t1;

    // j = 1
    t0 = x_bit[0][0];
    w0 = x_bit[0][1];
    z[0][0] = t0 ^ w0;

    t1 = x_bit[1][0];
    w1 = x_bit[1][1];
    z[1][0] = t1 ^ w1;

    SecAnd_first_order(&w0, t0, &w1, t1);

    // j = 2
    z[0][1] = w0;
    t0 = z[0][0];
    w0 = x_bit[0][2];
    z[0][0] = t0 ^ w0;

    z[1][1] = w1;
    t1 = z[1][0];
    w1 = x_bit[1][2];
    z[1][0] = t1 ^ w1;

    SecAnd_first_order(&w0, t0, &w1, t1);

    // j = 3
    z[0][1] = z[0][1] ^ w0;
    t0 = z[0][0];
    w0 = x_bit[0][3];
    z[0][0] = t0 ^ w0;

    z[1][1] = z[1][1] ^ w1;
    t1 = z[1][0];
    w1 = x_bit[1][3];
    z[1][0] = t1 ^ w1;

    SecAnd_first_order(&w0, t0, &w1, t1);

    t0 = z[0][1];
    z[0][1] = t0 ^ w0;

    t1 = z[1][1];
    z[1][1] = t1 ^ w1;

    SecAnd_first_order(&w0, t0, &w1, t1);

    z[0][2] = w0;
    z[0][3] = 0;

    z[1][2] = w1;
    z[1][3] = 0;
}

static void SecBitAddBitSubConsAdd(uint32_t x_bit[2][kappa], uint32_t y_bit[2][kappa], uint32_t z_bit[2][lambda])
{
    SecBitAdd(x_bit, z_bit);
    SecBitSub(y_bit, z_bit);
    SecConsAdd(z_bit);
}

#endif

static void __attribute__((noinline)) pack_bitslice(uint32_t x[kappa], uint32_t y[kappa], const uint8_t coins[SABER_POLYCOINBYTES], uint32_t scratch0[32], uint32_t scratch1[32])
{
    int i;
    uint32_t x_bit[kappa] = {0};
    uint32_t y_bit[kappa] = {0};

    for (i = 0; i < 32; i++)
    {
        scratch0[i] = (coins[i]) & 0x0f;
        scratch1[i] = (coins[i] >> 4) & 0x0f;
    }

    for (i = 0; i < 32; i++)
    {

        x_bit[0] = x_bit[0] | ((scratch0[i] & 0x01) << i);
        x_bit[1] = x_bit[1] | (((scratch0[i] >> 1) & 0x01) << i);
        x_bit[2] = x_bit[2] | (((scratch0[i] >> 2) & 0x01) << i);
        x_bit[3] = x_bit[3] | (((scratch0[i] >> 3) & 0x01) << i);

        y_bit[0] = y_bit[0] | ((scratch1[i] & 0x01) << i);
        y_bit[1] = y_bit[1] | (((scratch1[i] >> 1) & 0x01) << i);
        y_bit[2] = y_bit[2] | (((scratch1[i] >> 2) & 0x01) << i);
        y_bit[3] = y_bit[3] | (((scratch1[i] >> 3) & 0x01) << i);
    }

    memcpy(x, x_bit, kappa * sizeof(uint32_t));
    memcpy(y, y_bit, kappa * sizeof(uint32_t));
}

static void __attribute__((noinline)) unpack_bitslice(uint16_t r[32], uint32_t z_bit[lambda])
{

    int i;
    uint32_t z;

    for (i = 0; i < 32; i += 2)
    {
        //unpack lambda bits

        z = ((((z_bit[3] & 0x02) << 2) | ((z_bit[2] & 0x02) << 1) | ((z_bit[1] & 0x02) << 0) | ((z_bit[0] & 0x02) >> 1)) << 16) |
            ((z_bit[3] & 0x01) << 3) | ((z_bit[2] & 0x01) << 2) | ((z_bit[1] & 0x01) << 1) | ((z_bit[0] & 0x01));

        memcpy(r + i, &z, 4);

        z_bit[0] = z_bit[0] >> 2;
        z_bit[1] = z_bit[1] >> 2;
        z_bit[2] = z_bit[2] >> 2;
        z_bit[3] = z_bit[3] >> 2;
    }
}

#ifdef SABER_MASKING_ASM
extern void B2A_Goubin(uint16_t BPtr[2], uint16_t RPtr[2]);
#else

// #include <arm_acle.h>
static uint32_t __usub16(uint32_t a, uint32_t b)
{
    uint16_t tmp1, tmp2;
    tmp1 = (uint16_t)((a >> 16) & 0xffff) - (uint16_t)((b >> 16) & 0xffff);
    tmp2 = (uint16_t)(a & 0xffff) - (uint16_t)(b & 0xffff);
    return ((uint32_t)((tmp1 << 16) | tmp2));
}

static void B2A_Goubin(uint16_t BPtr[2], uint16_t RPtr[2]) // SIMD: converts 2 coefficients in parallel
{
    uint32_t B, R, T, A, y;

    memcpy(&B, BPtr, 4);
    memcpy(&R, RPtr, 4);

    y = random_uint32();
    T = __usub16(B, y);
    B = B ^ y;
    T = T ^ B;
    A = B ^ R;
    A = __usub16(A, R);
    A = A ^ T;

    R = R ^ y;

    // Extra step because of SecConsAdd, only for bitsliced sampler, not general B2A
    R = __usub16(R, ((kappa << 16) | kappa));

    memcpy(BPtr, &A, 4);
    memcpy(RPtr, &R, 4);
}
#endif

void cbd_masked(uint16_t s[SABER_SHARES][SABER_L][SABER_N], const uint8_t coins[SABER_SHARES][SABER_L * SABER_POLYCOINBYTES])
{
    int32_t i, j, k;
    uint32_t scratch0[32], scratch1[32];
    uint32_t x_bit[2][kappa], y_bit[2][kappa]; //individual bits
    uint32_t z_bit[2][lambda];                 //individual bits

    for (i = 0; i < SABER_L; i++)
    {
        for (j = 0; j < SABER_N / 32; j++)
        { //creates 32 samples at a time

            pack_bitslice(x_bit[0], y_bit[0], &coins[0][i * SABER_POLYCOINBYTES + 32 * j], scratch0, scratch1);
            pack_bitslice(x_bit[1], y_bit[1], &coins[1][i * SABER_POLYCOINBYTES + 32 * j], scratch1, scratch0);

            SecBitAddBitSubConsAdd(x_bit, y_bit, z_bit);

            unpack_bitslice(&s[0][i][32 * j], z_bit[0]);
            unpack_bitslice(&s[1][i][32 * j], z_bit[1]);

            for (k = 0; k < 32; k += 2)
            {
                B2A_Goubin(&s[0][i][32 * j + k], &s[1][i][32 * j + k]);
            }
        }
    }
}
