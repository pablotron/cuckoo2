
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
		case 3:	hash += get16bits (data);
				hash ^= hash << 16;
				hash ^= data[sizeof (uint16_t)] << 18;
				hash += hash >> 11;
				break;
		case 2:	hash += get16bits (data);
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
	for (i=0; i < len; i++)
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
