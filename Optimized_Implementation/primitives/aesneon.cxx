/*
 * Copyright (C) 2018 Novacoin developers
 */

// g++ aes_assist_1_neon.cpp -mcpu=cortex-a53+crypto -std=c++11

#include <cstddef>
#include <iostream>
#include <iomanip>
#include <arm_neon.h>

using namespace std;

typedef int32x4_t __m128i;

  int32_t b0[4] {};
  int32_t b1[4] {0x0fffffff, 0x0bbbbbbb, 0x08888888, 0x04444444};

static __m128i assist256_1 (__m128i a, __m128i b) {
    __m128i c{};
    b = vdupq_laneq_s32(b, 3); // shuffle ( , 0xff or 3,3,3,3)
    c = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(a), 12)); // slli (12 = 16 - 4)
    a = veorq_s32(a, c); // xor
    c = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(c), 12));
    a = veorq_s32(a, c);
    c = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(c), 12));
    a = veorq_s32(a, c);
    return veorq_s32(a, b); // return a = veorq_s32(a, b);
}

static __m128i assist256_2 (__m128i a, __m128i c) {
    __m128i b{}, d{};

    d = (__m128i)vaeseq_u8((uint8x16_t)a, (uint8x16_t){});
    uint8x16_t d_tmp {(uint8x16_t)d}; //d
    uint8x16_t dest = {
        d_tmp[0x4], d_tmp[0x1], d_tmp[0xE], d_tmp[0xB],
        d_tmp[0x1], d_tmp[0xE], d_tmp[0xB], d_tmp[0x4],
        d_tmp[0xC], d_tmp[0x9], d_tmp[0x6], d_tmp[0x3],
        d_tmp[0x9], d_tmp[0x6], d_tmp[0x3], d_tmp[0xC]
    };
    d = (__m128i)dest; //d = dest ^ (__m128i)((uint32x4_t){0, rcon, 0, rcon}); drop xor - rcon == 0
    b = vdupq_laneq_s32(d, 2); // shuffle ( , 0xaa or 2,2,2,2)
    d = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(c), 12));
    c = veorq_s32(c, d);
    d = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(d), 12));
    c = veorq_s32(c, d);
    d = vreinterpretq_s32_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s32(d), 12));
    c = veorq_s32(c, d);
    return veorq_s32(c, b); // return c = veorq_s32(c, b);
}

static  __m128i aeskeygenassist (__m128i a, unsigned rcon) {

    a = (__m128i)vaeseq_u8((uint8x16_t)a, (uint8x16_t){});
    uint8x16_t d_tmp {(uint8x16_t)a};
    uint8x16_t dest = {
        d_tmp[0x4], d_tmp[0x1], d_tmp[0xE], d_tmp[0xB],
        d_tmp[0x1], d_tmp[0xE], d_tmp[0xB], d_tmp[0x4],
        d_tmp[0xC], d_tmp[0x9], d_tmp[0x6], d_tmp[0x3],
        d_tmp[0x9], d_tmp[0x6], d_tmp[0x3], d_tmp[0xC]
    };
    return (__m128i)(vreinterpretq_u32_u8(dest) ^ (uint32x4_t){0, rcon, 0, rcon});
}

void aes256_KeyExpansion_NI(__m128i* keyExp, const __m128i* userkey)
{
    __m128i temp1, temp2, temp3;

    temp1 = keyExp[0] = vld1q_s32((int32_t *)userkey);
    temp3 = keyExp[1] = vld1q_s32((int32_t *)(userkey+1));

    temp2 = aeskeygenassist(temp3, 0x01);
    temp1 = keyExp[2] = assist256_1(temp1, temp2);
    temp3 = keyExp[3] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x02);
    temp1 = keyExp[4] = assist256_1(temp1, temp2);
    temp3 = keyExp[5] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x04);
    temp1 = keyExp[6] = assist256_1(temp1, temp2);
    temp3 = keyExp[7] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x08);
    temp1 = keyExp[8] = assist256_1(temp1, temp2);
    temp3 = keyExp[9] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x10);
    temp1 = keyExp[10] = assist256_1(temp1, temp2);
    temp3 = keyExp[11] = assist256_2(temp1, temp3);

    temp2 = aeskeygenassist(temp3, 0x20);
    temp1 = keyExp[12] = assist256_1(temp1, temp2);
    temp3 = keyExp[13] = assist256_2(temp1, temp3);

    keyExp[14] = assist256_1(temp1, aeskeygenassist(temp3, 0x40));
}

static __m128i increment_be_soft(__m128i x) {
    // preparation
    int8_t a[16] {};
    int8_t *pv = (int8_t *)&x;
    a[0] = pv[0]; a[1] = pv[1]; a[2] = pv[2]; a[3] = pv[3];
    a[4] = pv[4]; a[5] = pv[5]; a[6] = pv[6]; a[7] = pv[7];
    a[8] = pv[8]; a[9] = pv[9]; a[10] = pv[10]; a[11] = pv[11];
    a[12] = pv[12]; a[13] = pv[13]; a[14] = pv[14]; a[15] = pv[15];

    int8_t mask[16] = {
    0x0f, 0x0e, 0x0d, 0x0c,
    0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04,
    0x03, 0x02, 0x01, 0x00
    };

    int8_t ret[16];

    //x = _mm_shuffle_epi8 (x, swap);

    for (int j = 0; j < 16; j++) {
    // ret[j] = (mask[0] & 0x80) ? 0 : a[(mask[j] & 0x0f)]; drop cheking against 0x80 see mask[] value
    ret[j] = a[(mask[j] & 0x0f)]; // bytes by revers order - becouse 0..15 against & 0x0f
    };

    //x = _mm_add_epi64 (x, _mm_set_epi32 (0, 0, 0, 1));
    int64_t ret_as_i64[2] {};
    int64_t *pret = (int64_t *)&ret;
    ret_as_i64[0] = (pret[0]) + 1;
    ret_as_i64[1] = (pret[1]) + 0;
    pv = (int8_t *)&ret_as_i64;
    a[0] = pv[0]; a[1] = pv[1]; a[2] = pv[2]; a[3] = pv[3];
    a[4] = pv[4]; a[5] = pv[5]; a[6] = pv[6]; a[7] = pv[7];
    a[8] = pv[8]; a[9] = pv[9]; a[10] = pv[10]; a[11] = pv[11];
    a[12] = pv[12]; a[13] = pv[13]; a[14] = pv[14]; a[15] = pv[15];

    //x = _mm_shuffle_epi8 (x, swap);
    for (int j = 0; j < 16; j++) {
    // ret[j] = (mask[0] & 0x80) ? 0 : a[(mask[j] & 0x0f)]; drop cheking against 0x80 see mask[] value
    ret[j] = a[(mask[j] & 0x0f)]; // bytes by revers order - becouse 0..15 against & 0x0f
    };

    __m128i* p128 = (__m128i *)&ret;
    return p128[0];
}

int main(const int argc, const char **argv)
{
    __m128i rkeys[15] {};
    uint8_t k[32] {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    aes256_KeyExpansion_NI(rkeys, (__m128i *)k);
    for ( auto& el : rkeys ){
    __m128i tmp = el;
    int32_t *vp = (int32_t *)&tmp;
    b0[0] = vp[0];
    b0[1] = vp[1];
    b0[2] = vp[2];
    b0[3] = vp[3];
    cout << right << hex << setw(10) << b0[0] << setw(10) << b0[1] << setw(10) << b0[2] << setw(10) << b0[3] << endl;
    }
    return 0;
}