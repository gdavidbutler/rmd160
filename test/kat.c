#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rmd160.h"
#include "rmd320.h"

/* Spec vectors from https://homes.esat.kuleuven.be/~bosselae/ripemd160.html */
struct svec {
  const char *in;
  unsigned int inlen;
  const char *h160;
  const char *h320;
};

static const struct svec Svec[] = {
  { "", 0
   ,"9c1185a5c5e9fc54612808977ee8f548b2258d31"
   ,"22d65d5661536cdc75c1fdf5c6de7b41b9f27325ebc61e8557177d705a0ec880151c3a32a00899b8" }
 ,{ "a", 1
   ,"0bdc9d2d256b3ee9daae347be6f4dc835a467ffe"
   ,"ce78850638f92658a5a585097579926dda667a5716562cfcf6fbe77f63542f99b04705d6970dff5d" }
 ,{ "abc", 3
   ,"8eb208f7e05d987a9b044a8e98c6b087f15a0bfc"
   ,"de4c01b3054f8930a79d09ae738e92301e5a17085beffdc1b8d116713e74f82fa942d64cdbc4682d" }
 ,{ "message digest", 14
   ,"5d0689ef49d2fae572b881b123a85ffa21595f36"
   ,"3a8e28502ed45d422f68844f9dd316e7b98533fa3f2a91d29f84d425c88d6b4eff727df66a7c0197" }
 ,{ "abcdefghijklmnopqrstuvwxyz", 26
   ,"f71c27109c692c1b56bbdceb5b9d2865b3708dbc"
   ,"cabdb1810b92470a2093aa6bce05952c28348cf43ff60841975166bb40ed234004b8824463e6b009" }
 ,{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56
   ,"12a053384a9c0c88e405a06c27dcf49ada62eb2b"
   ,"d034a7950cf722021ba4b84df769a5de2060e259df4c9bb4a4268c0e935bbc7470a969c9d072a1ac" }
 ,{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", 62
   ,"b0e20b6e3116640286ed3a87a5713079b21f5189"
   ,"ed544940c86d67f250d232c30b7b3e5770e0c60c8cb9a4cafe3b11388af9920e1b99230b843c86a4" }
 ,{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 80
   ,"9b752e45573d4b39f4dbd3323cab82bf63326bfb"
   ,"557888af5f6d8ed62ab66945c6d2a0a47ecd5341e915eb8fea1d0524955f825dc717e4a008ab2d42" }
};

static void
hexof(
  const unsigned char *b
 ,unsigned int n
 ,char *o
){
  static const char m[] = "0123456789abcdef";
  unsigned int q;

  for (q = 0; q < n; ++q) {
    *o++ = m[(b[q] >> 4) & 0xf];
    *o++ = m[b[q]       & 0xf];
  }
  *o = 0;
}

static int
check(
  const char *label
 ,const char *got
 ,const char *want
){
  if (strcmp(got, want) == 0)
    return (0);
  fprintf(stderr, "FAIL %s: got %s want %s\n", label, got, want);
  return (1);
}

int
main(
  void
){
  static const unsigned char Hk1[20] = {
    0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
   ,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
  };
  static const char Hd1[] = "Hi There";
  static const char Hk2[] = "Jefe";
  static const char Hd2[] = "what do ya want for nothing?";
  static unsigned char Hk6[80];
  static const char Hd6[] = "Test Using Larger Than Block-Size Key - Hash Key First";
  static const char Msg[] =
    "The quick brown fox jumps over the lazy dog."
    "The quick brown fox jumps over the lazy dog."
    "The quick brown fox jumps over the lazy dog."
    "The quick brown fox jumps over the lazy dog.";
  static const unsigned int Chunks[] = { 1, 7, 31, 55, 56, 63, 64, 65, 88, 127, 128 };
  rmd160_t *c160;
  rmd320_t *c320;
  unsigned char h160[RMD160_SZ];
  unsigned char h320[RMD320_SZ];
  char b160[2 * RMD160_SZ + 1];
  char b320[2 * RMD320_SZ + 1];
  char ref160[2 * RMD160_SZ + 1];
  char ref320[2 * RMD320_SZ + 1];
  unsigned char *mega;
  unsigned int msglen;
  unsigned int q;
  unsigned int off;
  unsigned int n;
  int fail;

  if (!(c160 = malloc(rmd160tsize()))) {
    fprintf(stderr, "FAIL: malloc(rmd160tsize)\n");
    return (1);
  }
  if (!(c320 = malloc(rmd320tsize()))) {
    fprintf(stderr, "FAIL: malloc(rmd320tsize)\n");
    free(c160);
    return (1);
  }
  fail = 0;

  /* Spec KATs */
  for (q = 0; q < sizeof (Svec) / sizeof (Svec[0]); ++q) {
    rmd160init(c160);
    rmd160update(c160, (const unsigned char *)Svec[q].in, Svec[q].inlen);
    rmd160final(c160, h160);
    hexof(h160, RMD160_SZ, b160);
    fail += check("rmd160 spec", b160, Svec[q].h160);

    rmd320init(c320);
    rmd320update(c320, (const unsigned char *)Svec[q].in, Svec[q].inlen);
    rmd320final(c320, h320);
    hexof(h320, RMD320_SZ, b320);
    fail += check("rmd320 spec", b320, Svec[q].h320);
  }

  /* Million-'a' */
  if (!(mega = malloc(1000000))) {
    fprintf(stderr, "FAIL: malloc(1000000)\n");
    free(c160);
    free(c320);
    return (1);
  }
  memset(mega, 'a', 1000000);
  rmd160init(c160);
  rmd160update(c160, mega, 1000000);
  rmd160final(c160, h160);
  hexof(h160, RMD160_SZ, b160);
  fail += check("rmd160 million-a", b160, "52783243c1697bdbe16d37f97f68f08325dc1528");

  rmd320init(c320);
  rmd320update(c320, mega, 1000000);
  rmd320final(c320, h320);
  hexof(h320, RMD320_SZ, b320);
  fail += check("rmd320 million-a", b320, "bdee37f4371e20646b8b0d862dda16292ae36f40965e8c8509e63d1dbddecc503e2b63eb9245bb66");
  free(mega);

  /* Streaming-boundary equivalence: one-shot vs chunk-by-chunk at sizes
     that straddle the 64-byte block buffer */
  msglen = sizeof (Msg) - 1;
  rmd160init(c160);
  rmd160update(c160, (const unsigned char *)Msg, msglen);
  rmd160final(c160, h160);
  hexof(h160, RMD160_SZ, ref160);
  rmd320init(c320);
  rmd320update(c320, (const unsigned char *)Msg, msglen);
  rmd320final(c320, h320);
  hexof(h320, RMD320_SZ, ref320);
  for (q = 0; q < sizeof (Chunks) / sizeof (Chunks[0]); ++q) {
    rmd160init(c160);
    rmd320init(c320);
    for (off = 0; off < msglen; off += n) {
      n = Chunks[q];
      if (off + n > msglen)
        n = msglen - off;
      rmd160update(c160, (const unsigned char *)Msg + off, n);
      rmd320update(c320, (const unsigned char *)Msg + off, n);
    }
    rmd160final(c160, h160);
    rmd320final(c320, h320);
    hexof(h160, RMD160_SZ, b160);
    hexof(h320, RMD320_SZ, b320);
    fail += check("rmd160 stream", b160, ref160);
    fail += check("rmd320 stream", b320, ref320);
  }

  /* HMAC-RIPEMD-160 from RFC 2286 */
  rmd160hmac(Hk1, sizeof (Hk1)
            ,(const unsigned char *)Hd1, sizeof (Hd1) - 1, h160);
  hexof(h160, RMD160_SZ, b160);
  fail += check("hmac-rmd160 t1", b160, "24cb4bd67d20fc1a5d2ed7732dcc39377f0a5668");

  rmd160hmac((const unsigned char *)Hk2, sizeof (Hk2) - 1
            ,(const unsigned char *)Hd2, sizeof (Hd2) - 1, h160);
  hexof(h160, RMD160_SZ, b160);
  fail += check("hmac-rmd160 t2", b160, "dda6c0213a485a9e24f4742064a7f033b43c4069");

  for (q = 0; q < sizeof (Hk6); ++q)
    Hk6[q] = 0xaa;
  rmd160hmac(Hk6, sizeof (Hk6)
            ,(const unsigned char *)Hd6, sizeof (Hd6) - 1, h160);
  hexof(h160, RMD160_SZ, b160);
  fail += check("hmac-rmd160 t6", b160, "6466ca07ac5eac29e1bd523e5ada7605b791fd8b");

  /* HMAC-RIPEMD-320: no RFC vectors exist; regression against this impl */
  rmd320hmac(Hk1, sizeof (Hk1)
            ,(const unsigned char *)Hd1, sizeof (Hd1) - 1, h320);
  hexof(h320, RMD320_SZ, b320);
  fail += check("hmac-rmd320 t1", b320, "c6e94996ce2f32b5e819a9b6d2ec2c69d727a66612729fa070deeffed1ac066fe73fd1397d02d049");

  rmd320hmac(Hk6, sizeof (Hk6)
            ,(const unsigned char *)Hd6, sizeof (Hd6) - 1, h320);
  hexof(h320, RMD320_SZ, b320);
  fail += check("hmac-rmd320 t6", b320, "a61c1123c1a23ffdd9af5fb28b62b95f989876ac322452f5e4f4470c25182ef4816aaec7f0377167");

  /* the one-shot and the keyed form, one kinit then a khmac a message,
   * each RFC 2104's computed here from the hash alone, at keys under,
   * at and past the block; the one-shot and
   * the keyed form writing over their own data under a key past the
   * block; and a key past the block keyed as its hash, so no byte of it
   * is left in the state */
  {
    static const unsigned int Kl[] = { 0, 1, 20, 64 - 1, 64, 64 + 1, 2 * 64 + 7 };
    static const unsigned int Dl[] = { 0, 1, 55, 64, 200, 500 };
    static unsigned char Kb[2 * 64 + 7];
    static unsigned char Db[500];
    static unsigned char Wb[500];
    rmd160_t *kc;
    rmd160k_t *kx;
    rmd160k_t *ky;
    unsigned char kb[64];
    unsigned char p[64];
    unsigned char t[RMD160_SZ];
    unsigned char u[RMD160_SZ];
    unsigned int i;
    unsigned int j;
    unsigned int l;

    for (i = 0; i < sizeof (Kb); ++i)
      Kb[i] = (unsigned char)(i * 13 + 5);
    for (i = 0; i < sizeof (Db); ++i)
      Db[i] = (unsigned char)(i * 7 + 3);
    kx = ky = 0;
    if (!(kc = malloc(rmd160tsize())) || !(kx = calloc(1, rmd160ktsize())) || !(ky = calloc(1, rmd160ktsize()))) {
      fprintf(stderr, "FAIL: malloc(rmd160tsize, rmd160ktsize)\n");
      free(kc);
      free(kx);
      return (1);
    }
    for (i = 0; i < sizeof (Kl) / sizeof (Kl[0]); ++i) {
      memset(kb, 0, sizeof (kb));
      if (Kl[i] > sizeof (kb)) {
        rmd160init(kc);
        rmd160update(kc, Kb, Kl[i]);
        rmd160final(kc, kb);
      } else
        memcpy(kb, Kb, Kl[i]);
      rmd160kinit(kx, Kb, Kl[i]);
      for (j = 0; j < sizeof (Dl) / sizeof (Dl[0]); ++j) {
        for (l = 0; l < sizeof (p); ++l)
          p[l] = kb[l] ^ 0x36;
        rmd160init(kc);
        rmd160update(kc, p, sizeof (p));
        rmd160update(kc, Db, Dl[j]);
        rmd160final(kc, t);
        for (l = 0; l < sizeof (p); ++l)
          p[l] = kb[l] ^ 0x5c;
        rmd160init(kc);
        rmd160update(kc, p, sizeof (p));
        rmd160update(kc, t, RMD160_SZ);
        rmd160final(kc, t);
        rmd160hmac(Kb, Kl[i], Db, Dl[j], u);
        if (memcmp(t, u, RMD160_SZ)) {
          fprintf(stderr, "FAIL rmd160hmac: key %u data %u not RFC 2104's\n", Kl[i], Dl[j]);
          ++fail;
        }
        rmd160khmac(kx, Db, Dl[j], u);
        if (memcmp(t, u, RMD160_SZ)) {
          fprintf(stderr, "FAIL rmd160khmac: key %u data %u not RFC 2104's\n", Kl[i], Dl[j]);
          ++fail;
        }
      }
    }
    rmd160hmac(Kb, sizeof (Kb), Db, 100, u);
    memcpy(Wb, Db, sizeof (Wb));
    rmd160hmac(Kb, sizeof (Kb), Wb, 100, Wb);
    if (memcmp(Wb, u, RMD160_SZ)) {
      fprintf(stderr, "FAIL rmd160hmac: over its own data, a long key\n");
      ++fail;
    }
    memcpy(Wb, Db, sizeof (Wb));
    rmd160kinit(kx, Kb, sizeof (Kb));
    rmd160khmac(kx, Wb, 100, Wb);
    if (memcmp(Wb, u, RMD160_SZ)) {
      fprintf(stderr, "FAIL rmd160khmac: over its own data, a long key\n");
      ++fail;
    }
    rmd160init(kc);
    rmd160update(kc, Kb, sizeof (Kb));
    rmd160final(kc, t);
    memset(kx, 0, rmd160ktsize());
    rmd160kinit(kx, Kb, sizeof (Kb));
    rmd160kinit(ky, t, RMD160_SZ);
    if (memcmp(kx, ky, rmd160ktsize())) {
      fprintf(stderr, "FAIL rmd160kinit: a long key's bytes left in the state\n");
      ++fail;
    }
    free(kc);
    free(kx);
    free(ky);
  }

  /* the one-shot and the keyed form, one kinit then a khmac a message,
   * each RFC 2104's computed here from the hash alone, at keys under,
   * at and past the block; the one-shot and
   * the keyed form writing over their own data under a key past the
   * block; and a key past the block keyed as its hash, so no byte of it
   * is left in the state */
  {
    static const unsigned int Kl[] = { 0, 1, 20, 64 - 1, 64, 64 + 1, 2 * 64 + 7 };
    static const unsigned int Dl[] = { 0, 1, 55, 64, 200, 500 };
    static unsigned char Kb[2 * 64 + 7];
    static unsigned char Db[500];
    static unsigned char Wb[500];
    rmd320_t *kc;
    rmd320k_t *kx;
    rmd320k_t *ky;
    unsigned char kb[64];
    unsigned char p[64];
    unsigned char t[RMD320_SZ];
    unsigned char u[RMD320_SZ];
    unsigned int i;
    unsigned int j;
    unsigned int l;

    for (i = 0; i < sizeof (Kb); ++i)
      Kb[i] = (unsigned char)(i * 13 + 5);
    for (i = 0; i < sizeof (Db); ++i)
      Db[i] = (unsigned char)(i * 7 + 3);
    kx = ky = 0;
    if (!(kc = malloc(rmd320tsize())) || !(kx = calloc(1, rmd320ktsize())) || !(ky = calloc(1, rmd320ktsize()))) {
      fprintf(stderr, "FAIL: malloc(rmd320tsize, rmd320ktsize)\n");
      free(kc);
      free(kx);
      return (1);
    }
    for (i = 0; i < sizeof (Kl) / sizeof (Kl[0]); ++i) {
      memset(kb, 0, sizeof (kb));
      if (Kl[i] > sizeof (kb)) {
        rmd320init(kc);
        rmd320update(kc, Kb, Kl[i]);
        rmd320final(kc, kb);
      } else
        memcpy(kb, Kb, Kl[i]);
      rmd320kinit(kx, Kb, Kl[i]);
      for (j = 0; j < sizeof (Dl) / sizeof (Dl[0]); ++j) {
        for (l = 0; l < sizeof (p); ++l)
          p[l] = kb[l] ^ 0x36;
        rmd320init(kc);
        rmd320update(kc, p, sizeof (p));
        rmd320update(kc, Db, Dl[j]);
        rmd320final(kc, t);
        for (l = 0; l < sizeof (p); ++l)
          p[l] = kb[l] ^ 0x5c;
        rmd320init(kc);
        rmd320update(kc, p, sizeof (p));
        rmd320update(kc, t, RMD320_SZ);
        rmd320final(kc, t);
        rmd320hmac(Kb, Kl[i], Db, Dl[j], u);
        if (memcmp(t, u, RMD320_SZ)) {
          fprintf(stderr, "FAIL rmd320hmac: key %u data %u not RFC 2104's\n", Kl[i], Dl[j]);
          ++fail;
        }
        rmd320khmac(kx, Db, Dl[j], u);
        if (memcmp(t, u, RMD320_SZ)) {
          fprintf(stderr, "FAIL rmd320khmac: key %u data %u not RFC 2104's\n", Kl[i], Dl[j]);
          ++fail;
        }
      }
    }
    rmd320hmac(Kb, sizeof (Kb), Db, 100, u);
    memcpy(Wb, Db, sizeof (Wb));
    rmd320hmac(Kb, sizeof (Kb), Wb, 100, Wb);
    if (memcmp(Wb, u, RMD320_SZ)) {
      fprintf(stderr, "FAIL rmd320hmac: over its own data, a long key\n");
      ++fail;
    }
    memcpy(Wb, Db, sizeof (Wb));
    rmd320kinit(kx, Kb, sizeof (Kb));
    rmd320khmac(kx, Wb, 100, Wb);
    if (memcmp(Wb, u, RMD320_SZ)) {
      fprintf(stderr, "FAIL rmd320khmac: over its own data, a long key\n");
      ++fail;
    }
    rmd320init(kc);
    rmd320update(kc, Kb, sizeof (Kb));
    rmd320final(kc, t);
    memset(kx, 0, rmd320ktsize());
    rmd320kinit(kx, Kb, sizeof (Kb));
    rmd320kinit(ky, t, RMD320_SZ);
    if (memcmp(kx, ky, rmd320ktsize())) {
      fprintf(stderr, "FAIL rmd320kinit: a long key's bytes left in the state\n");
      ++fail;
    }
    free(kc);
    free(kx);
    free(ky);
  }

  free(c160);
  free(c320);
  if (fail) {
    fprintf(stderr, "%d test(s) failed\n", fail);
    return (1);
  }
  printf("all tests passed\n");
  return (0);
}
