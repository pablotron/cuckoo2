#ifndef CK_HASH_OPT_H
#define CK_HASH_OPT_H

typedef struct {
  uint32_t flags;

  void *(*malloc)(ck_hash *, size_t);
  void *(*realloc)(ck_hash *, void *, size_t);
  void (*free)(ck_hash *, void *);

  /* hash callback */
  ck_err (*hash)(ck_hash *, void *, size_t, uint32_t *);

  /* resize callback */
  ck_err (*resize)(ck_hash *);

  size_t default_capa,
         max_tries,
         max_resizes;
} ck_hash_opt;

ck_hash_opt *ck_get_default_opt(void);

#endif /* CK_HASH_OPT_H */
