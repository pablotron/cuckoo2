#ifndef CK_ITER_H
#define CK_ITER_H

typedef struct {
  ck_hash *hash;
  size_t   num_resizes, 
           ofs;
} ck_iter;

ck_err ck_iter_init(ck_iter *, ck_hash *);
ck_err ck_iter_next(ck_iter *, ck_entry **);
ck_err ck_iter_reset(ck_iter *);

#endif /* CK_ITER_H */
