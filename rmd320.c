/*
 * rmd160 - a small RIPEMD-160 / RIPEMD-320 implementation
 * Copyright (C) 2018-2024 G. David Butler <gdb@dbSystems.com>
 *
 * This file is part of rmd160
 *
 * rmd160 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmd160 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "rmd320.h"

typedef unsigned int rmd320_bt; /* unsigned 32 bits */
struct rmd320 {
 rmd320_bt h[10];      /* unsigned 32 bits */
 rmd320_bt bh;         /* bytes processed high */
 rmd320_bt bl;         /* bytes processed low */
 unsigned int l;       /* current short data */
 unsigned char d[64];  /* short data */
};

unsigned int
rmd320tsize(
  void
){
  return (sizeof (rmd320_t));
}

void
rmd320init(
  rmd320_t *v
){
  v->h[0] = 0x67452301U;
  v->h[1] = 0xefcdab89U;
  v->h[2] = 0x98badcfeU;
  v->h[3] = 0x10325476U;
  v->h[4] = 0xc3d2e1f0U;
  v->h[5] = 0x76543210U;
  v->h[6] = 0xfedcba98U;
  v->h[7] = 0x89abcdefU;
  v->h[8] = 0x01234567U;
  v->h[9] = 0x3c2d1e0fU;
  v->bh = v->bl = 0;
  v->l = 0;
}

static void
rmd320mix(
  rmd320_bt h[]
 ,const unsigned char x[]
){
  static const rmd320_bt k[10] = { /* added constants */
    0x00000000U
   ,0x50a28be6U
   ,0x5a827999U
   ,0x5c4dd124U
   ,0x6ed9eba1U
   ,0x6d703ef3U
   ,0x8f1bbcdcU
   ,0x7a6d76e9U
   ,0xa953fd4eU
   ,0x00000000U
  };
  static const unsigned char r[10][16] = { /* message word (from bytes) */
   { 0, 4, 8,12,16,20,24,28,32,36,40,44,48,52,56,60}
  ,{20,56,28, 0,36, 8,44,16,52,24,60,32, 4,40,12,48}
  ,{28,16,52, 4,40,24,60,12,48, 0,36,20, 8,56,44,32}
  ,{24,44,12,28, 0,52,20,40,56,60,32,48,16,36, 4, 8}
  ,{12,40,56,16,36,60,32, 4, 8,28, 0,24,52,44,20,48}
  ,{60,20, 4,12,28,56,24,36,44,32,48, 8,40, 0,16,52}
  ,{ 4,36,44,40, 0,32,48,16,52,12,28,60,56,20,24, 8}
  ,{32,24,16, 4,12,44,60, 0,20,48, 8,52,36,28,40,56}
  ,{16, 0,20,36,28,48, 8,40,56, 4,12,32,44,24,60,52}
  ,{48,60,40,16, 4,20,32,28,24, 8,52,56, 0,12,36,44}
  };
  static const unsigned char s[10][16] = { /* amount to rotate left */
   {11,14,15,12, 5, 8, 7, 9,11,13,14,15, 6, 7, 9, 8}
  ,{ 8, 9, 9,11,13,15,15, 5, 7, 7, 8,11,14,14,12, 6}
  ,{ 7, 6, 8,13,11, 9, 7,15, 7,12,15, 9,11, 7,13,12}
  ,{ 9,13,15, 7,12, 8, 9,11, 7, 7,12, 7, 6,15,13,11}
  ,{11,13, 6, 7,14, 9,13,15,14, 8,13, 6, 5,12, 7, 5}
  ,{ 9, 7,15,11, 8, 6, 6,14,12,13, 5,14,13,13, 7, 5}
  ,{11,12,14,15,14,15, 9, 8, 9,14, 5, 6, 8, 6, 5,12}
  ,{15, 5, 8,11,14,14, 6,14, 6, 9,12, 9,12, 5,15, 8}
  ,{ 9,15, 5,11, 6, 8,13,12, 5,12,13,14,11, 8, 5, 6}
  ,{ 8, 5,12, 9,12, 5,14, 6, 8,13, 6, 5,15,13,11,11}
  };
  static const unsigned char v[10][16][5] = { /* hash rotate */
  {
   {0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2}
  ,{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4}
  },{
   {5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7}
  ,{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9}
  },{
   {4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1}
  ,{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3}
  },{
   {9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6}
  ,{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8}
  },{
   {3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0}
  ,{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2}
  },{
   {8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5}
  ,{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7}
  },{
   {2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4}
  ,{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1}
  },{
   {7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9}
  ,{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6}
  },{
   {1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3}
  ,{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0}
  },{
   {6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8}
  ,{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5}
  }
  };
  rmd320_bt t[10]; /* a=0 b=1 c=2 d=3 e=4 a'=5 b'=6 c'=7 d'=8 e'=9 */
  rmd320_bt f;
  unsigned int i;
  unsigned int j;

  for (i = 0; i < 10; ++i)
    t[i] = h[i];
  for (i = 0; i < 10; ++i) {
    for (j = 0; j < 16; ++j) {
      switch (i) {
      case 0: case 9: /* x XOR y XOR z */
        f = t[v[i][j][1]] ^ t[v[i][j][2]] ^ t[v[i][j][3]];
        break;
      case 2: case 7: /* (x AND y) OR (NOT(x) AND z) */
        f = (t[v[i][j][1]] & t[v[i][j][2]]) | (~t[v[i][j][1]] & t[v[i][j][3]]);
        break;
      case 4: case 5: /* (x OR NOT(y)) XOR z */
        f = (t[v[i][j][1]] | ~t[v[i][j][2]]) ^ t[v[i][j][3]];
        break;
      case 6: case 3: /* (x AND z) OR (y AND NOT(z)) */
        f = (t[v[i][j][1]] & t[v[i][j][3]]) | (t[v[i][j][2]] & ~t[v[i][j][3]]);
        break;
      case 8: case 1: /* x XOR (y OR NOT(z)) */
        f = t[v[i][j][1]] ^ (t[v[i][j][2]] | ~t[v[i][j][3]]);
        break;
      }
      f += t[v[i][j][0]]
         + (x[r[i][j] + 0] << 0 | x[r[i][j] + 1] << 8 | x[r[i][j] + 2] << 16 | x[r[i][j] + 3] << 24)
         + k[i];
      t[v[i][j][0]] = ((f << s[i][j]) | (f >> (32 - s[i][j]))) /* rotate left */
                    + t[v[i][j][4]];
      t[v[i][j][2]] = (t[v[i][j][2]] << 10) | (t[v[i][j][2]] >> (32 - 10)); /* rotate left */
    }
    switch (i) {
    case 1:
      f = t[0];
      t[0] = t[5];
      t[5] = f;
      break;
    case 3:
      f = t[1];
      t[1] = t[6];
      t[6] = f;
      break;
    case 5:
      f = t[2];
      t[2] = t[7];
      t[7] = f;
      break;
    case 7:
      f = t[3];
      t[3] = t[8];
      t[8] = f;
      break;
    case 9:
      f = t[4];
      t[4] = t[9];
      t[9] = f;
      break;
    default:
      break;
    }
  }
  for (i = 0; i < 10; ++i)
    h[i] += t[i];
}

void
rmd320update(
  rmd320_t *v
 ,const unsigned char *d
 ,unsigned int l
){
  unsigned char *s;

  if (v->l) {
    unsigned int i;

    for (i = v->l, s = v->d + i; l && i < 64; --l, ++i, ++s, ++d)
      *s = *d;
    if (i == 64) {
      rmd320mix(v->h, v->d);
      if ((v->bl += 64) < 64)
        ++v->bh;
      v->l = 0;
    } else {
      v->l = i;
      return;
    }
  }
  for (; l >= 64; l -= 64, d += 64) {
    rmd320mix(v->h, d);
    if ((v->bl += 64) < 64)
      ++v->bh;
  }
  if (l) {
    v->l = l;
    for (s = v->d; l; --l, ++s, ++d)
      *s = *d;
  }
}

void
rmd320final(
  rmd320_t *v
 ,unsigned char *h
){
  unsigned char *s;
  unsigned int i;

  if ((i = v->l))
    if ((v->bl += i) < i)
      ++v->bh;
  s = v->d + i++;
  *s++ = 0x80;
  if (i > 64 - 8) {
    for (; i < 64; ++i, ++s)
      *s = 0x00;
    rmd320mix(v->h, v->d);
    i = 0;
    s = v->d;
  }
  for (; i < 64 - 8; ++i, ++s)
    *s = 0x00;
  /* bytes to bits * 8=2^3 */
  *s++ = v->bl << 3;
  *s++ = v->bl >> (1 * 8 - 3);
  *s++ = v->bl >> (2 * 8 - 3);
  *s++ = v->bl >> (3 * 8 - 3);
  *s++ = v->bh << 3;
  *s++ = v->bh >> (1 * 8 - 3);
  *s++ = v->bh >> (2 * 8 - 3);
  *s   = v->bh >> (3 * 8 - 3);
  rmd320mix(v->h, v->d);
  for (i = 0; i < 10; ++i) {
    *h++ = v->h[i] >> (0 * 8);
    *h++ = v->h[i] >> (1 * 8);
    *h++ = v->h[i] >> (2 * 8);
    *h++ = v->h[i] >> (3 * 8);
  }
}

void
rmd320hmac(
  const unsigned char *k
 ,unsigned int kl
 ,const unsigned char *d
 ,unsigned int dl
 ,unsigned char *h
){
  rmd320_t c;
  unsigned char i[64];
  unsigned char o[64];
  unsigned int l;

  if (kl > 64) {
    rmd320init(&c);
    rmd320update(&c, k, kl);
    rmd320final(&c, h);
    k = h;
    kl = 40;
  }
  for (l = 0; l < kl; ++l) {
    i[l] = *(k + l) ^ 0x36;
    o[l] = *(k + l) ^ 0x5c;
  }
  for (; l < 64; ++l) {
    i[l] = 0x00 ^ 0x36;
    o[l] = 0x00 ^ 0x5c;
  }
  rmd320init(&c);
  rmd320update(&c, i, sizeof (i));
  rmd320update(&c, d, dl);
  rmd320final(&c, h);
  rmd320init(&c);
  rmd320update(&c, o, sizeof (o));
  rmd320update(&c, h, 40);
  rmd320final(&c, h);
}

void
rmd320hex(
  const unsigned char *h
 ,char *o
){
  unsigned int i;

  for (i = 0; i < 40; ++i, ++h) {
    static const char m[] = "0123456789abcdef";

    *o++ = m[(*h >> 4) & 0xf];
    *o++ = m[(*h >> 0) & 0xf];
  }
}
