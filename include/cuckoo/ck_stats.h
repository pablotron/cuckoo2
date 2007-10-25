#ifndef CK_STATS_H
#define CK_STATS_H

typedef enum {
  /* number of collisions since the last resize */
  CK_STAT_NUM_COLS,

  /* total number of collisions, regardless of resizes */
  CK_STAT_TOTAL_COLS,

  /* number of resizes due to collisions */
  CK_STAT_NUM_COL_RESIZES
} ck_stat_t;
#define CK_STAT_LAST 3

ck_err ck_stat_describe(ck_stat_t s, char *buf, size_t buf_len);

#endif /* CK_STATS_H */
