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

void test_00_basic(int argc, char *argv[]) {
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

    /* fprintf(stderr, "setting %s => %s\n", key, val); */

    /* insert key/val pair and check for error */
    if ((err = ck_set(&hash, key, len, NULL, val)) != CK_OK) {
      ck_strerror(err, buf, sizeof(buf));
      fprintf(stderr, "error: %s\n", buf);
      exit(EXIT_FAILURE);
    }
  }

  /* clean up hash */
  ck_fini(&hash);
}
