#ifndef CK_ERROR_H
#define CK_ERROR_H

typedef enum {
  CK_OK,
  CK_NONE,
  CK_ERR_NOMEM,
  CK_ERR_NULL_HASH,
  CK_ERR_NULL_BUF,
  CK_ERR_SMALL_BUF,
  CK_ERR_BAD_ERROR,
  CK_ERR_TOO_MANY_RESIZES,
  CK_ERR_RESIZE_COLLISION,
  CK_ERR_HASH_WAS_RESIZED,
  CK_ERR_LAST
} ck_err;

ck_err ck_strerror(ck_err err, char *buf, size_t buf_len);

#endif /* CK_ERROR_H */
