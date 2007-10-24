#ifndef CK_HASH_H
#define CK_HASH_H

#include <stdint.h>
#include <unistd.h>

typedef struct ck_hash_st ck_hash;

typedef enum {
  CK_OK,
  CK_NONE,
  CK_ERR_LAST
} ck_err;

ck_err ck_strerror(ck_err err, char *buf, size_t buf_len);

typedef struct {
  uint32_t keys[2];
  void     *val;
} ck_entry;

typedef struct {
  uint32_t flags;
  ck_err (*cb)(void *, size_t, uint32_t *, void *);

  ck_err (*malloc)(ck_hash *, size_t);
  ck_err (*realloc)(ck_hash *, size_t);
  ck_err (*free)(ck_hash *, void *);

  size_t growth_factor,
         default_capa[2];
} ck_hash_opt;

struct ck_hash_st {
  uint32_t flags;
  ck_hash_opt *opt;

  /* entry tables */
  ck_entry *bins[2];
  size_t    used[2], 
            capa[2];

  uint32_t  stats;
};

#define CK_HAHS(h) ((ck_hash*) (h))
#define CK_USED(h) ((h)->used[0] + (h)->used[1])
#define CK_CAPA(h) ((h)->capa[0] + (h)->capa[1])

ck_err ck_new(ck_hash_opt *, ck_hash *ret);
ck_err ck_free(ck_hash *hash);

ck_err ck_key(ck_hash *hash, void *key, uint32_t key_len, uint32_t *ret);

ck_err ck_get(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret);
ck_err ck_set(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void *val);
ck_err ck_rm(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret);

#define ck_has(h, k, kl, kz) (ck_get((h), (k), (kl), (kz), NULL) != CK_NONE)

#endif /* CK_HASH_H */
