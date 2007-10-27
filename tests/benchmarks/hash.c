
/* for strndup() */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include <cuckoo/cuckoo.h>  /* for my hash tbl */

#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)

#define NUM_BITS (sizeof(uint32_t) * 8)

/* for hash distribution test */
#define NUM_BINS 8
#define BIN_SIZE(i) ((size_t) (1 << (4 + 2 * (i))))

extern char *strndup(const char *, size_t);

typedef enum {
  STATE_IN_SPACE,
  STATE_IN_WORD
} State;

typedef uint32_t (*HashFn)(char *, size_t);

typedef struct { 
  /* mean and std dev. of the dist of this hash bin size */
  double ave, dev;

  /* min/max of the dist of this hash bin size */
  size_t min, max;
} DistStats;

#define ZEROS {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define HASH(a) { #a, ck_hash_##a, ZEROS, 0.0, NULL }
static struct {
  char   *name;
  HashFn  fn;
  int     bias[NUM_BITS];
  double  time;

  DistStats *stats;
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

  { NULL, NULL, ZEROS, 0.0, NULL }
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
test_bias(HashFn fn, Word *words, size_t num_words, int *bias) {
  size_t i, j;
  uint32_t hash;

  for (i = 0; i < num_words; i++) {
    hash = (*fn)(words[i].ptr, words[i].len);

    for (j = 0; j < NUM_BITS; j++)
      bias[j] += (hash & (1 << j)) ? 1 : -1;
  }
}

static void
test_dist(HashFn fn, DistStats *stats, Word *words, size_t num_words, size_t **bins) {
  size_t i, j;
  uint32_t hash;

  /* clear bins */
  for (i = 0; i < NUM_BINS; i++) 
    memset(bins[i], 0, sizeof(size_t) * BIN_SIZE(i));

  /* clear dist stats */
  memset(stats, 0, sizeof(DistStats) * NUM_BINS);

  /* hash words across all bins */
  for (i = 0; i < num_words; i++) {
    hash = (*fn)(words[i].ptr, words[i].len);

    for (j = 0; j < NUM_BINS; j++)
      bins[j][hash % BIN_SIZE(j)]++;
  }

  /* calculate the average, min, and max bin size */
  for (i = 0; i < NUM_BINS; i++) {
    /* reset the min and max for this bin */
    stats[i].max = 0;
    stats[i].min = BIN_SIZE(i + 1);

    /* calculate the average, min, and max */
    for (j = 0; j < BIN_SIZE(i); j++) {
      if (bins[i][j] > stats[i].max)
        stats[i].max = bins[i][j];
      if (bins[i][j] < stats[i].min)
        stats[i].min = bins[i][j];

      stats[i].ave += bins[i][j];
    }
    stats[i].ave /= BIN_SIZE(i);

    /* calculate the standard deviation */
    for (j = 0; j < BIN_SIZE(i); j++)
      stats[i].dev += pow(stats[i].ave - bins[i][j], 2);
    stats[i].dev = sqrt(stats[i].dev / BIN_SIZE(i));
  }
}

static void
dump_results(size_t num_words) {
  size_t i, j;
  double bias, bias_min, bias_max, bias_ave, bias_dev,
         dist_min, dist_max, dist_dev;

  for (i = 0; hashes[i].name; i++) {
    bias_ave = bias_dev = dist_dev = 0.0;
    bias_min = dist_min = num_words;
    bias_max = dist_max = 0 - num_words;

    /* calculate min/max/ave bit bias */
    for (j = 0; j < NUM_BITS; j++) {
      bias = hashes[i].bias[j];

      if (bias < bias_min)
        bias_min = bias;
      if (bias > bias_max)
        bias_max = bias;

      bias_ave += bias;
    }
    bias_ave /= NUM_BITS;

    /* calculate bit bias std deviation */
    for (j = 0; j < NUM_BITS; j++) {
      bias = hashes[i].bias[j] / num_words;
      bias_dev += pow(bias_ave - bias, 2);
    }
    bias_dev = sqrt(bias_dev / NUM_BITS);

    for (j = 0; j < NUM_BINS; j++) {
      if (hashes[i].stats[j].min * 1.0 / BIN_SIZE(j) < dist_min)
        dist_min = hashes[i].stats[j].min * 1.0 / BIN_SIZE(j);
      if (hashes[i].stats[j].max * 1.0 / BIN_SIZE(j) > dist_max)
        dist_max = hashes[i].stats[j].max * 1.0 / BIN_SIZE(j);
      dist_dev += hashes[i].stats[j].dev / BIN_SIZE(j);
    }
    dist_dev /= NUM_BINS * num_words;

    /* XXX: is this correct? */
    dist_min /= num_words;
    dist_max /= num_words;

    printf(
      /* name,time,time_per_word, */
      "%s,%4.3f,%2.5f,"

      /* bias_ave,bias_dev,bias_min,bias_max, */
      "%2.5f,%2.5f,%2.5f,%2.5f,"
    
      /* dist_mean_dev,dist_min_pct,dist_max_pct */
      "%2.5f,%2.5f,%2.5f\n",

      /* name,time,time_per_word, */
      hashes[i].name, hashes[i].time, 
      hashes[i].time / num_words * 1000000,

      /* bias_ave,bias_dev,bias_min,bias_max, */
      bias_ave, bias_dev, bias_min, bias_max,

      /* dist_mean_dev_pct,dist_min_pct,dist_max_pct */
      dist_dev * 100.0, dist_min * 100.0, dist_max * 100.0

      /* XXX add per-bin dists? */
    );
  }
}

int main(int argc, char *argv[]) {
  Word *words;
  State state = 0;
  FILE *fh;
  struct timeval load_tv[2], bias_tv[2];
  char  buf[4096], word_buf[1024];
  size_t  i, o, len, word_len, num_words, max_words, **bins;
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

  /* allocate test bins */
  if ((bins = malloc(sizeof(size_t*) * NUM_BINS)) == NULL) {
    LOG("couldn't allocate test bins");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < NUM_BINS; i++) {
    bins[i] = malloc(BIN_SIZE(i) * sizeof(size_t));
    if (bins[i] == NULL) {
      LOG("couldn't allocate test bin of %d elements", BIN_SIZE(i));
      exit(EXIT_FAILURE);
    }
  }

  /* allocate distribution stats for this hash */
  for (i = 0; hashes[i].name; i++) {
    if ((hashes[i].stats = malloc(sizeof(DistStats) * NUM_BINS)) == NULL) {
      LOG("couldn't allocate stats for hash %s", hashes[i].name);
      exit(EXIT_FAILURE);
    }
  }

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
    LOG("testing %s (benchmark)", hashes[i].name);

    /* get start time */
    if (gettimeofday(bias_tv + 0, NULL) == -1)
      perror("gettimeofday() failed: ");

    /* test hash bit bias */
    test_bias(hashes[i].fn, words, num_words, hashes[i].bias);

    /* get end time */
    if (gettimeofday(bias_tv + 1, NULL) == -1)
      perror("gettimeofday() failed: ");

    /* save time stats */
    hashes[i].time = bias_tv[1].tv_sec - bias_tv[0].tv_sec + 
                     (bias_tv[1].tv_usec - bias_tv[0].tv_usec) / 1000000.0;

    /* test distribution */
    LOG("testing %s (distribution)", hashes[i].name);
    test_dist(hashes[i].fn, hashes[i].stats, words, num_words, bins);
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

  printf(
    "name,time,time_per_word,"
    "bias_ave,bias_dev,bias_min,bias_max,"
    "dist_mean_dev_pct,dist_min_pct,dist_min,dist_max_pct"
    "\n"
  );
  dump_results(num_words);

  /* return success */
  return EXIT_SUCCESS;
}

