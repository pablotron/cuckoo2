#ifndef CK_CFG_H
#define CK_CFG_H

typedef struct {
  uint32_t flags;

  void *(*malloc)(ck_hash *, size_t);
  void *(*realloc)(ck_hash *, void *, size_t);
  void (*free)(ck_hash *, void *);

  /* hash callback */
  ck_err (*hash)(ck_hash *, void *, size_t, uint32_t *);

  /* resize callback */
  ck_err (*resize)(ck_hash *, size_t *);

  size_t default_capa,
         max_tries,
         max_resizes;
} ck_cfg;

ck_cfg *ck_get_default_cfg(void);

#endif /* CK_CFG_H */
