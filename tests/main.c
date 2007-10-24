#include <cuckoo/cuckoo.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "list.h"

#define UNUSED(a) ((void) (a))

int main(int argc, char *argv[]) {
#include "list.c"
  return EXIT_SUCCESS;
}
