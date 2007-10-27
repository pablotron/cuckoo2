
/* for strndup() */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

#include <cuckoo/cuckoo.h>  /* for my hash tbl */

#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)

#define NUM_BITS (sizeof(uint32_t) * 8)

extern char *strndup(const char *, size_t);

typedef enum {
  STATE_IN_SPACE,
  STATE_IN_WORD
} State;

typedef uint32_t (*HashFn)(char *, size_t);

#define ZEROS {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define HASH(a) { #a, ck_hash_##a, ZEROS, 0.0 }
static struct {
  char   *name;
  HashFn  fn;
  int     bias[NUM_BITS];
  double  time;
} hashes[] = {
  HASH(superfast),
  HASH(fnv),
  HASH(alphanum),
  HASH(jenkins_lookup3),
  HASH(rs),
  HASH(js),
  HASH(pjw),
  HASH(elf),
  HASH(bkdr),
  HASH(sdbm),
  HASH(djb),
  HASH(dek),
  HASH(ly),
  HASH(rot13),

  { NULL, NULL, ZEROS, 0.0 }
};
#undef HASH
#undef ZEROS

typedef struct {
  char *ptr;
  size_t len;
} Word;

static void 
resize_words(Word **words, size_t new_size) {
  if ((*words = realloc(*words, new_size * sizeof(Word))) == NULL)
    perror("couldn't allocate memory:");
}

static void
test_bias(Word *words, size_t num_words, HashFn fn, int *bias) {
  size_t i, j;
  uint32_t hash;

  for (i = 0; i < num_words; i++) {
    hash = (*fn)(words[i].ptr, words[i].len);

    for (j = 0; j < NUM_BITS; j++)
      bias[j] += (hash & (1 << j)) ? 1 : -1;
  }
}

int main(int argc, char *argv[]) {
  Word *words;
  State state = 0;
  FILE *fh;
  struct timeval load_tv[2], bias_tv[2];
  char  buf[4096], word_buf[1024];
  size_t  i, j, o, len, word_len, num_words, max_words;
  double load_time;

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

  /*************************/
  /* READ WORDS FROM STDIN */
  /*************************/

  /* get first timestamp */
  if (gettimeofday(load_tv, NULL) == -1)
    perror("gettimeofday() failed: ");

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
  if (gettimeofday(load_tv + 1, NULL) == -1)
    perror("gettimeofday() failed: ");

  /* print out stats */
  fclose(fh);
  LOG("read %d words", num_words);

  for (i = 0; hashes[i].name; i++) {
    LOG("testing %s", hashes[i].name);

    /* get start time */
    if (gettimeofday(bias_tv + 0, NULL) == -1)
      perror("gettimeofday() failed: ");

    test_bias(words, num_words, hashes[i].fn, hashes[i].bias);

    /* get end time */
    if (gettimeofday(bias_tv + 1, NULL) == -1)
      perror("gettimeofday() failed: ");

    /* save time stats */
    hashes[i].time = bias_tv[1].tv_sec - bias_tv[0].tv_sec + 
                     (bias_tv[1].tv_usec - bias_tv[0].tv_usec) / 1000000.0;
  }


  /********************/
  /* PRINT BENCHMARKS */
  /********************/

  load_time = load_tv[1].tv_sec - load_tv[0].tv_sec + 
              (load_tv[1].tv_usec - load_tv[0].tv_usec) / 1000000.0;

  LOG(
    "Timing Results (%d words):\n"
    "load (from disk):    %4.3fsec (%2.5fusec/word)",

    num_words,
    load_time, load_time / num_words * 1000000 
  );

  /* print out timing results */
  for (i = 0; hashes[i].name; i++) {
    LOG(
      "%-16s:    %4.3fsec (%2.5fusec/word)", 

      hashes[i].name, hashes[i].time, 
      hashes[i].time / num_words * 1000000 
    );
  }

  LOG("\nBias Results:");
  for (i = 0; hashes[i].name; i++) {
    fprintf(stderr, "%s", hashes[i].name);

    for (j = 0; j < NUM_BITS; j++)
      fprintf(stderr, ",%4.2f", hashes[i].bias[j] * 100.0 / num_words);

    fprintf(stderr, "\n");
  }


  /* return success */
  return EXIT_SUCCESS;
}
