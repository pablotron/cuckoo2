#ifndef CK_HASH_H
#define CK_HASH_H

struct ck_hash_st {
  uint32_t flags;
  ck_cfg   *cfg;

  /* entry tables */
  ck_entry *bins;
  size_t    used[2], 
            capa[2];

  ck_entry  failed_entry;

  uint32_t  stats[CK_STAT_LAST];
};

#define CK_HASH(h) ((ck_hash*) (h))
#define CK_USED(h) ((h)->used[0] + (h)->used[1])
#define CK_CAPA(h) ((h)->capa[0] + (h)->capa[1])
#define CK_HAS(h, k, kl, kz) (ck_get((h), (k), (kl), (kz), NULL) != CK_NONE)

ck_err ck_init(ck_hash *hash, ck_cfg *);
ck_err ck_fini(ck_hash *hash);

ck_err ck_key(ck_hash *hash, void *key, uint32_t key_len, uint32_t *ret);

ck_err ck_get(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret);
ck_err ck_set(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void *val);
ck_err ck_rm(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret);

ck_err ck_dump(ck_hash *hash, FILE *io);

#endif /* CK_HASH_H */
