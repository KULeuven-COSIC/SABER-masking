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

#include "fips202_masking.h"
#include "string.h"

#ifdef PROFILE_HASHING
#include "hal.h"
extern unsigned long long hash_cycles;
#endif

static void __attribute__((noinline)) memxor(void *dest, const void *src, size_t len)
{
    char *d = dest;
    const char *s = src;
    while (len--)
    {
        *d++ ^= *s++;
    }
}

const int keccakf_piln[24] = {
    10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1};

const int keccakf_rotc[24] = {
    1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14,
    27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44};

// Theta, Rho , Pi functions
static void __attribute__((noinline)) sha3_theta_rho_pi(uint64_t st[25], uint64_t bc[5])
{

    int i, j;
    uint64_t t;

    for (i = 0; i < 5; i++)
    {
        bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
    }

    for (i = 0; i < 5; i++)
    {
        t = bc[(i + 4) % 5] ^ ROL(bc[(i + 1) % 5], 1);
        for (j = 0; j < 25; j += 5)
        {
            st[j + i] ^= t;
        }
    }

    t = st[1];
    for (i = 0; i < 24; i++)
    {
        j = keccakf_piln[i];
        bc[0] = st[j];
        st[j] = ROL(t, keccakf_rotc[i]);
        t = bc[0];
    }
}

#ifdef SABER_MASKING_ASM
extern void sha3_chi_masked_nonlinear(uint64_t st1[25], uint64_t st2[25], uint64_t bc1[5], uint64_t bc2[5]);
#endif

static void __attribute__((noinline)) sha3_chi_masked(uint64_t st1[25], uint64_t st2[25], uint64_t bc1[5], uint64_t bc2[5])
{

    int j;

    for (j = 0; j < 25; j += 5)
    {

        memcpy(bc1, st1 + j, 5 * 8);
        memcpy(bc2, st2 + j, 5 * 8);

#ifdef SABER_MASKING_ASM
        sha3_chi_masked_nonlinear(st1 + j, st2 + j, bc1, bc2);
#else
        for (int i = 0; i < 5; i++)
        {
            st1[j + i] ^= ((~bc1[(i + 1) % 5]) & bc1[(i + 2) % 5]);
            st1[j + i] ^= ((bc1[(i + 1) % 5]) & bc2[(i + 2) % 5]);
            st2[j + i] ^= ((~bc2[(i + 1) % 5]) & bc2[(i + 2) % 5]);
            st2[j + i] ^= ((bc2[(i + 1) % 5]) & bc1[(i + 2) % 5]);
        }
#endif
    }
}

static void __attribute__((noinline)) KeccakF1600_StatePermute_masked(uint64_t st1[25], uint64_t st2[25])
{

    // variables
    int r;
    uint64_t bc1[5], bc2[5];

    // constants
    const uint64_t keccakf_rndc[24] = {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
        0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
        0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
        0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
        0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
        0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
        0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
        0x8000000000008080, 0x0000000080000001, 0x8000000080008008};

    // actual iteration
    for (r = 0; r < NROUNDS; r++)
    {

        // Theta Rho Pi
        sha3_theta_rho_pi(st1, bc1);
        sha3_theta_rho_pi(st2, bc2);

        //  Chi
        sha3_chi_masked(st1, st2, bc1, bc2);

        //  Iota
        st1[0] ^= keccakf_rndc[r];
    }
}

static void __attribute__((noinline)) keccak_absorb_masked(uint64_t s[SABER_SHARES][25], uint32_t r, size_t mlen, const uint8_t m[SABER_SHARES][mlen], uint8_t p)
{
    size_t l = 0;

    while (mlen >= r)
    {

        memxor(s[0], &m[0][l], r);
        memxor(s[1], &m[1][l], r);
        KeccakF1600_StatePermute_masked(s[0], s[1]);
        mlen -= r;
        l += r;
    }

    memxor(s[0], &m[0][l], mlen);
    // padding
    ((uint8_t *)s[0])[mlen] ^= p;
    ((uint8_t *)s[0])[r - 1] ^= 128;

    memxor(s[1], &m[1][l], mlen);
}

static void __attribute__((noinline)) keccak_squeezeblocks_masked(uint32_t r, size_t nblocks, size_t tlen, uint8_t t[SABER_SHARES][tlen], uint64_t s[SABER_SHARES][25])
{

    size_t l = 0;

    while (nblocks > 0)
    {

        KeccakF1600_StatePermute_masked(s[0], s[1]);
        memcpy(&t[0][l], s[0], r);
        memcpy(&t[1][l], s[1], r);

        l += r;
        nblocks--;
    }
}

void shake128_masked(size_t outlen, uint8_t output[SABER_SHARES][outlen], size_t inlen, const uint8_t input[SABER_SHARES][inlen])
{
#ifdef PROFILE_HASHING
    uint64_t time0 = hal_get_time();
#endif
    size_t i, l;
    uint64_t s[SABER_SHARES][25] = {0};
    unsigned char t[SABER_SHARES][SHAKE128_RATE];
    unsigned long long nblocks = outlen / SHAKE128_RATE;

    /* Absorb input */
    keccak_absorb_masked(s, SHAKE128_RATE, inlen, input, 0x1F);

    /* Squeeze output */
    keccak_squeezeblocks_masked(SHAKE128_RATE, nblocks, outlen, output, s);

    l = nblocks * SHAKE128_RATE;
    outlen -= nblocks * SHAKE128_RATE;

    if (outlen)
    {
        keccak_squeezeblocks_masked(SHAKE128_RATE, 1, SHAKE128_RATE, t, s);

        for (i = 0; i < SABER_SHARES; i++)
        {
            memcpy(&output[i][l], t[i], outlen);
        }
    }

#ifdef PROFILE_HASHING
    uint64_t time1 = hal_get_time();
    hash_cycles += (time1 - time0);
#endif
}

void sha3_512_masked(uint8_t output[SABER_SHARES][64], size_t inlen, const uint8_t input[SABER_SHARES][inlen])
{
#ifdef PROFILE_HASHING
    uint64_t time0 = hal_get_time();
#endif

    size_t i;
    uint64_t s[SABER_SHARES][25] = {0};
    uint8_t t[SABER_SHARES][SHA3_512_RATE];

    /* Absorb input */
    keccak_absorb_masked(s, SHA3_512_RATE, inlen, input, 0x06);

    /* Squeeze output */
    keccak_squeezeblocks_masked(SHA3_512_RATE, 1, SHA3_512_RATE, t, s);

    for (i = 0; i < SABER_SHARES; i++)
    {
        memcpy(output[i], t[i], 64);
    }

#ifdef PROFILE_HASHING
    uint64_t time1 = hal_get_time();
    hash_cycles += (time1 - time0);
#endif
}
