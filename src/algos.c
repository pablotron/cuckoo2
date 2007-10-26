
#include "ck_private.h"

/*
 * (used by superfasthash below)
 */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif /* get16bits */

/*
 * (used by superfasthash below)
 */
#undef get32bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get32bits(d) (*((const uint32_t *) (d)))
#endif

#if !defined (get32bits)
#define get32bits(d) (  (uint32_t)(d)[0]                \
                     + ((uint32_t)(d)[1]<<UINT32_C(8))  \
                     + ((uint32_t)(d)[2]<<UINT32_C(16)) \
                     + ((uint32_t)(d)[3]<<UINT32_C(24)) )
#endif /* get32bits */


/*
 * Paul Hseih's SuperFastHash 
 *
 * http://www.azillionmonkeys.com/qed/hash.html
 *
 */
uint32_t 
ck_hash_superfast(const char * data, int len) {
  uint32_t hash = 0, tmp;
  int rem;

  if (len <= 0 || data == NULL) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }

  /* Handle end cases */
  switch (rem) {
    case 3:  hash += get16bits (data);
        hash ^= hash << 16;
        hash ^= data[sizeof (uint16_t)] << 18;
        hash += hash >> 11;
        break;
    case 2:  hash += get16bits (data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1: hash += *data;
        hash ^= hash << 10;
        hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}

/*
 * Fowler / Noll / Vo (FNV) Hash 
 * 
 * http://www.isthe.com/chongo/tech/comp/fnv/ 
 * 
 */
uint32_t 
ck_hash_fnv(const char * data, int len) {
  int i;
  uint32_t hash;

  hash = UINT32_C(2166136261);
  for (i = 0; i < len; i++)
    hash = (UINT32_C(16777619) * hash) ^ data[i];

  return hash;
}

/* 
 * AlphaNum hash.  According to Hsieh, this was created by either Chris
 * Torek or Dan Bernstein (no URL available).
 */
uint32_t 
ck_hash_alphanum(const char * s, int len) {
  uint32_t h; 
  int i;

  for (h = 0, i = 0; i < len; i++)
    h = (h << 5) + (h * 5) + (unsigned char) s[i];

  return h;
} 

/*
 * RS Hash 
 * (source: "Algorithms in C", by Robert Sedgwick)
 */
uint32_t
ck_hash_rs(char *str, size_t len) {
  uint32_t  b = 378551, 
            a = 63689,
            hash = 0;
  size_t i = 0;
 
  for (i = 0; i < len; str++, i++) {
    hash = hash * a + (unsigned char)(*str);
    a = a * b;
  }

  return hash;
}

/*
 * JS Hash Function
 */
uint32_t 
ck_hash_js(char *str, size_t len) {
  uint32_t hash = 1315423911;
  size_t i = 0;
 
  for (i = 0; i < len; str++, i++)
    hash ^= ((hash << 5) + (unsigned char)(*str) + (hash >> 2));
   
  return hash;
}

/*
 * PJ Weinberger Hash
 * (source: "Compilers (Principles, Techniques and Tools)")
 */

#define PJW_UINT_BITS (sizeof(uint32_t) * 8)
#define PJW_3_4THS    ((uint32_t) (PJW_UINT_BITS * 3 / 4))
#define PJW_1_8TH     ((uint32_t) (PJW_UINT_BITS / 8))
#define PJW_HIGH_BITS ((uint32_t) (0xFFFFFFFF << (PJW_UINT_BITS - PJW_1_8TH)))

uint32_t 
ck_hash_pjw(char *str, size_t len) {
  uint32_t hash = 0, test = 0;
  size_t i = 0;
 
  for (i = 0; i < len; str++, i++) {
    hash = (hash << PJW_1_8TH) + (unsigned char)(*str);
 
    if ((test = hash & PJW_HIGH_BITS) != 0)
      hash = ((hash ^ (test >> PJW_3_4THS)) & (~PJW_HIGH_BITS));
  }
  return hash;
}


/*
 * ELF Hash Function
 */
uint32_t 
ck_hash_elf(char *str, size_t len) {
  uint32_t hash = 0, x = 0;
  size_t i;
 
  for (i = 0; i < len; str++, i++) {
    hash = (hash << 4) + (unsigned char)(*str);
    if ((x = hash & 0xF0000000L) != 0) {
      hash ^= (x >> 24);
      hash &= ~x;
    }
  }

  return hash;
}


/*
 * BKDR Hash Function
 * (source: "The C Programming Language")
 */

#define BKDR_SEED 131313

uint32_t 
ck_hash_bkdr(char *str, size_t len) {
  uint32_t hash = 0;
  size_t i = 0;
 
  for (i = 0; i < len; str++, i++)
    hash = (hash * BKDR_SEED) + (unsigned char)(*str);

  return hash;
}


/*
 * SDBM Hash Function
 */

uint32_t 
ck_hash_sdbm(char *str, size_t len) {
  uint32_t hash = 0;
  size_t i;
 
  for (i = 0; i < len; str++, i++)
    hash = (unsigned char)(*str) + (hash << 6) + (hash << 16) - hash;

  return hash;
}


/*
 * DJB Hash Function
 */

uint32_t
ck_hash_djb(char *str, size_t len) {
  uint32_t hash = 5381;
  size_t i = 0;
 
  for (i = 0; i < len; str++, i++)
    hash = ((hash << 5) + hash) + (unsigned char)(*str);

  return hash;
}


/*
 * DEK Hash Function
 */
uint32_t 
ck_hash_dek(char *str, size_t len) {
  uint32_t hash = len;
  size_t i;
 
  for (i = 0; i < len; str++, i++)
    hash = ((hash << 5) ^ (hash >> 27)) ^ (unsigned char)(*str);

  return hash;
}


/*
 * LY Hash Function
 * (source: http://leo.yuriev.ru/random)
 */
uint32_t 
ck_hash_ly(char *str, size_t len) {
  uint32_t hash = 0;
  size_t i;
 
  for (i = 0; i < len; str++, i++)
    hash = (hash * 1664525) + (unsigned char)(*str) + 1013904223;

  return hash;
}


/*
 * ROT13 Hash Function
 */

uint32_t 
ck_hash_rot13(char *str, uint32_t len) {
  uint32_t hash = 0;
 
  while (len-- && str++) {
    hash += (unsigned char)(*str);
    hash -= (hash << 13) | (hash >> 19);
  }

  return hash;
}

/****************************/
/* HASH ALGORITHM CALLBACKS */
/****************************/

ck_err 
ck_hash_cb_hseish_jenkins3(ck_hash *hash, void *key, size_t key_len, uint32_t *ret) {
  UNUSED(hash);

  ret[0] = ck_hash_superfast(key, key_len);
  ret[1] = ck_hash_jenkins_lookup3(key, key_len);

  DEBUG("%s => [%ld, %ld]", (char*) key, (unsigned long) ret[0], (unsigned long) ret[1]);

  return CK_OK;
}

ck_err 
ck_hash_cb_jenkins_hashword2(ck_hash *hash, void *key, size_t key_len, uint32_t *ret) {
  UNUSED(hash);
  ck_hash_jenkins_hashword2(key, key_len, ret);
  return CK_OK;
}

ck_err 
ck_hash_cb_jenkins_hashlittle2(ck_hash *hash, void *key, size_t key_len, uint32_t *ret) {
  UNUSED(hash);
  ck_hash_jenkins_hashlittle2(key, key_len, ret);
  return CK_OK;
}
