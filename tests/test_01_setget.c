#include <cuckoo/cuckoo.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define UNUSED(a) ((void) (a))

static struct { char *key, *val; } 
pairs[] = {
  { "asdf", "fsda" },
  { "foo",  "bar" },
  { "bar",  "baz" },
  { "blum", "flub" },
  { "99", "bottles" },
  { "of", "beer" },
  { "on", "the" },
  { "wall", "99" },
  { "bottles", "of" },
  { "beer", "you" },
  { "take", "one" },
  { "down", "and" },
  { "pass", "it" },
  { "around", "99" },
  { "bottles2", "of" },
  { "beer2", "on" },
  { "on2", "the" },
  { "wall2", "..." },
  { NULL,   NULL } 
};

void test_01_setget(int argc, char *argv[]) {
  ck_hash hash;
  ck_err err;
  char *key, *val, buf[1024];
  size_t i, len;

  UNUSED(argc);
  UNUSED(argv);

  /* init hash */
  ck_init(&hash, NULL);

  for (i = 0; pairs[i].key; i++) {
    /* get key, key length, and value */
    key = pairs[i].key;
    len = strlen(key) + 1;
    val = pairs[i].val;

    /* insert key/val pair and check for error */
    if ((err = ck_set(&hash, key, len, NULL, val)) != CK_OK) {
      ck_strerror(err, buf, sizeof(buf));
      fprintf(stderr, "error: %s\n", buf);
      exit(EXIT_FAILURE);
    }
  }

  /* get all results back */
  for (i = 0; pairs[i].key; i++) {
    /* get key, key length, and value */
    key = pairs[i].key;
    len = strlen(key) + 1;

    if ((err = ck_get(&hash, key, len, NULL, &val)) != CK_OK) {
      ck_strerror(err, buf, sizeof(buf));

      fprintf(stderr, "error getting %s: %s\n", key, buf);

      /* dump hash */
      fprintf(stderr, "\nhash dump:\n");
      ck_dump(&hash, stderr);
      fprintf(stderr, "\n");

      /* exit with failure */
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "got %s => %s ", key, val);
  }



  /* clean up hash */
  ck_fini(&hash);
}
