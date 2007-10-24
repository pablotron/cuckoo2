#ifndef CK_ERROR_H
#define CK_ERROR_H

typedef enum {
  CK_OK,
  CK_NONE,
  CK_ERR_LAST
} ck_err;

ck_err ck_strerror(ck_err err, char *buf, size_t buf_len);

#endif /* CK_ERROR_H */
