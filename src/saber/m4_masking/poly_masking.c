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
#include "poly_masking.h"
#include "poly_mul.h"
#include "cbd_masking.h"
#include "fips202.h"
#include "fips202_masking.h"
#include "pack_unpack.h"
#include "pack_unpack_masking.h"
#include "A2B.h"

#define h1 (1 << (SABER_EQ - SABER_EP - 1))
#define h2 ((1 << (SABER_EP - 2)) - (1 << (SABER_EP - SABER_ET - 1)) + (1 << (SABER_EQ - SABER_EP - 1)))

extern void ClearStack(size_t len);

static inline shake128incctx shake128_absorb_seed(const uint8_t seed[SABER_SEEDBYTES])
{
    shake128incctx ctx;
    shake128_inc_init(&ctx);
    shake128_inc_absorb(&ctx, seed, SABER_SEEDBYTES);
    shake128_inc_finalize(&ctx);
    return ctx;
}

// noinline such that toom-cook can reclaim the memory of shake_out
static void __attribute__((noinline)) shake128_squeeze_poly(uint16_t poly[SABER_N], shake128incctx *ctx)
{
    uint8_t shake_out[SABER_POLYBYTES];

    shake128_inc_squeeze(shake_out, SABER_POLYBYTES, ctx);
    BS2POLq(shake_out, poly);
}

void GenSecret_masked(uint16_t s[SABER_SHARES][SABER_L][SABER_N], const uint8_t seed_sp[SABER_SHARES][SABER_NOISE_SEEDBYTES])
{
    uint8_t shake_out[SABER_SHARES][SABER_L * SABER_POLYCOINBYTES];

    shake128_masked(SABER_L * SABER_POLYCOINBYTES, shake_out, SABER_NOISE_SEEDBYTES, seed_sp);
    cbd_masked(s, shake_out);
}

void MatrixVectorMulEnc_masked(const uint8_t ct0[SABER_POLYVECCOMPRESSEDBYTES], const uint8_t seed_A[SABER_SEEDBYTES], uint16_t sp[SABER_SHARES][SABER_L][SABER_N], shake128incctx shake_masking_ctx[2])
{
    size_t i, j, k;
    uint16_t A[SABER_N];
    uint16_t bp[SABER_SHARES][SABER_N];
    uint8_t cmp[SABER_SHARES][SABER_POLYCOMPRESSEDBYTES];

    shake128incctx shake_A_ctx = shake128_absorb_seed(seed_A);

    for (i = 0; i < SABER_L; i++)
    {

        for (j = 0; j < SABER_L; j++)
        {

            shake128_squeeze_poly(A, &shake_A_ctx);

            for (k = 0; k < SABER_SHARES; k++)
            {
                ClearStack(3076); // Stack overwrites in TC could leak between the shares
                if (j == 0)
                {
                    poly_mul(sp[k][j], A, bp[k]);
                }
                else
                {
                    poly_mul_acc(sp[k][j], A, bp[k]);
                }
            }
        }

        for (j = 0; j < SABER_N; j++)
        {
            bp[0][j] += h1;
        }

        poly_A2A(bp, SABER_Q - 1, 1);

        POLp2BS_sub(cmp[0], &ct0[i * SABER_POLYCOMPRESSEDBYTES], bp[0]);
        POLp2BS(cmp[1], bp[1]);

        shake128_inc_absorb(&shake_masking_ctx[0], cmp[0], SABER_POLYCOMPRESSEDBYTES);
        shake128_inc_absorb(&shake_masking_ctx[1], cmp[1], SABER_POLYCOMPRESSEDBYTES);
    }
    shake128_inc_ctx_release(&shake_A_ctx);
}

void InnerProdEnc_masked(const uint8_t ct1[SABER_SCALEBYTES_KEM], const uint8_t pk[SABER_INDCPA_PUBLICKEYBYTES], uint16_t sp[SABER_SHARES][SABER_L][SABER_N], const uint8_t m[SABER_SHARES][SABER_KEYBYTES], shake128incctx shake_masking_ctx[2])
{
    size_t j, k;
    uint16_t mp[SABER_SHARES][SABER_N];
    uint16_t vp[SABER_SHARES][SABER_N];
    uint8_t cmp[SABER_SHARES][SABER_SCALEBYTES_KEM];
    uint16_t(*b) = mp[0];

    for (j = 0; j < SABER_L; j++)
    {

        BS2POLp(&pk[j * SABER_POLYCOMPRESSEDBYTES], b);

        for (k = 0; k < SABER_SHARES; k++)
        {
            ClearStack(3076); // Stack overwrites in TC could leak between the shares
            if (j == 0)
            {
                poly_mul(sp[k][j], b, vp[k]);
            }
            else
            {
                poly_mul_acc(sp[k][j], b, vp[k]);
            }
        }
    }

    for (k = 0; k < SABER_SHARES; k++)
    {
        BS2POLmsg(m[k], mp[k]);
        for (j = 0; j < SABER_N; j++)
        {
            vp[k][j] = vp[k][j] - (mp[k][j] << (SABER_EP - 1));
            if (k == 0)
            {
                vp[k][j] += h1;
            }
        }
    }

    poly_A2A(vp, SABER_P - 1, 2);

    POLT2BS_sub(cmp[0], ct1, vp[0]);
    POLT2BS(cmp[1], vp[1]);

    shake128_inc_absorb(&shake_masking_ctx[0], cmp[0], SABER_SCALEBYTES_KEM);
    shake128_inc_absorb(&shake_masking_ctx[1], cmp[1], SABER_SCALEBYTES_KEM);
}

void InnerProdDec_masked(uint8_t m[SABER_SHARES][SABER_KEYBYTES], const uint8_t ciphertext[SABER_BYTES_CCA_DEC], uint16_t s[SABER_SHARES][SABER_L][SABER_N])
{
    size_t i, j;
    uint16_t v[SABER_SHARES][SABER_N];
    uint16_t bp[SABER_N];
    uint16_t(*cm) = bp;

    for (i = 0; i < SABER_L; i++)
    {

        BS2POLp(&ciphertext[i * SABER_POLYCOMPRESSEDBYTES], bp);

        for (j = 0; j < SABER_SHARES; j++)
        {
            ClearStack(3076); // Stack overwrites in TC could leak between the shares
            if (i == 0)
            {
                poly_mul(s[j][i], bp, v[j]);
            }
            else
            {
                poly_mul_acc(s[j][i], bp, v[j]);
            }
        }
    }

    BS2POLT(ciphertext + SABER_POLYVECCOMPRESSEDBYTES, cm);

    for (i = 0; i < SABER_N; i++)
    {
        v[0][i] += h2 - (cm[i] << (SABER_EP - SABER_ET));
    }

    poly_A2A(v, SABER_P - 1, 3);

    for (i = 0; i < SABER_SHARES; i++)
    {
        POLmsg2BS(m[i], v[i]);
    }
}