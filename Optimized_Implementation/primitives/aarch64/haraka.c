/*
 *  Alexey Demidov
 *  Radius Group, LLC
 *  balthazar@yandex.ru
 *
 *  Microsoft Reference Source License (Ms-RSL)
 */

// clang++ harakaneon.cpp -mcpu=cortex-a53+simd+crypto -std=c++11 -O3

#include <stddef.h>
#include <arm_neon.h>

static const uint8x16_t zero8x16 = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

static const uint8x16_t rc8x16[48] = {
    {0x9d, 0x7b, 0x81, 0x75, 0xf0, 0xfe, 0xc5, 0xb2, 0x0a, 0xc0, 0x20, 0xe6, 0x4c, 0x70, 0x84, 0x06},
    {0x17, 0xf7, 0x08, 0x2f, 0xa4, 0x6b, 0x0f, 0x64, 0x6b, 0xa0, 0xf3, 0x88, 0xe1, 0xb4, 0x66, 0x8b},
    {0x14, 0x91, 0x02, 0x9f, 0x60, 0x9d, 0x02, 0xcf, 0x98, 0x84, 0xf2, 0x53, 0x2d, 0xde, 0x02, 0x34},
    {0x79, 0x4f, 0x5b, 0xfd, 0xaf, 0xbc, 0xf3, 0xbb, 0x08, 0x4f, 0x7b, 0x2e, 0xe6, 0xea, 0xd6, 0x0e},
    {0x44, 0x70, 0x39, 0xbe, 0x1c, 0xcd, 0xee, 0x79, 0x8b, 0x44, 0x72, 0x48, 0xcb, 0xb0, 0xcf, 0xcb},
    {0x7b, 0x05, 0x8a, 0x2b, 0xed, 0x35, 0x53, 0x8d, 0xb7, 0x32, 0x90, 0x6e, 0xee, 0xcd, 0xea, 0x7e},
    {0x1b, 0xef, 0x4f, 0xda, 0x61, 0x27, 0x41, 0xe2, 0xd0, 0x7c, 0x2e, 0x5e, 0x43, 0x8f, 0xc2, 0x67},
    {0x3b, 0x0b, 0xc7, 0x1f, 0xe2, 0xfd, 0x5f, 0x67, 0x07, 0xcc, 0xca, 0xaf, 0xb0, 0xd9, 0x24, 0x29},
    {0xee, 0x65, 0xd4, 0xb9, 0xca, 0x8f, 0xdb, 0xec, 0xe9, 0x7f, 0x86, 0xe6, 0xf1, 0x63, 0x4d, 0xab},
    {0x33, 0x7e, 0x03, 0xad, 0x4f, 0x40, 0x2a, 0x5b, 0x64, 0xcd, 0xb7, 0xd4, 0x84, 0xbf, 0x30, 0x1c},
    {0x00, 0x98, 0xf6, 0x8d, 0x2e, 0x8b, 0x02, 0x69, 0xbf, 0x23, 0x17, 0x94, 0xb9, 0x0b, 0xcc, 0xb2},
    {0x8a, 0x2d, 0x9d, 0x5c, 0xc8, 0x9e, 0xaa, 0x4a, 0x72, 0x55, 0x6f, 0xde, 0xa6, 0x78, 0x04, 0xfa},
    {0xd4, 0x9f, 0x12, 0x29, 0x2e, 0x4f, 0xfa, 0x0e, 0x12, 0x2a, 0x77, 0x6b, 0x2b, 0x9f, 0xb4, 0xdf},
    {0xee, 0x12, 0x6a, 0xbb, 0xae, 0x11, 0xd6, 0x32, 0x36, 0xa2, 0x49, 0xf4, 0x44, 0x03, 0xa1, 0x1e},
    {0xa6, 0xec, 0xa8, 0x9c, 0xc9, 0x00, 0x96, 0x5f, 0x84, 0x00, 0x05, 0x4b, 0x88, 0x49, 0x04, 0xaf},
    {0xec, 0x93, 0xe5, 0x27, 0xe3, 0xc7, 0xa2, 0x78, 0x4f, 0x9c, 0x19, 0x9d, 0xd8, 0x5e, 0x02, 0x21},
    {0x73, 0x01, 0xd4, 0x82, 0xcd, 0x2e, 0x28, 0xb9, 0xb7, 0xc9, 0x59, 0xa7, 0xf8, 0xaa, 0x3a, 0xbf},
    {0x6b, 0x7d, 0x30, 0x10, 0xd9, 0xef, 0xf2, 0x37, 0x17, 0xb0, 0x86, 0x61, 0x0d, 0x70, 0x60, 0x62},
    {0xc6, 0x9a, 0xfc, 0xf6, 0x53, 0x91, 0xc2, 0x81, 0x43, 0x04, 0x30, 0x21, 0xc2, 0x45, 0xca, 0x5a},
    {0x3a, 0x94, 0xd1, 0x36, 0xe8, 0x92, 0xaf, 0x2c, 0xbb, 0x68, 0x6b, 0x22, 0x3c, 0x97, 0x23, 0x92},
    {0xb4, 0x71, 0x10, 0xe5, 0x58, 0xb9, 0xba, 0x6c, 0xeb, 0x86, 0x58, 0x22, 0x38, 0x92, 0xbf, 0xd3},
    {0x8d, 0x12, 0xe1, 0x24, 0xdd, 0xfd, 0x3d, 0x93, 0x77, 0xc6, 0xf0, 0xae, 0xe5, 0x3c, 0x86, 0xdb},
    {0xb1, 0x12, 0x22, 0xcb, 0xe3, 0x8d, 0xe4, 0x83, 0x9c, 0xa0, 0xeb, 0xff, 0x68, 0x62, 0x60, 0xbb},
    {0x7d, 0xf7, 0x2b, 0xc7, 0x4e, 0x1a, 0xb9, 0x2d, 0x9c, 0xd1, 0xe4, 0xe2, 0xdc, 0xd3, 0x4b, 0x73},
    {0x4e, 0x92, 0xb3, 0x2c, 0xc4, 0x15, 0x14, 0x4b, 0x43, 0x1b, 0x30, 0x61, 0xc3, 0x47, 0xbb, 0x43},
    {0x99, 0x68, 0xeb, 0x16, 0xdd, 0x31, 0xb2, 0x03, 0xf6, 0xef, 0x07, 0xe7, 0xa8, 0x75, 0xa7, 0xdb},
    {0x2c, 0x47, 0xca, 0x7e, 0x02, 0x23, 0x5e, 0x8e, 0x77, 0x59, 0x75, 0x3c, 0x4b, 0x61, 0xf3, 0x6d},
    {0xf9, 0x17, 0x86, 0xb8, 0xb9, 0xe5, 0x1b, 0x6d, 0x77, 0x7d, 0xde, 0xd6, 0x17, 0x5a, 0xa7, 0xcd},
    {0x5d, 0xee, 0x46, 0xa9, 0x9d, 0x06, 0x6c, 0x9d, 0xaa, 0xe9, 0xa8, 0x6b, 0xf0, 0x43, 0x6b, 0xec},
    {0xc1, 0x27, 0xf3, 0x3b, 0x59, 0x11, 0x53, 0xa2, 0x2b, 0x33, 0x57, 0xf9, 0x50, 0x69, 0x1e, 0xcb},
    {0xd9, 0xd0, 0x0e, 0x60, 0x53, 0x03, 0xed, 0xe4, 0x9c, 0x61, 0xda, 0x00, 0x75, 0x0c, 0xee, 0x2c},
    {0x50, 0xa3, 0xa4, 0x63, 0xbc, 0xba, 0xbb, 0x80, 0xab, 0x0c, 0xe9, 0x96, 0xa1, 0xa5, 0xb1, 0xf0},
    {0x39, 0xca, 0x8d, 0x93, 0x30, 0xde, 0x0d, 0xab, 0x88, 0x29, 0x96, 0x5e, 0x02, 0xb1, 0x3d, 0xae},
    {0x42, 0xb4, 0x75, 0x2e, 0xa8, 0xf3, 0x14, 0x88, 0x0b, 0xa4, 0x54, 0xd5, 0x38, 0x8f, 0xbb, 0x17},
    {0xf6, 0x16, 0x0a, 0x36, 0x79, 0xb7, 0xb6, 0xae, 0xd7, 0x7f, 0x42, 0x5f, 0x5b, 0x8a, 0xbb, 0x34},
    {0xde, 0xaf, 0xba, 0xff, 0x18, 0x59, 0xce, 0x43, 0x38, 0x54, 0xe5, 0xcb, 0x41, 0x52, 0xf6, 0x26},
    {0x78, 0xc9, 0x9e, 0x83, 0xf7, 0x9c, 0xca, 0xa2, 0x6a, 0x02, 0xf3, 0xb9, 0x54, 0x9a, 0xe9, 0x4c},
    {0x35, 0x12, 0x90, 0x22, 0x28, 0x6e, 0xc0, 0x40, 0xbe, 0xf7, 0xdf, 0x1b, 0x1a, 0xa5, 0x51, 0xae},
    {0xcf, 0x59, 0xa6, 0x48, 0x0f, 0xbc, 0x73, 0xc1, 0x2b, 0xd2, 0x7e, 0xba, 0x3c, 0x61, 0xc1, 0xa0},
    {0xa1, 0x9d, 0xc5, 0xe9, 0xfd, 0xbd, 0xd6, 0x4a, 0x88, 0x82, 0x28, 0x02, 0x03, 0xcc, 0x6a, 0x75},
    {0xd2, 0x2a, 0x8f, 0x84, 0x78, 0x47, 0x7e, 0x36, 0x1e, 0xd3, 0xe7, 0x0d, 0x38, 0x72, 0xf3, 0x2f},
    {0x8f, 0xd5, 0x3b, 0xb7, 0x35, 0xb1, 0x36, 0xee, 0x8b, 0xbe, 0x74, 0xcf, 0x6a, 0x5c, 0xd9, 0x08},
    {0x4a, 0x3e, 0x74, 0xa3, 0x38, 0x18, 0xae, 0x66, 0x98, 0xee, 0xd6, 0xc9, 0x34, 0xf4, 0x80, 0x58},
    {0xbd, 0x69, 0x93, 0x9a, 0xc7, 0xf4, 0xfd, 0xd0, 0x99, 0xbd, 0xfa, 0xae, 0xf0, 0x23, 0x30, 0x59},
    {0x2a, 0xcb, 0x1e, 0x6f, 0x7b, 0x63, 0xcc, 0xa5, 0x6f, 0x6e, 0x60, 0xeb, 0xd1, 0xe3, 0x9a, 0x32},
    {0xab, 0x94, 0x75, 0xcb, 0xd6, 0x93, 0xdc, 0xa4, 0x94, 0x15, 0xe0, 0x49, 0xeb, 0x07, 0x02, 0xe0},
    {0xf8, 0x8e, 0x20, 0x65, 0xa6, 0x66, 0x23, 0x94, 0x80, 0xc8, 0x51, 0xf7, 0x4f, 0x0c, 0xaa, 0x1c},
    {0x4a, 0x7e, 0xe6, 0xe3, 0x9f, 0x23, 0x03, 0xbd, 0xdd, 0xc1, 0x2d, 0xdb, 0x7f, 0xf5, 0xf7, 0x02}};

//#define HASH_SIZE 32
//
//struct hash {
//    uint8_t h[HASH_SIZE];
//} __attribute__ ((aligned (16)));

#define u128 uint8x16_t

// haraka_f 6 rounds
void haraka256_256(unsigned char *out, const unsigned char *in) {
  u128 s[2], tmp, s_save[2];

  s_save[0] = s[0] = vld1q_u8(in);
  s_save[1] = s[1] = vld1q_u8(in + 16);

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[0];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[1];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[2];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[3];

  tmp = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = tmp;

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[4];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[5];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[6];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[7];

  tmp = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = tmp;

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[8];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[9];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[10];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[11];

  tmp = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = tmp;

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[12];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[13];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[14];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[15];

  tmp = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = tmp;

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[16];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[17];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[18];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[19];

  tmp = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = tmp;

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[20];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[21];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[22];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[23];

  tmp = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = tmp;

  s[0] = veorq_u8(s[0], s_save[0]);
  s[1] = veorq_u8(s[1], s_save[1]);

  vst1q_u8(out, s[0]);
  vst1q_u8(out + 16, s[1]);
}

void haraka256_256_chain(unsigned char *out, const unsigned char *in, int chainlen) {
  uint8x16_t s[2], tmp, s_save[2];
  int cnt;

  s_save[0] = s[0] = vld1q_u8(in);
  s_save[1] = s[1] = vld1q_u8(in + 16);

  for (cnt = 0; cnt < chainlen; cnt++) {
      for (int i = 0; i < 6; i++) {
         s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[(i+0)];
         s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[(i+1)];
         s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[(i+2)];
         s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[(i+3)];

         tmp = (uint8x16_t) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
         s[0] = (uint8x16_t) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
        s[1] = tmp;
      };

      s[0] = s_save[0] = veorq_u8(s[0], s_save[0]);
      s[1] = s_save[1] = veorq_u8(s[1], s_save[1]);
  }

  vst1q_u8(out, s[0]);
  vst1q_u8(out + 16, s[1]);
}

void haraka256_256_4x(unsigned char *out, const unsigned char *in) {
  u128 s[4][2], tmp;

  s[0][0] = vld1q_u8(in);
  s[0][1] = vld1q_u8(in + 16);
  s[1][0] = vld1q_u8(in + 32);
  s[1][1] = vld1q_u8(in + 48);
  s[2][0] = vld1q_u8(in + 64);
  s[2][1] = vld1q_u8(in + 80);
  s[3][0] = vld1q_u8(in + 96);
  s[3][1] = vld1q_u8(in + 112);


  for (unsigned idx = 0; idx < 6; ++idx) {
  s[0][0] = vaesmcq_u8(vaeseq_u8(s[0][0], zero8x16)) ^ rc8x16[4 * idx + 0];
  s[0][1] = vaesmcq_u8(vaeseq_u8(s[0][1], zero8x16)) ^ rc8x16[4 * idx + 1];
  s[0][0] = vaesmcq_u8(vaeseq_u8(s[0][0], zero8x16)) ^ rc8x16[4 * idx + 2];
  s[0][1] = vaesmcq_u8(vaeseq_u8(s[0][1], zero8x16)) ^ rc8x16[4 * idx + 3];
  s[1][0] = vaesmcq_u8(vaeseq_u8(s[1][0], zero8x16)) ^ rc8x16[4 * idx + 0];
  s[1][1] = vaesmcq_u8(vaeseq_u8(s[1][1], zero8x16)) ^ rc8x16[4 * idx + 1];
  s[1][0] = vaesmcq_u8(vaeseq_u8(s[1][0], zero8x16)) ^ rc8x16[4 * idx + 2];
  s[1][1] = vaesmcq_u8(vaeseq_u8(s[1][1], zero8x16)) ^ rc8x16[4 * idx + 3];
  s[2][0] = vaesmcq_u8(vaeseq_u8(s[2][0], zero8x16)) ^ rc8x16[4 * idx + 0];
  s[2][1] = vaesmcq_u8(vaeseq_u8(s[2][1], zero8x16)) ^ rc8x16[4 * idx + 1];
  s[2][0] = vaesmcq_u8(vaeseq_u8(s[2][0], zero8x16)) ^ rc8x16[4 * idx + 2];
  s[2][1] = vaesmcq_u8(vaeseq_u8(s[2][1], zero8x16)) ^ rc8x16[4 * idx + 3];
  s[3][0] = vaesmcq_u8(vaeseq_u8(s[3][0], zero8x16)) ^ rc8x16[4 * idx + 0];
  s[3][1] = vaesmcq_u8(vaeseq_u8(s[3][1], zero8x16)) ^ rc8x16[4 * idx + 1];
  s[3][0] = vaesmcq_u8(vaeseq_u8(s[3][0], zero8x16)) ^ rc8x16[4 * idx + 2];
  s[3][1] = vaesmcq_u8(vaeseq_u8(s[3][1], zero8x16)) ^ rc8x16[4 * idx + 3];

  tmp = (u128) vzip2q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][1]);
  s[0][0] = (u128) vzip1q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][1]);
  s[0][1] = tmp;
  tmp = (u128) vzip2q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][1]);
  s[1][0] = (u128) vzip1q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][1]);
  s[1][1] = tmp;
  tmp = (u128) vzip2q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][1]);
  s[2][0] = (u128) vzip1q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][1]);
  s[2][1] = tmp;
  tmp = (u128) vzip2q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][1]);
  s[3][0] = (u128) vzip1q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][1]);
  s[3][1] = tmp;
  };

  s[0][0] = veorq_u8(s[0][0], vld1q_u8(in));
  s[0][1] = veorq_u8(s[0][1], vld1q_u8(in + 16));
  s[1][0] = veorq_u8(s[1][0], vld1q_u8(in + 32));
  s[1][1] = veorq_u8(s[1][1], vld1q_u8(in + 48));
  s[2][0] = veorq_u8(s[2][0], vld1q_u8(in + 64));
  s[2][1] = veorq_u8(s[2][1], vld1q_u8(in + 80));
  s[3][0] = veorq_u8(s[3][0], vld1q_u8(in + 96));
  s[3][1] = veorq_u8(s[3][1], vld1q_u8(in + 112));

  vst1q_u8(out, s[0][0]);
  vst1q_u8(out + 16, s[0][1]);
  vst1q_u8(out + 32, s[1][0]);
  vst1q_u8(out + 48, s[1][1]);
  vst1q_u8(out + 64, s[2][0]);
  vst1q_u8(out + 80, s[2][1]);
  vst1q_u8(out + 96, s[3][0]);
  vst1q_u8(out + 112, s[3][1]);
}

void haraka256_256_4x_chain(unsigned char *out, const unsigned char *in, int chainlen) {
    u128 s[4][2], tmp;
    int ctr;

    s[0][0] = vld1q_u8(in);
    s[0][1] = vld1q_u8(in + 16);
    s[1][0] = vld1q_u8(in + 32);
    s[1][1] = vld1q_u8(in + 48);
    s[2][0] = vld1q_u8(in + 64);
    s[2][1] = vld1q_u8(in + 80);
    s[3][0] = vld1q_u8(in + 96);
    s[3][1] = vld1q_u8(in + 112);

  for (ctr = 0; ctr < chainlen; ctr++) {
      /* Rounds */
      for (unsigned idx = 0; idx < 6; ++idx) {
      s[0][0] = vaesmcq_u8(vaeseq_u8(s[0][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[0][1] = vaesmcq_u8(vaeseq_u8(s[0][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[0][0] = vaesmcq_u8(vaeseq_u8(s[0][0], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[0][1] = vaesmcq_u8(vaeseq_u8(s[0][1], zero8x16)) ^ rc8x16[4 * idx + 3];
      s[1][0] = vaesmcq_u8(vaeseq_u8(s[1][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[1][1] = vaesmcq_u8(vaeseq_u8(s[1][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[1][0] = vaesmcq_u8(vaeseq_u8(s[1][0], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[1][1] = vaesmcq_u8(vaeseq_u8(s[1][1], zero8x16)) ^ rc8x16[4 * idx + 3];
      s[2][0] = vaesmcq_u8(vaeseq_u8(s[2][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[2][1] = vaesmcq_u8(vaeseq_u8(s[2][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[2][0] = vaesmcq_u8(vaeseq_u8(s[2][0], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[2][1] = vaesmcq_u8(vaeseq_u8(s[2][1], zero8x16)) ^ rc8x16[4 * idx + 3];
      s[3][0] = vaesmcq_u8(vaeseq_u8(s[3][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[3][1] = vaesmcq_u8(vaeseq_u8(s[3][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[3][0] = vaesmcq_u8(vaeseq_u8(s[3][0], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[3][1] = vaesmcq_u8(vaeseq_u8(s[3][1], zero8x16)) ^ rc8x16[4 * idx + 3];

      tmp = (u128) vzip2q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][1]);
      s[0][0] = (u128) vzip1q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][1]);
      s[0][1] = tmp;
      tmp = (u128) vzip2q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][1]);
      s[1][0] = (u128) vzip1q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][1]);
      s[1][1] = tmp;
      tmp = (u128) vzip2q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][1]);
      s[2][0] = (u128) vzip1q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][1]);
      s[2][1] = tmp;
      tmp = (u128) vzip2q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][1]);
      s[3][0] = (u128) vzip1q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][1]);
      s[3][1] = tmp;
      };

      s[0][0] = veorq_u8(s[0][0], vld1q_u8(in));
      s[0][1] = veorq_u8(s[0][1], vld1q_u8(in + 16));
      s[1][0] = veorq_u8(s[1][0], vld1q_u8(in + 32));
      s[1][1] = veorq_u8(s[1][1], vld1q_u8(in + 48));
      s[2][0] = veorq_u8(s[2][0], vld1q_u8(in + 64));
      s[2][1] = veorq_u8(s[2][1], vld1q_u8(in + 80));
      s[3][0] = veorq_u8(s[3][0], vld1q_u8(in + 96));
      s[3][1] = veorq_u8(s[3][1], vld1q_u8(in + 112));
  }

  vst1q_u8(out, s[0][0]);
  vst1q_u8(out + 16, s[0][1]);
  vst1q_u8(out + 32, s[1][0]);
  vst1q_u8(out + 48, s[1][1]);
  vst1q_u8(out + 64, s[2][0]);
  vst1q_u8(out + 80, s[2][1]);
  vst1q_u8(out + 96, s[3][0]);
  vst1q_u8(out + 112, s[3][1]);
}

void haraka512_256(unsigned char *out, const unsigned char *in) {
  u128 s[4], s_save[4];
  uint32x4_t tmp;

  s_save[0] = s[0] = vld1q_u8(in);
  s_save[1] = s[1] = vld1q_u8(in + 16);
  s_save[2] = s[2] = vld1q_u8(in + 32);
  s_save[3] = s[3] = vld1q_u8(in + 48);

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[0];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[1];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[2];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[3];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[4];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[5];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[6];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[7];

  tmp = vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[3] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[1], tmp);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[1], tmp);

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[8];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[9];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[10];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[11];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[12];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[13];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[14];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[15];

  tmp = vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[3] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[1], tmp);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[1], tmp);

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[16];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[17];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[18];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[19];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[20];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[21];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[22];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[23];

  tmp = vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[3] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[1], tmp);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[1], tmp);

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[24];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[25];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[26];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[27];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[28];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[29];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[30];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[31];

  tmp = vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[3] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[1], tmp);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[1], tmp);

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[32];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[33];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[34];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[35];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[36];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[37];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[38];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[39];

  tmp = vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[3] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[1], tmp);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[1], tmp);

  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[40];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[41];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[42];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[43];
  s[0] = vaesmcq_u8(vaeseq_u8(s[0], zero8x16)) ^ rc8x16[44];
  s[1] = vaesmcq_u8(vaeseq_u8(s[1], zero8x16)) ^ rc8x16[45];
  s[2] = vaesmcq_u8(vaeseq_u8(s[2], zero8x16)) ^ rc8x16[46];
  s[3] = vaesmcq_u8(vaeseq_u8(s[3], zero8x16)) ^ rc8x16[47];

  tmp = vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[1]);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[2], (uint32x4_t)s[3]);
  s[3] = (u128) vzip1q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[0] = (u128) vzip2q_u32((uint32x4_t)s[0], (uint32x4_t)s[2]);
  s[2] = (u128) vzip2q_u32((uint32x4_t)s[1], tmp);
  s[1] = (u128) vzip1q_u32((uint32x4_t)s[1], tmp);

  s[0] = veorq_u8(s[0], s_save[0]); // = veorq_u8(s[0], vld1q_u8(in));
  s[1] = veorq_u8(s[1], s_save[1]);
  s[2] = veorq_u8(s[2], s_save[2]);
  s[3] = veorq_u8(s[3], s_save[3]);

  *(uint64_t*)(out) = vreinterpretq_u64_u8(s[0])[1];
  *(uint64_t*)(out + 8) = vreinterpretq_u64_u8(s[1])[1];
  *(uint64_t*)(out + 16) = vreinterpretq_u64_u8(s[2])[0];
  *(uint64_t*)(out + 24) = vreinterpretq_u64_u8(s[3])[0];
}

void haraka512_256_4x(unsigned char *out, const unsigned char *in) {
  u128 s[4][4];
  uint32x4_t tmp;

  s[0][0] = vld1q_u8(in);
  s[0][1] = vld1q_u8(in + 16);
  s[0][2] = vld1q_u8(in + 32);
  s[0][3] = vld1q_u8(in + 48);
  s[1][0] = vld1q_u8(in + 64);
  s[1][1] = vld1q_u8(in + 80);
  s[1][2] = vld1q_u8(in + 96);
  s[1][3] = vld1q_u8(in + 112);
  s[2][0] = vld1q_u8(in + 128);
  s[2][1] = vld1q_u8(in + 144);
  s[2][2] = vld1q_u8(in + 160);
  s[2][3] = vld1q_u8(in + 176);
  s[3][0] = vld1q_u8(in + 192);
  s[3][1] = vld1q_u8(in + 208);
  s[3][2] = vld1q_u8(in + 224);
  s[3][3] = vld1q_u8(in + 240);

  for (unsigned idx = 0; idx < 6; ++idx) {

      s[0][0] = vaesmcq_u8(vaeseq_u8(s[0][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[0][1] = vaesmcq_u8(vaeseq_u8(s[0][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[0][2] = vaesmcq_u8(vaeseq_u8(s[0][2], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[0][3] = vaesmcq_u8(vaeseq_u8(s[0][3], zero8x16)) ^ rc8x16[4 * idx + 3];
      s[0][0] = vaesmcq_u8(vaeseq_u8(s[0][0], zero8x16)) ^ rc8x16[4 * idx + 4];
      s[0][1] = vaesmcq_u8(vaeseq_u8(s[0][1], zero8x16)) ^ rc8x16[4 * idx + 5];
      s[0][2] = vaesmcq_u8(vaeseq_u8(s[0][2], zero8x16)) ^ rc8x16[4 * idx + 6];
      s[0][3] = vaesmcq_u8(vaeseq_u8(s[0][3], zero8x16)) ^ rc8x16[4 * idx + 7];

      tmp = vzip1q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][1]);
      s[0][0] = (u128) vzip2q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][1]);
      s[0][1] = (u128) vzip1q_u32((uint32x4_t)s[0][2], (uint32x4_t)s[0][3]);
      s[0][2] = (u128) vzip2q_u32((uint32x4_t)s[0][2], (uint32x4_t)s[0][3]);
      s[0][3] = (u128) vzip1q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][2]);
      s[0][0] = (u128) vzip2q_u32((uint32x4_t)s[0][0], (uint32x4_t)s[0][2]);
      s[0][2] = (u128) vzip2q_u32((uint32x4_t)s[0][1], tmp);
      s[0][1] = (u128) vzip1q_u32((uint32x4_t)s[0][1], tmp);

      s[1][0] = vaesmcq_u8(vaeseq_u8(s[1][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[1][1] = vaesmcq_u8(vaeseq_u8(s[1][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[1][2] = vaesmcq_u8(vaeseq_u8(s[1][2], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[1][3] = vaesmcq_u8(vaeseq_u8(s[1][3], zero8x16)) ^ rc8x16[4 * idx + 3];
      s[1][0] = vaesmcq_u8(vaeseq_u8(s[1][0], zero8x16)) ^ rc8x16[4 * idx + 4];
      s[1][1] = vaesmcq_u8(vaeseq_u8(s[1][1], zero8x16)) ^ rc8x16[4 * idx + 5];
      s[1][2] = vaesmcq_u8(vaeseq_u8(s[1][2], zero8x16)) ^ rc8x16[4 * idx + 6];
      s[1][3] = vaesmcq_u8(vaeseq_u8(s[1][3], zero8x16)) ^ rc8x16[4 * idx + 7];

      tmp = vzip1q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][1]);
      s[1][0] = (u128) vzip2q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][1]);
      s[1][1] = (u128) vzip1q_u32((uint32x4_t)s[1][2], (uint32x4_t)s[1][3]);
      s[1][2] = (u128) vzip2q_u32((uint32x4_t)s[1][2], (uint32x4_t)s[1][3]);
      s[1][3] = (u128) vzip1q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][2]);
      s[1][0] = (u128) vzip2q_u32((uint32x4_t)s[1][0], (uint32x4_t)s[1][2]);
      s[1][2] = (u128) vzip2q_u32((uint32x4_t)s[1][1], tmp);
      s[1][1] = (u128) vzip1q_u32((uint32x4_t)s[1][1], tmp);

      s[2][0] = vaesmcq_u8(vaeseq_u8(s[2][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[2][1] = vaesmcq_u8(vaeseq_u8(s[2][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[2][2] = vaesmcq_u8(vaeseq_u8(s[2][2], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[2][3] = vaesmcq_u8(vaeseq_u8(s[2][3], zero8x16)) ^ rc8x16[4 * idx + 3];
      s[2][0] = vaesmcq_u8(vaeseq_u8(s[2][0], zero8x16)) ^ rc8x16[4 * idx + 4];
      s[2][1] = vaesmcq_u8(vaeseq_u8(s[2][1], zero8x16)) ^ rc8x16[4 * idx + 5];
      s[2][2] = vaesmcq_u8(vaeseq_u8(s[2][2], zero8x16)) ^ rc8x16[4 * idx + 6];
      s[2][3] = vaesmcq_u8(vaeseq_u8(s[2][3], zero8x16)) ^ rc8x16[4 * idx + 7];

      tmp = vzip1q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][1]);
      s[2][0] = (u128) vzip2q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][1]);
      s[2][1] = (u128) vzip1q_u32((uint32x4_t)s[2][2], (uint32x4_t)s[2][3]);
      s[2][2] = (u128) vzip2q_u32((uint32x4_t)s[2][2], (uint32x4_t)s[2][3]);
      s[2][3] = (u128) vzip1q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][2]);
      s[2][0] = (u128) vzip2q_u32((uint32x4_t)s[2][0], (uint32x4_t)s[2][2]);
      s[2][2] = (u128) vzip2q_u32((uint32x4_t)s[2][1], tmp);
      s[2][1] = (u128) vzip1q_u32((uint32x4_t)s[2][1], tmp);

      s[3][0] = vaesmcq_u8(vaeseq_u8(s[3][0], zero8x16)) ^ rc8x16[4 * idx + 0];
      s[3][1] = vaesmcq_u8(vaeseq_u8(s[3][1], zero8x16)) ^ rc8x16[4 * idx + 1];
      s[3][2] = vaesmcq_u8(vaeseq_u8(s[3][2], zero8x16)) ^ rc8x16[4 * idx + 2];
      s[3][3] = vaesmcq_u8(vaeseq_u8(s[3][3], zero8x16)) ^ rc8x16[4 * idx + 3];
      s[3][0] = vaesmcq_u8(vaeseq_u8(s[3][0], zero8x16)) ^ rc8x16[4 * idx + 4];
      s[3][1] = vaesmcq_u8(vaeseq_u8(s[3][1], zero8x16)) ^ rc8x16[4 * idx + 5];
      s[3][2] = vaesmcq_u8(vaeseq_u8(s[3][2], zero8x16)) ^ rc8x16[4 * idx + 6];
      s[3][3] = vaesmcq_u8(vaeseq_u8(s[3][3], zero8x16)) ^ rc8x16[4 * idx + 7];

      tmp = vzip1q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][1]);
      s[3][0] = (u128) vzip2q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][1]);
      s[3][1] = (u128) vzip1q_u32((uint32x4_t)s[3][2], (uint32x4_t)s[3][3]);
      s[3][2] = (u128) vzip2q_u32((uint32x4_t)s[3][2], (uint32x4_t)s[3][3]);
      s[3][3] = (u128) vzip1q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][2]);
      s[3][0] = (u128) vzip2q_u32((uint32x4_t)s[3][0], (uint32x4_t)s[3][2]);
      s[3][2] = (u128) vzip2q_u32((uint32x4_t)s[3][1], tmp);
      s[3][1] = (u128) vzip1q_u32((uint32x4_t)s[3][1], tmp);
  }

  s[0][0] = veorq_u8(s[0][0], vld1q_u8(in));
  s[0][1] = veorq_u8(s[0][1], vld1q_u8(in + 16));
  s[0][2] = veorq_u8(s[0][2], vld1q_u8(in + 32));
  s[0][3] = veorq_u8(s[0][3], vld1q_u8(in + 48));
  s[1][0] = veorq_u8(s[1][0], vld1q_u8(in + 64));
  s[1][1] = veorq_u8(s[1][1], vld1q_u8(in + 80));
  s[1][2] = veorq_u8(s[1][2], vld1q_u8(in + 96));
  s[1][3] = veorq_u8(s[1][3], vld1q_u8(in + 112));
  s[2][0] = veorq_u8(s[2][0], vld1q_u8(in + 128));
  s[2][1] = veorq_u8(s[2][1], vld1q_u8(in + 144));
  s[2][2] = veorq_u8(s[2][2], vld1q_u8(in + 160));
  s[2][3] = veorq_u8(s[2][3], vld1q_u8(in + 176));
  s[3][0] = veorq_u8(s[3][0], vld1q_u8(in + 192));
  s[3][1] = veorq_u8(s[3][1], vld1q_u8(in + 208));
  s[3][2] = veorq_u8(s[3][2], vld1q_u8(in + 224));
  s[3][3] = veorq_u8(s[3][3], vld1q_u8(in + 240));

  *(uint64_t*)(out) = vreinterpretq_u64_u8(s[0][0])[1];
  *(uint64_t*)(out + 8) = vreinterpretq_u64_u8(s[0][1])[1];
  *(uint64_t*)(out + 16) = vreinterpretq_u64_u8(s[0][2])[0];
  *(uint64_t*)(out + 24) = vreinterpretq_u64_u8(s[0][3])[0];
  *(uint64_t*)(out + 32) = vreinterpretq_u64_u8(s[1][0])[1];
  *(uint64_t*)(out + 40) = vreinterpretq_u64_u8(s[1][1])[1];
  *(uint64_t*)(out + 48) = vreinterpretq_u64_u8(s[1][2])[0];
  *(uint64_t*)(out + 56) = vreinterpretq_u64_u8(s[1][3])[0];
  *(uint64_t*)(out + 64) = vreinterpretq_u64_u8(s[2][0])[1];
  *(uint64_t*)(out + 72) = vreinterpretq_u64_u8(s[2][1])[1];
  *(uint64_t*)(out + 80) = vreinterpretq_u64_u8(s[2][2])[0];
  *(uint64_t*)(out + 88) = vreinterpretq_u64_u8(s[2][3])[0];
  *(uint64_t*)(out + 96) = vreinterpretq_u64_u8(s[3][0])[1];
  *(uint64_t*)(out + 104) = vreinterpretq_u64_u8(s[3][1])[1];
  *(uint64_t*)(out + 112) = vreinterpretq_u64_u8(s[3][2])[0];
  *(uint64_t*)(out + 120) = vreinterpretq_u64_u8(s[3][3])[0];

}

