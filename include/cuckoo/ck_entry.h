#ifndef CK_ENTRY_H
#define CK_ENTRY_H

typedef struct {
  uint32_t keys[2], 
           key_len;
  void    *key, 
          *val;
} ck_entry;

#endif /* CK_ENTRY_H */
