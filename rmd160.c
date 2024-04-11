/*
 * rmd160 - a small RIPEMD-160 implementation
 * Copyright (C) 2018-2023 G. David Butler <gdb@dbSystems.com>
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

#include "rmd160.h"

typedef unsigned int rmd160_bt; /* unsigned 32 bits */
struct rmd160 {
 rmd160_bt h[5];       /* unsigned 32 bits */
 unsigned char d[64];  /* short data */
 unsigned long b;      /* bytes processed */
 unsigned int l;       /* current short data */
};

unsigned int
rmd160tsize(
  void
){
  return (sizeof (rmd160_t));
}

void
rmd160init(
  rmd160_t *v
){
  v->h[0] = 0x67452301U;
  v->h[1] = 0xefcdab89U;
  v->h[2] = 0x98badcfeU;
  v->h[3] = 0x10325476U;
  v->h[4] = 0xc3d2e1f0U;
  v->b = 0;
  v->l = 0;
}

static rmd160_bt
rmd160_f0(
 rmd160_bt x
,rmd160_bt y
,rmd160_bt z
){
  return (x ^ y ^ z);
}

static rmd160_bt
rmd160_f1(
 rmd160_bt x
,rmd160_bt y
,rmd160_bt z
){
  return ((x & y) | (~x & z)); 
}

static rmd160_bt
rmd160_f2(
 rmd160_bt x
,rmd160_bt y
,rmd160_bt z
){
  return ((x | ~y) ^ z);
}

static rmd160_bt
rmd160_f3(
 rmd160_bt x
,rmd160_bt y
,rmd160_bt z
){
  return ((x & z) | (y & ~z)); 
}

static rmd160_bt
rmd160_f4(
 rmd160_bt x
,rmd160_bt y
,rmd160_bt z
){
  return (x ^ (y | ~z));
}

static rmd160_bt
rmd160_rl(
 rmd160_bt x
,unsigned char n
){
  return (((x << n) | (x >> (32 - n))));
}

static void
rmd160mix(
  rmd160_bt h[]
 ,unsigned char x[]
){
  static rmd160_bt(*f[10])(rmd160_bt, rmd160_bt, rmd160_bt) = { /* nonlinear functions at bit level: exor, mux, -, mux, - */
    rmd160_f0
   ,rmd160_f1
   ,rmd160_f2
   ,rmd160_f3
   ,rmd160_f4

   ,rmd160_f4
   ,rmd160_f3
   ,rmd160_f2
   ,rmd160_f1
   ,rmd160_f0
  };
  static rmd160_bt k[10] = { /* added constants (hexadecimal) */
    0x00000000U
   ,0x5a827999U
   ,0x6ed9eba1U
   ,0x8f1bbcdcU
   ,0xa953fd4eU

   ,0x50a28be6U
   ,0x5c4dd124U
   ,0x6d703ef3U
   ,0x7a6d76e9U
   ,0x00000000U
  };
  static unsigned char r[10][16] = { /* selection of message word from byte array */
   { 0, 4, 8,12,16,20,24,28,32,36,40,44,48,52,56,60}
  ,{28,16,52, 4,40,24,60,12,48, 0,36,20, 8,56,44,32}
  ,{12,40,56,16,36,60,32, 4, 8,28, 0,24,52,44,20,48}
  ,{ 4,36,44,40, 0,32,48,16,52,12,28,60,56,20,24, 8}
  ,{16, 0,20,36,28,48, 8,40,56, 4,12,32,44,24,60,52}

  ,{20,56,28, 0,36, 8,44,16,52,24,60,32, 4,40,12,48}
  ,{24,44,12,28, 0,52,20,40,56,60,32,48,16,36, 4, 8}
  ,{60,20, 4,12,28,56,24,36,44,32,48, 8,40, 0,16,52}
  ,{32,24,16, 4,12,44,60, 0,20,48, 8,52,36,28,40,56}
  ,{48,60,40,16, 4,20,32,28,24, 8,52,56, 0,12,36,44}
  };
  static unsigned char s[10][16] = { /* amount for rotate left (rol) */
   {11,14,15,12, 5, 8, 7, 9,11,13,14,15, 6, 7, 9, 8}
  ,{ 7, 6, 8,13,11, 9, 7,15, 7,12,15, 9,11, 7,13,12}
  ,{11,13, 6, 7,14, 9,13,15,14, 8,13, 6, 5,12, 7, 5}
  ,{11,12,14,15,14,15, 9, 8, 9,14, 5, 6, 8, 6, 5,12}
  ,{ 9,15, 5,11, 6, 8,13,12, 5,12,13,14,11, 8, 5, 6}

  ,{ 8, 9, 9,11,13,15,15, 5, 7, 7, 8,11,14,14,12, 6}
  ,{ 9,13,15, 7,12, 8, 9,11, 7, 7,12, 7, 6,15,13,11}
  ,{ 9, 7,15,11, 8, 6, 6,14,12,13, 5,14,13,13, 7, 5}
  ,{15, 5, 8,11,14,14, 6,14, 6, 9,12, 9,12, 5,15, 8}
  ,{ 8, 5,12, 9,12, 5,14, 6, 8,13, 6, 5,15,13,11,11}
  };
  static unsigned char v[10][16][5] = { /* hash rotation */
  {
   {0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2}
  ,{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4}
  },{
   {4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1}
  ,{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3}
  },{
   {3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0}
  ,{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2}
  },{
   {2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4}
  ,{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1}
  },{
   {1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3}
  ,{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0},{0,1,2,3,4},{4,0,1,2,3},{3,4,0,1,2},{2,3,4,0,1},{1,2,3,4,0}
  }
  ,
  {
   {5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7}
  ,{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9}
  },{
   {9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6}
  ,{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8}
  },{
   {8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5}
  ,{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7}
  },{
   {7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9}
  ,{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6}
  },{
   {6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8}
  ,{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5},{5,6,7,8,9},{9,5,6,7,8},{8,9,5,6,7},{7,8,9,5,6},{6,7,8,9,5}
  }
  };
  rmd160_bt t[10]; /* a=0 b=1 c=2 d=3 e=4 a'=5 b'=6 c'=7 d'=8 e'=9 */
  unsigned int i;
  unsigned int j;

  for (i = 0; i < 5; ++i)
    t[5 + i] = t[i] = h[i];
  for (i = 0; i < 10; ++i) {
    for (j = 0; j < 16; ++j) {
      t[v[i][j][0]] = rmd160_rl(t[v[i][j][0]]
                              + f[i](t[v[i][j][1]], t[v[i][j][2]], t[v[i][j][3]])
                              + (x[r[i][j] + 0] << 0 | x[r[i][j] + 1] << 8 | x[r[i][j] + 2] << 16 | x[r[i][j] + 3] << 24)
                              + k[i]
                              , s[i][j]
                      )
                    + t[v[i][j][4]];
      t[v[i][j][2]] = rmd160_rl(t[v[i][j][2]], 10);
    }
  }
  t[2] = h[1] + t[2] + t[8];
  h[1] = h[2] + t[3] + t[9];
  h[2] = h[3] + t[4] + t[5];
  h[3] = h[4] + t[0] + t[6];
  h[4] = h[0] + t[1] + t[7];
  h[0] = t[2];
}

void
rmd160update(
  rmd160_t *v
 ,unsigned char *d
 ,unsigned int l
){
  unsigned char *s;

  if (v->l) {
    unsigned int i;

    for (i = v->l, s = v->d + i; l && i < 64; --l, ++i, ++s, ++d)
      *s = *d;
    if (i == 64) {
      rmd160mix(v->h, v->d);
      v->b += 64;
      v->l = 0;
    } else {
      v->l = i;
      return;
    }
  }
  for (; l >= 64; l -= 64, d += 64) {
    rmd160mix(v->h, d);
    v->b += 64;
  }
  if (l) {
    v->l = l;
    for (s = v->d; l; --l, ++s, ++d)
      *s = *d;
  }
}

void
rmd160final(
  rmd160_t *v
 ,unsigned char *h
){
  unsigned char *s;
  unsigned int i;

  if ((i = v->l))
    v->b += i;
  s = v->d + i++;
  *s++ = 0x80;
  if (i > 64 - 8) {
    for (; i < 64; ++i, ++s)
      *s = 0x00;
    rmd160mix(v->h, v->d);
    i = 0;
    s = v->d;
  }
  for (; i < 64 - 8; ++i, ++s)
    *s = 0x00;
  /* convert bytes to bits * 8=2^3 */
  *s++ = v->b << 3;
  *s++ = v->b >> (1 * 8 - 3);
  *s++ = v->b >> (2 * 8 - 3);
  *s++ = v->b >> (3 * 8 - 3);
  *s++ = v->b >> (4 * 8 - 3);
  *s++ = v->b >> (5 * 8 - 3);
  *s++ = v->b >> (6 * 8 - 3);
  *s   = v->b >> (7 * 8 - 3);
  rmd160mix(v->h, v->d);
  for (i = 0; i < 5; ++i) {
    *h++ = v->h[i] >> (0 * 8);
    *h++ = v->h[i] >> (1 * 8);
    *h++ = v->h[i] >> (2 * 8);
    *h++ = v->h[i] >> (3 * 8);
  }
}

void
rmd160hex(
  unsigned char *h
 ,char *o
){
  unsigned int i;

  for (i = 0; i < 20; ++i, ++h) {
    static char m[] = "0123456789abcdef";

    *o++ = m[(*h >> 4) & 0xf];
    *o++ = m[(*h >> 0) & 0xf];
  }
}