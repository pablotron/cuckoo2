#include "ck_private.h"

#if 0
#define CK_USED(h) ((h)->used[0] + (h)->used[1])
#define CK_CAPA(h) ((h)->capa[0] + (h)->capa[1])
#define ck_has(h, k, kl, kz) (ck_get((h), (k), (kl), (kz), NULL) != CK_NONE)
#endif /* 0 */

static ck_err 
ck_init_with_size(ck_hash *hash, ck_hash_opt *opt, size_t size) {
  ck_err err;

  /* check for return buffer */
  if (!hash)
    return CK_ERR_NULL_HASH;

  /* set default options */
  if (!opt)
    opt = ck_get_default_opt();

  /* clear and populate hash */
  memset(hash, 0, sizeof(ck_hash));
  hash->opt = opt;

  /* calculate size and make sure it's a multiple of 3 */
  size = (size > 0) ? size : opt->default_capa;
  if (mod = (size % 3)) 
    size += 3 - mod;

  /* get size in bytes of bins */
  size = size * sizeof(ck_entry);

  /* allocate bins */
  if ((hash->bins = opt->malloc(hash, size)) == NULL)
    return CK_ERR_NOMEM;

  /* populate bin capacities */
  hash->capa[0] = 2 * size / 3;
  hash->capa[1] = size / 3;

  /* return success */
  return CK_OK;
}

ck_err 
ck_init(ck_hash *hash, ck_hash_opt *opt) {
  return ck_init_with_size(hash, opt, 0);
}

ck_err 
ck_fini(ck_hash *hash) {
  ck_err err;

  /* free bins */
  if ((err = hash->opt->free(opt, hash->bins)) != CK_OK)
    return err;

  /* clear out internal elements */
  memset(hash, 0, sizeof(ck_hash));

  /* return success */
  return CK_OK;
}

ck_err 
ck_key(ck_hash *hash, void *key, uint32_t key_len, uint32_t *ret) {
  if (!key || !ret)
    return CK_ERR_NULL_BUF;
  if (!hash)
    return CK_ERR_NULL_HASH;

  /* hash value */
  return hash->opt->hash(hash, key, key_len, ret);
}

ck_err 
ck_get(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret) {
}

ck_err 
ck_set(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void *val) {
}

ck_err ck_rm(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret) {
}
