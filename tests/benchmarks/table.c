
/* for strndup() */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

#include <cuckoo/cuckoo.h>  /* for my hash tbl */
#include <glib.h>           /* for glib hash tbl */
#include <search.h>         /* for glibc hash tbl */

#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)

#define NUM_BENCHMARKS 9

extern char *strndup(const char *, size_t);

typedef enum {
  STATE_IN_SPACE,
  STATE_IN_WORD
} State;

typedef struct {
  char *ptr;
  size_t len;
  uint32_t keys[2];
} Word;

static void 
resize_words(Word **words, size_t new_size) {
  if ((*words = realloc(*words, new_size * sizeof(Word))) == NULL)
    perror("couldn't allocate memory:");
}

static void 
test_glibc_hash(Word *words, size_t num_words, struct timeval *tv) {
  size_t i, to = 0;
  ENTRY e;

  if (hcreate(num_words) == 0) {
    fprintf(stderr, "hcreate() failed\n");
    exit(EXIT_FAILURE);
  }
    

  /******************************/
  /* INSERT WORDS IN GLIBC HASH */
  /******************************/

  /* get start timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;
  
  /* insert words */
  for (i = 0; i < num_words; i++) {
    e.key = words[i].ptr;
    e.data = words[i].ptr;
    if (hsearch(e, ENTER) == NULL) {
      fprintf(stderr, "hsearch() insert failed\n");
      exit(EXIT_FAILURE);
    }
  }

  /* get end timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  printf("finished adding words to glibc hash\n");

  /*******************************/
  /* LOOK UP WORDS IN GLIBC HASH */
  /*******************************/

  /* get start timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  /* lookup words */
  for (i = 0; i < num_words; i++) {
    e.key = words[i].ptr;
    if (!hsearch(e, FIND)) {
      fprintf(stderr, "hsearch() lookup failed\n");
      exit(EXIT_FAILURE);
    }
  }

  /* get start timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  printf("finished reading words from glibc hash\n");

  /* clean up glibc hash */
  hdestroy();
}

static void 
test_glib_hash(Word *words, size_t num_words, struct timeval *tv) {
  size_t i, to = 0;
  GHashTable *hash;
  void *val;

  hash = g_hash_table_new(g_str_hash, g_str_equal);

  /*****************************/
  /* INSERT WORDS IN GLIB HASH */
  /*****************************/

  /* get start timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;
  
  /* insert words */
  for (i = 0; i < num_words; i++)
    g_hash_table_insert(hash, words[i].ptr, words[i].ptr);

  /* get end timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  printf("finished adding words to glib hash\n");

  /******************************/
  /* LOOK UP WORDS IN GLIB HASH */
  /******************************/

  /* get start timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  /* lookup words */
  for (i = 0; i < num_words; i++)
    val = g_hash_table_lookup(hash, words[i].ptr);

  /* get start timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  printf("finished reading words from glib hash\n");

  /* clean up glib hash */

  g_hash_table_destroy(hash);
}

int main(int argc, char *argv[]) {
  FILE *fh;
  char  *val, buf[4096], word_buf[1024];
  Word *words;
  size_t  i, o, to, len, word_len, num_words, max_words;
  double results[NUM_BENCHMARKS];
  State state = 0;
  ck_hash hash;
  ck_err  err;
  struct timeval tv[2 * NUM_BENCHMARKS];

  /*************************/
  /* INITIALIZE EVERYTHING */
  /*************************/

  /* check for required argument */
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* open input file */
  if ((fh = fopen(argv[1], "rb")) == NULL)
    perror("fopen failed: ");

  /* init word list */
  num_words = 0;
  max_words = 1024;
  words = NULL; 
  resize_words(&words, max_words);

  /* init hash */
  ck_init(&hash, NULL);

  /* init time */
  to = 0;

  /*************************/
  /* READ WORDS FROM STDIN */
  /*************************/

  /* get first timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  while ((len = fread(buf, 1, sizeof(buf), fh)) > 0) {
    state = STATE_IN_SPACE;

    for (i = 0, o = 0; i < len; i++) {
      switch (state) {
      case STATE_IN_SPACE:
        if (isalnum(buf[i]) || buf[i] == '-') {
          state = STATE_IN_WORD;
          o = i;
        }
        break;
      case STATE_IN_WORD:
        if (!isalnum(buf[i]) && buf[i] != '-') {
          state = STATE_IN_SPACE;

          if (i - o) {
            /* copy current word to word buffer */
            word_len = i - o + 1;
            memcpy(word_buf, buf + o, word_len);
            word_buf[i - o] = '\0';

            /* copy word buffer to word list */
            words[num_words].ptr = strndup(word_buf, word_len);
            words[num_words].len = word_len;

            /* hash word */
            err = ck_key(&hash, word_buf, word_len, words[num_words].keys);
            if (err  != CK_OK) {
              ck_strerror(err, buf, sizeof(buf));
              fprintf(stderr, "couldn't hash word \"%s\": %s\n", word_buf, buf);
              exit(EXIT_FAILURE);
            }

            /* dump word */
            /* fprintf(stderr, "got \"%s\"\n", word_buf); */

            /* increment word counter */
            num_words++;

            /* resize word list (if necessary) */
            if (num_words == max_words) {
              max_words *= 2;
              resize_words(&words, max_words);
            }
          }
        }
        break;
      default:
        fprintf(stderr, "unknown state: %d\n", state);
        exit(-1);
      }
    }
  }

  /* get second timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  /* print out stats */
  fclose(fh);
  LOG("read %d words", num_words);

/* 
 *   LOG("first 10 words:");
 *   for (i = 0; i < 10; i++)
 *     LOG("%02d. %s (%d)", i, words[i].ptr, words[i].len);
 * 
 *   LOG("last 10 words:\n");
 *   for (i = num_words - 10; i < num_words; i++)
 *     LOG("%06d. %s (%d)", i, words[i].ptr, words[i].len);
 */ 

  /**************************/
  /* INSERT WORDS INTO HASH */
  /**************************/

  /* get third timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  for (i = 0; i < num_words; i++) {
    /* LOG("%03d. adding word \"%s\" (%d)", i, words[i].ptr, words[i].len); */

    /* insert word into hash table */
    err = ck_set(&hash, words[i].ptr, words[i].len, NULL, words[i].ptr);

    /* check for error */
    if (err != CK_OK) {
      ck_strerror(err, buf, sizeof(buf));
      fprintf(stderr, "couldn't insert word \"%s\": %s", words[i].ptr, buf);
      exit(EXIT_FAILURE);
    }
  }
  
  /* get fourth timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  LOG("finished adding words to hash");

  /************************/
  /* READ WORDS FROM HASH */
  /************************/

  /* get fifth timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  for (i = 0; i < num_words; i++) {
    if ((err = ck_get(&hash, words[i].ptr, words[i].len, NULL, (void*) &val)) != CK_OK) {
      ck_strerror(err, buf, sizeof(buf));
      fprintf(stderr, "couldn't get word \"%s\": %s", words[i].ptr, buf);
      exit(EXIT_FAILURE);
    }
  }

  /* get sixth timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  printf("finished reading words from hash\n");

  /* clean up hash */
  ck_fini(&hash);

  /* create new hash */
  ck_init(&hash, NULL);

  /*********************************/
  /* INSERT WORDS INTO SECOND HASH */
  /*********************************/

  /* get seventh timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  for (i = 0; i < num_words; i++) {
    /* LOG("%03d. adding word \"%s\" (%d)", i, words[i].ptr, words[i].len); */

    /* insert word into hash table */
    err = ck_set(&hash, words[i].ptr, words[i].len, words[i].keys, words[i].ptr);

    /* check for error */
    if (err != CK_OK) {
      ck_strerror(err, buf, sizeof(buf));
      fprintf(stderr, "couldn't insert word \"%s\": %s", words[i].ptr, buf);
      exit(EXIT_FAILURE);
    }
  }
  
  /* get eighth timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  LOG("finished adding words to second hash");

  /*******************************/
  /* READ WORDS FROM SECOND HASH */
  /*******************************/

  /* get nineth timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  for (i = 0; i < num_words; i++) {
    if ((err = ck_get(&hash, words[i].ptr, words[i].len, words[i].keys, (void*) &val)) != CK_OK) {
      ck_strerror(err, buf, sizeof(buf));
      fprintf(stderr, "couldn't get word \"%s\": %s", words[i].ptr, buf);
      exit(EXIT_FAILURE);
    }
  }

  /* get tenth timestamp */
  if (gettimeofday(tv + to, NULL) == -1)
    perror("gettimeofday() failed: ");
  to++;

  printf("finished reading words from second hash\n");

  test_glib_hash(words, num_words, tv + to);
  to += 4;

  test_glibc_hash(words, num_words, tv + to);
  to += 4;

  /********************/
  /* PRINT BENCHMARKS */
  /********************/

  for (i = 0; i < NUM_BENCHMARKS; i++)
    results[i] = tv[2 * i + 1].tv_sec - tv[2 * i].tv_sec + 
                 (tv[2 * i + 1].tv_usec - tv[2 * i].tv_usec) / 1000000.0;

  fprintf(stderr, 
    "Results (%d words):\n"
    "load (from disk):    %4.3fsec (%2.5fusec/word)\n"
    "insert (hashed):     %4.3fsec (%2.5fusec/word)\n"
    "lookup (hashed):     %4.3fsec (%2.5fusec/word)\n"
    "insert (pre-hashed): %4.3fsec (%2.5fusec/word)\n"
    "lookup (pre-hashed): %4.3fsec (%2.5fusec/word)\n"
    "insert (glib):       %4.3fsec (%2.5fusec/word)\n"
    "lookup (glib):       %4.3fsec (%2.5fusec/word)\n"
    "insert (glibc):      %4.3fsec (%2.5fusec/word)\n"
    "lookup (glibc):      %4.3fsec (%2.5fusec/word)\n"
    "\n",

    num_words,
    results[0], results[0] / num_words * 1000000,
    results[1], results[1] / num_words * 1000000,
    results[2], results[2] / num_words * 1000000,
    results[3], results[3] / num_words * 1000000,
    results[4], results[4] / num_words * 1000000,
    results[5], results[5] / num_words * 1000000,
    results[6], results[6] / num_words * 1000000,
    results[5], results[7] / num_words * 1000000,
    results[6], results[8] / num_words * 1000000
  );

  /* return success */
  return EXIT_SUCCESS;
}
