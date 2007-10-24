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
do_resize(ck_hash *hash) {
  ck_entry *new_bins;
  size_t i, ofs, old_used, old_capa, new_capa[2], new_size;
  
  /* check for return buffer */
  if (!hash)
    return CK_ERR_NULL_HASH;

  /* calculate total old size */
  old_capa = hash->capa[0] + hash->capa[1];

  /* default behavior is to double the size */
  new_capa[0] = 2 * hash->capa[0];
  new_capa[1] = 2 * hash->capa[1];
  new_size = 2 * old_capa * sizeof(ck_entry);
   
  /* allocate and clear new bins */
  if ((new_bins = hash->cfg->malloc(hash, new_size)) == NULL)
    return CK_ERR_NOMEM;
  memset(new_bins, 0, new_size);
  
  /* add old values to new bins */
  for (i = 0; old_used > 0 && i < old_capa; i++) {
    if (!hash->bins[i].key)
      continue;

    /* calculate offset for entry in first bin */
    ofs = hash->bins[i].keys[0] % new_capa[0];

    if (!new_bins[ofs].key) {
      /* save entry */
      new_bins[ofs] = hash->bins[i];
      old_used--;

      continue;
    }

    /* calculate offset for entry in first bin */
    ofs = new_capa[0] + (hash->bins[i].keys[1] % new_capa[1]);

    if (!new_bins[ofs].key) {
      /* save entry */
      new_bins[ofs] = hash->bins[i];
      old_used--;

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
  ck_hash_cb_hseish_jenkins3,
  do_resize,

  /* tunables */
  24,   /* default capa */
  5,    /* max_tries */
  5     /* max_resizes */
};

ck_cfg *
ck_get_default_cfg(void) {
  return &default_config; 
}
