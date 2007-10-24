#include "ck_private.h"

static ck_err 
ck_init_with_size(ck_hash *hash, ck_cfg *cfg, size_t size) {
  uint32_t mod;

  /* check for return buffer */
  if (!hash)
    return CK_ERR_NULL_HASH;

  /* set default cfgions */
  if (!cfg)
    cfg = ck_get_default_cfg();

  /* clear and populate hash */
  memset(hash, 0, sizeof(ck_hash));
  hash->cfg = cfg;

  /* calculate size and make sure it's a multiple of 3 */
  size = (size > 0) ? size : cfg->default_capa;
  if ((mod = (size % 3)) != 0) 
    size += 3 - mod;

  /* get size in bytes of bins */
  size = size * sizeof(ck_entry);

  /* allocate bins */
  if ((hash->bins = (*(cfg->malloc))(hash, size)) == NULL)
    return CK_ERR_NOMEM;

  /* populate bin capacities */
  hash->capa[0] = 2 * size / 3;
  hash->capa[1] = size / 3;

  /* return success */
  return CK_OK;
}

ck_err 
ck_init(ck_hash *hash, ck_cfg *cfg) {
  return ck_init_with_size(hash, cfg, 0);
}

ck_err 
ck_fini(ck_hash *hash) {
  /* FIXME: add null check */

  /* free bins */
  (*(hash->cfg->free))(hash, hash->bins);

  /* clear out internal elements */
  memset(hash, 0, sizeof(ck_hash));

  /* return success */
  return CK_OK;
}

ck_err 
ck_key(ck_hash *hash, void *key, uint32_t key_len, uint32_t *ret) {
  if (!key || !key_len || !ret)
    return CK_ERR_NULL_BUF;
  if (!hash)
    return CK_ERR_NULL_HASH;

  /* hash value */
  return hash->cfg->hash(hash, key, key_len, ret);
}

static ck_err
do_get_entry(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, ck_entry **ret) {
  uint32_t i, k[2];
  ck_err err;
  ck_entry *e;

  /* hash key */
  if (!keys) {
    if ((err = ck_key(hash, key, key_len, k)) != CK_OK)
      return err;
    keys = k;
  }

  /* check each bin */
  for (i = 0; i < 2; i++) {
    /* get matching entry from bin */
    e = hash->bins + (i * hash->capa[i]) + (keys[i] % hash->capa[i]);

    /* check entry */
    if (e->key && e->keys[i] == keys[i] && e->key_len == key_len && memcmp(key, e->key, key_len)) {
      /* save return value */
      if (ret) 
        *ret = e;

      /* return success */
      return CK_OK;
    }
  }

  /* return not found */
  return CK_NONE;
}

ck_err 
ck_get(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret) {
  ck_err err;
  ck_entry *e;

  /* null checks */
  if (!key || !key_len || !keys)
    return CK_ERR_NULL_BUF;
  if (!hash)
    return CK_ERR_NULL_HASH;

  /* find entry */
  if ((err = do_get_entry(hash, key, key_len, keys, &e)) != CK_OK)
    return err;

  /* save return value */
  if (ret) 
    *ret = e->val;

  /* return success */
  return CK_OK;
}

ck_err 
ck_set(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void *val) {
  uint32_t i, k[2], num_tries, num_resizes;
  ck_err err;
  ck_entry *e, oe, ne;

  /* hash key */
  if (!keys) {
    if ((err = ck_key(hash, key, key_len, k)) != CK_OK)
      return err;
    keys = k;
  }

  /* populate new entry */
  ne.key = key;
  ne.key_len = key_len;
  ne.val = val;
  ne.keys[0] = ne.keys[0];
  ne.keys[1] = ne.keys[1];

  /* set the number of resizes */
  num_resizes = hash->cfg->max_resizes;

  do {
    /* set/reset number of tries (insert attempts) */
    num_tries = hash->cfg->max_tries;

    do {
      /* check each bin */
      for (i = 0; i < 2; i++) {
        /* get matching entry from bin */
        e = hash->bins + (ne.keys[i] % hash->capa[i]);

        /* save old entry and write new entry */
        oe = *e;
        *e = ne;
        ne = oe;

        /* if the old entry was empty or the keys for the old and new
         * entry were identical, then return success */
        if (!ne.key || (e->keys[0] == ne.keys[0] && e->keys[1] == ne.keys[1]))
          return CK_OK;

        /* increment the collision counters */
        hash->stats[CK_STAT_NUM_COLS]++;
        hash->stats[CK_STAT_TOTAL_COLS]++;
      }
    } while (num_tries-- > 0);

    /* if we've made it this far then we have an entry in @ne 
     * that has collided more than hash->cfg->max_tries times, 
     * so we need to resize the bins and try again */
    /* decriment the number of resizes */
    num_resizes--;

    if (num_resizes > 0) {
      /* clear per-resize collision counter */
      hash->stats[CK_STAT_NUM_COLS] = 0;

      /* increment the resize counter */
      hash->stats[CK_STAT_NUM_COL_RESIZES]++;

      /* resize bins */
      if ((err = (*(hash->cfg->resize))(hash)) != CK_OK) {
        /* couldn't resize the bins; save the failed entry and return
         * the error */

        /* XXX: is there a better way to return the failed entry? this
         * isn't particularly thread-safe */
        hash->failed_entry = ne;
        return err;
      }
    }
  } while (num_resizes-- > 0);

  /* at this point we have tried resizing the hash max_retries times, so
   * something is wonky, return an error */

  /* XXX: is there a better way to return the failed entry? this
   * isn't particularly thread-safe */
  hash->failed_entry = ne;
  return CK_ERR_TOO_MANY_RESIZES;
}

ck_err 
ck_rm(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret) {
  ck_err err;
  ck_entry *e;

  /* null checks */
  if (!key || !key_len || !keys)
    return CK_ERR_NULL_BUF;
  if (!hash)
    return CK_ERR_NULL_HASH;

  err = do_get_entry(hash, key, key_len, keys, &e);
  if (err == CK_NONE)
    return CK_OK;
  if (err != CK_OK)
    return err;

  /* save return value */
  if (ret)
    *ret = e->val;

  /* clear entry */
  e->key = NULL;
  
  /* return success */
  return CK_OK;
}
