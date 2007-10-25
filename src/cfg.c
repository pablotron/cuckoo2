#include "ck_private.h"

static void *
malloc_wrapper(ck_hash *hash, size_t size) {
  UNUSED(hash);
  return malloc(size);
}

static void *
realloc_wrapper(ck_hash *hash, void *ptr, size_t new_size) {
  UNUSED(hash);
  return realloc(ptr, new_size);
}

static void 
free_wrapper(ck_hash *hash, void *ptr) {
  UNUSED(hash);
  free(ptr);
}

static ck_err
do_resize_calc(ck_hash *hash, size_t *new_capa) {
#ifdef CK_SAFETY_CHECKS
  /* check for return buffer */
  if (!hash)
    return CK_ERR_NULL_HASH;
  if (!new_capa)
    return CK_ERR_NULL_BUF;
#endif /* CK_SAFETY_CHECKS */

  /* default behavior is to double the size */
  new_capa[0] = 2 * hash->capa[0];
  new_capa[1] = 2 * hash->capa[1];

  /* print out debugging information */
  DEBUG("old_capa = [%d, %d], new_capa = [%d, %d]", hash->capa[0], hash->capa[1], new_capa[0], new_capa[1]);

  /* return success */
  return CK_OK;
}

/* FIXME: i'd really like c99 initializers here */
static ck_cfg 
default_config = {
  /* flags */
  0,

  /* memory functions */
  malloc_wrapper,
  realloc_wrapper,
  free_wrapper,

  /* hashing functions */
  NULL,
  do_resize_calc,

  /* tunables */
  48,   /* default capa */
  5,    /* max_tries */
  5     /* max_resizes */
};

ck_err
ck_cfg_init(ck_cfg *ret) {
#ifdef CK_SAFETY_CHECKS
  /* check for return buffer */
  if (!cfg)
    return CK_ERR_NULL_BUF;
#endif /* CK_SAFETY_CHECKS */

  /* copy defaults */
  memcpy(ret, &default_config, sizeof(ck_cfg));

  /* return success */
  return CK_OK;
}

ck_cfg *
ck_get_default_cfg(void) {
  return &default_config; 
}
