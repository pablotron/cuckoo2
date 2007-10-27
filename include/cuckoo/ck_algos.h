#ifndef CK_ALCOS_H
#define CK_ALCOS_H

/******************************/
/* INDIVIDUAL HASH ALGORITHMS */
/******************************/
uint32_t ck_hash_superfast(char *, size_t);
uint32_t ck_hash_fnv(char *, size_t);
uint32_t ck_hash_alphanum(char *, size_t);
uint32_t ck_hash_jenkins_lookup3(char *, size_t);

uint32_t ck_hash_rs(char *str, size_t len);
uint32_t ck_hash_js(char *str, size_t len);
uint32_t ck_hash_pjw(char *str, size_t len);
uint32_t ck_hash_elf(char *str, size_t len);
uint32_t ck_hash_bkdr(char *str, size_t len);
uint32_t ck_hash_sdbm(char *str, size_t len);
uint32_t ck_hash_djb(char *str, size_t len);
uint32_t ck_hash_dek(char *str, size_t len);
uint32_t ck_hash_ly(char *str, size_t len);
uint32_t ck_hash_rot13(char *str, size_t len);

ck_err ck_hash_jenkins_hashlittle2(const void *, size_t, uint32_t *);
ck_err ck_hash_jenkins_hashword2(const void *data, size_t len, uint32_t *ret);

/****************************/
/* HASH ALGORITHM CALLBACKS */
/****************************/
ck_err ck_hash_cb_hseish_jenkins3(ck_hash *, void *, size_t, uint32_t *);
ck_err ck_hash_cb_jenkins_hashword2(ck_hash *, void *, size_t, uint32_t *);
ck_err ck_hash_cb_jenkins_hashlittle2(ck_hash *, void *, size_t, uint32_t *);


#endif /* CK_ALCOS_H */
