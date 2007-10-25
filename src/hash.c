#include "ck_private.h"

static ck_err 
ck_init_with_size(ck_hash *hash, ck_cfg *cfg, size_t size) {
  uint32_t mod;

#ifdef CK_SAFETY_CHECKS
  /* check for return buffer */
  if (!hash)
    return CK_ERR_NULL_HASH;
#endif /* CK_SAFETY_CHECKS */

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

  /* allocate bins */
  if ((hash->bins = (*(cfg->malloc))(hash, size * sizeof(ck_entry))) == NULL)
    return CK_ERR_NOMEM;
  memset(hash->bins, 0, size * sizeof(ck_entry));

  /* populate bin capacities */
  hash->capa[0] = 2 * size / 3;
  hash->capa[1] = size / 3;
  DEBUG("capa = [%d, %d]", hash->capa[0], hash->capa[1]);

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

#define DEFAULT_HASH ck_hash_jenkins_hashlittle2 
#define KEY(h, k, kl, ret, ret_err)                       \
  (ret_err) = (h)->cfg->hash ?                            \
              (*((h)->cfg->hash))((h), (k), (kl), (ret)) : \
              DEFAULT_HASH((k), (kl), (ret))

ck_err 
ck_key(ck_hash *hash, void *key, uint32_t key_len, uint32_t *ret) {
  ck_err err;

#ifdef CK_SAFETY_CHECKS
  if (!key || !key_len || !ret)
    return CK_ERR_NULL_BUF;
  if (!hash)
    return CK_ERR_NULL_HASH;
#endif /* CK_SAFETY_CHECKS */

  /* hash value */
  KEY(hash, key, key_len, ret, err);

  /* return error */
  return err;
}

#define GET_ENTRY(h, k, kl, pre_keys, ret, ret_err) do {                  \
  ck_entry *te[2];                                                        \
  uint32_t i, tk[2];                                                      \
                                                                          \
  if (!(pre_keys)) {                                                      \
    KEY((h), (k), (kl), tk, (ret_err));                                   \
    if ((ret_err) != CK_OK)                                               \
      return (ret_err);                                                   \
                                                                          \
    (pre_keys) = tk;                                                      \
  }                                                                       \
                                                                          \
  (ret_err) = CK_NONE;                                                    \
                                                                          \
  for (i = 0; i < 2; i++) {                                               \
    te[i] = (h)->bins +                                                   \
            (h)->capa[0] * i +                                            \
            ((pre_keys)[i] % (h)->capa[i]);                               \
                                                                          \
    if (te[i]->key && te[i]->key_len == (kl) &&                           \
        te[i]->keys[i] == (pre_keys)[i] &&                                \
        (te[i]->key == (k) || !memcmp(te[i]->key, (k), (kl)))) {          \
      if (ret)                                                            \
         *(ret) = te[i];                                                  \
                                                                          \
      (ret_err) = CK_OK;                                                  \
      break;                                                              \
    }                                                                     \
  }                                                                       \
} while (0)


static ck_err
do_resize(ck_hash *hash) {
  ck_entry *new_bins;
  ck_err err;
  size_t i, ofs, old_used, old_capa, 
         new_capa[2], new_size;
  
#ifdef CK_SAFETY_CHECKS
  /* check for return buffer */
  if (!hash)
    return CK_ERR_NULL_HASH;
#endif /* CK_SAFETY_CHECKS */
  
  /* calculate new size */
  if ((err = (*(hash->cfg->resize))(hash, new_capa)) != CK_OK)
    return err;
     
#ifdef CK_DEBUG
  DEBUG("old_capa = [%d, %d], new_capa = [%d, %d]", hash->capa[0], hash->capa[1], new_capa[0], new_capa[1]);

  DEBUG("old hash (before resize):");
  ck_dump(hash, stderr);
#endif /* CK_DEBUG */

  /* calculate total old capacity (in entries), total 
   * old used (in entries), and new total size (in bytes) */
  old_capa = hash->capa[0] + hash->capa[1];
  old_used = CK_USED(hash);
  new_size = (new_capa[0] + new_capa[1]) * sizeof(ck_entry);

  /* allocate and clear new bins */
  if ((new_bins = (*(hash->cfg->malloc))(hash, new_size)) == NULL)
    return CK_ERR_NOMEM;
  memset(new_bins, 0, new_size);
  
  /* add old values to new bins */
  for (i = 0; i < old_capa; i++) {
    if (!hash->bins[i].key)
      continue;

    /* calculate offset for entry in first bin */
    ofs = hash->bins[i].keys[0] % new_capa[0];

    if (!new_bins[ofs].key) {
      /* save entry */
      new_bins[ofs] = hash->bins[i];
      old_used--;

      DEBUG("moving %s:%s from %d to %d", 
            (char*) new_bins[ofs].key, 
            (char*) new_bins[ofs].val, 
            i, ofs);
      DEBUG("old entry = {keys:[%ld, %ld], key:\"%s\", val:\"%s\"}\n"  
            "new entry = {keys:[%ld, %ld], key:\"%s\", val:\"%s\"}", 

            (unsigned long) hash->bins[i].keys[0], 
            (unsigned long) hash->bins[i].keys[1], 
            (char*) hash->bins[i].key, 
            (char*) hash->bins[i].val,

            (unsigned long) new_bins[ofs].keys[0], 
            (unsigned long) new_bins[ofs].keys[1], 
            (char*) new_bins[ofs].key, 
            (char*) new_bins[ofs].val
      ); 


      continue;
    }

    /* calculate offset for entry in second bin */
    ofs = new_capa[0] + (hash->bins[i].keys[1] % new_capa[1]);

    if (!new_bins[ofs].key) {
      /* save entry */
      new_bins[ofs] = hash->bins[i];
      old_used--;

      DEBUG("moving (2) %s:%s from %d to %d", 
            (char*) new_bins[ofs].key, 
            (char*) new_bins[ofs].val, 
            i, ofs);
      DEBUG("entry (2) = {keys:[%ld, %ld], key:\"%s\", val:\"%s\"}", 
            (unsigned long) new_bins[ofs].keys[0], 
            (unsigned long) new_bins[ofs].keys[1], 
            (char*) new_bins[ofs].key, 
            (char*) new_bins[ofs].val); 


      continue;
    }

    /* if we got here then we already have a collision, abort with an
     * error */
    /* FIXME: there should be a more graceful way of handling this */
    (*(hash->cfg->free))(hash, new_bins);
    return CK_ERR_RESIZE_COLLISION;
  }

  /* if we got here then we've succcessfully repopulated the new bins,
   * so save the new capacity and free the old bins */
  hash->capa[0] = new_capa[0];
  hash->capa[1] = new_capa[1];
  
  (*(hash->cfg->free))(hash, hash->bins);
  hash->bins = new_bins;

#ifdef CK_DEBUG
  DEBUG("new hash (after resize):");
  ck_dump(hash, stderr);
#endif /* CK_DEBUG */

  /* return success */
  return CK_OK;
}

ck_err 
ck_get(ck_hash *hash, void *key, uint32_t key_len, uint32_t *keys, void **ret) {
  ck_err err;
  ck_entry *e;

#ifdef CK_SAFETY_CHECKS
  /* null checks */
  if (!key || !key_len)
    return CK_ERR_NULL_BUF;
  if (!hash)
    return CK_ERR_NULL_HASH;
#endif /* CK_SAFETY_CHECKS */

  /* find entry */
  GET_ENTRY(hash, key, key_len, keys, &e, err);
  if (err != CK_OK)
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
    KEY(hash, key, key_len, k, err);
    if (err != CK_OK)
      return err;
    keys = k;

    DEBUG("%s => [%d, %d]", (char*) key, keys[0] % hash->capa[0], keys[1] % hash->capa[1]);
  }

  /* populate new entry */
  ne.key = key;
  ne.key_len = key_len;
  ne.val = val;
  ne.keys[0] = keys[0];
  ne.keys[1] = keys[1];

  /* set the number of resizes */
  num_resizes = hash->cfg->max_resizes;

  do {
    /* set/reset number of tries (insert attempts) */
    num_tries = hash->cfg->max_tries;

    do {
      /* check each bin */
      for (i = 0; i < 2; i++) {
        /* get matching entry from bin */
        e = hash->bins + (i ? hash->capa[0] : 0) + (ne.keys[i] % hash->capa[i]);
        DEBUG("e = %p, bins = %p", (void*) e, (void*) hash->bins);

        /* save old entry and write new entry */
        oe = *e;
        *e = ne;
        ne = oe;

        /* if the old entry was empty or the keys for the old and new
         * entry were identical, then return success */
        if (!ne.key || 
            (e->key == ne.key && e->key_len == ne.key_len) || 
            (e->keys[0] == ne.keys[0] && e->keys[1] == ne.keys[1])) {
          DEBUG("setting %d to %s:%s", e - hash->bins, (char*) e->key, (char*) e->val);
          hash->used++;

#ifdef CK_DEBUG
          ck_dump(hash, stderr);
#endif /* CK_DEBUG */

#ifdef CK_TRACK_STATS
          hash->stats[CK_STAT_NUM_INSERTS]++;
#endif /* CK_TRACK_STATS */
  
          return CK_OK;
        }

#ifdef CK_TRACK_STATS
        /* increment the collision counters */
        hash->stats[CK_STAT_NUM_COLS]++;
        hash->stats[CK_STAT_TOTAL_COLS]++;
#endif /* CK_TRACK_STATS */
      }
    } while (num_tries-- > 0);

    /* if we've made it this far then we have an entry in @ne 
     * that has collided more than hash->cfg->max_tries times, 
     * so we need to resize the bins and try again */
    /* decriment the number of resizes */
    num_resizes--;

    if (num_resizes > 0) {
      /* clear per-resize collision counter */
#ifdef CK_TRACK_STATS
      hash->stats[CK_STAT_NUM_COLS] = 0;
#endif /* CK_TRACK_STATS */

      /*
       * increment the resize counter
       * 
       * NOTE: we _always_ increment the resize counter, even with stat
       * tracking disabled.  this is because the iterator interface uses
       * the number of resizes to determine if it's synchronized with
       * the hash
       * 
       */
      hash->stats[CK_STAT_NUM_RESIZES]++;

      /* resize bins */
      DEBUG("resizing bins (%d resizes remaining)", num_resizes);
      if ((err = do_resize(hash)) != CK_OK) {
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

#ifdef CK_SAFETY_CHECKS
  /* null checks */
  if (!key || !key_len || !keys)
    return CK_ERR_NULL_BUF;
  if (!hash)
    return CK_ERR_NULL_HASH;
#endif /* CK_SAFETY_CHECKS */

  GET_ENTRY(hash, key, key_len, keys, &e, err);
  if (err == CK_NONE)
    return CK_OK;
  if (err != CK_OK)
    return err;

  /* save return value */
  if (ret)
    *ret = e->val;

  /* clear entry */
  e->key = NULL;

  hash->used--;

#ifdef CK_TRACK_STATS
  hash->stats[CK_STAT_NUM_DELETES]++;
#endif /* CK_TRACK_STATS */
  
  /* return success */
  return CK_OK;
}

ck_err
ck_dump(ck_hash *hash, FILE *io) {
  size_t i, capa;

#ifdef CK_SAFETY_CHECKS
  if (!hash)
    return CK_ERR_NULL_HASH;
#endif /* CK_SAFETY_CHECKS */

  /* print all entries to io stream */
  capa = hash->capa[0] + hash->capa[1];

  fprintf(
    io, 
    "{used: %d, capa: [%d, %d], stats: {cols: %d, total_cols: %d, resizes: %d}}\n",
    hash->used, hash->capa[0], hash->capa[1],
    hash->stats[CK_STAT_NUM_COLS],
    hash->stats[CK_STAT_TOTAL_COLS],
    hash->stats[CK_STAT_NUM_RESIZES] 
  );

  for (i = 0; i < capa; i++)
    if (hash->bins[i].key)
      fprintf(io, "%03d:%s:%s\n", i, (char*) hash->bins[i].key, (char*) hash->bins[i].val);

  /* return success */
  return CK_OK;
}
