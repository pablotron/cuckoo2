#include "ck_private.h"

static char *err_strs[] = {
  "ok",
  "not found",
  "no memory",
  "null hash pointer",
  "null or empty buffer",
  "small buffer",
  "invalid error code",
  "too many resizes",
  "collision during resize",
  "hash was resized (iterator invalid)",
  NULL
};

ck_err
ck_strerror(ck_err err, char *buf, size_t buf_len) {
  char *str;
  size_t len;

  /* check error code */
  if (err >= CK_ERR_LAST)
    return CK_ERR_BAD_ERROR;

  /* check dest buffer */
  if (!buf)
    return CK_ERR_NULL_BUF;
  
  /* get error string */
  str = err_strs[err];
  len = strlen(str) + 1;

  /* check dest buffer length */
  if (buf_len < len)
    return CK_ERR_SMALL_BUF;

  /* copy error string and return success */
  memcpy(buf, str, len);
  return CK_OK;
}
