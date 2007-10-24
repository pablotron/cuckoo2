#ifndef CK_ALCOS_H
#define CK_ALCOS_H

/******************************/
/* INDIVIDUAL HASH ALGORITHMS */
/******************************/
uint32_t ck_hash_superfast(const char *, int);
uint32_t ck_hash_fnv(const char *, int);
uint32_t ck_hash_alphanum(const char *, int);
uint32_t ck_hash_jenkins_lookup3(const void *, size_t);
void ck_hash_jenkins_hashword2(const void *data, size_t len, uint32_t *ret);

/****************************/
/* HASH ALGORITHM CALLBACKS */
/****************************/
ck_err ck_hash_cb_hseish_jenkins3(ck_hash *, void *, size_t, uint32_t *);
ck_err ck_hash_cb_jenkins_hashword2(ck_hash *, void *, size_t, uint32_t *);

#endif /* CK_ALCOS_H */
