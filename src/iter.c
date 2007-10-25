#include "ck_private.h"

ck_err
ck_iter_init(ck_iter *ret, ck_hash *hash) {
#ifdef CK_SAFETY_CHECKS
  if (!hash)
    return CK_ERR_NULL_HASH;
  if (!ret)
    return CK_ERR_NULL_BUF;
#endif /* CK_SAFETY_CHECKS */

  memset(ret, 0, sizeof(ck_iter));
  ret->hash = hash;

  /* return success */
  return ck_iter_reset(ret);
}

ck_err
ck_iter_next(ck_iter *iter, ck_entry **ret) {
#ifdef CK_SAFETY_CHECKS
  if (!hash)
    return CK_ERR_NULL_HASH;
  if (!iter)
    return CK_ERR_NULL_BUF;
#endif /* CK_SAFETY_CHECKS */

  /* check to see if hash was resized */
  if (iter->num_resizes != iter->hash->stats[CK_STAT_NUM_RESIZES])
    return CK_ERR_HASH_WAS_RESIZED;

  for (; iter->ofs < CK_CAPA(iter->hash); iter->ofs++) {
    if (!iter->hash->bins[iter->ofs].key)
      continue;

    /* save return values */
    if (ret)
      *ret = iter->hash->bins + iter->ofs;

    /* pre-increment offset (so we don't get stuck on one value) */
    iter->ofs++;

    /* return success */
    return CK_OK;
  }
  
  /* check to see if we're at the end of the hash */
  if (iter->ofs >= CK_CAPA(iter->hash))
    return CK_NONE;

  /* return success */
  return CK_OK;
}

ck_err
ck_iter_reset(ck_iter *iter) {
#ifdef CK_SAFETY_CHECKS
  if (!hash)
    return CK_ERR_NULL_HASH;
  if (!iter)
    return CK_ERR_NULL_BUF;
#endif /* CK_SAFETY_CHECKS */

  /* reset iterator */
  iter->ofs = 0;
  iter->num_resizes = iter->hash->stats[CK_STAT_NUM_RESIZES];

  /* return success */
  return CK_OK;
}
