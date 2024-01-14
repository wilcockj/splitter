/*
 * Goals for splitter,
 * simple standalone header only lib
 * to nicely split data of N length
 * into B buckets
 * Will return in a struct that informs
 * whether all buckets were used, in the
 * case of more buckets than length
 * if all buckets used how many in each
 * bucket and remainder if there is one
 * could also just return nominal bucket
 * usage and the last bucket usage
 *
 *
 */
#define PULOG_IMPLEMENTATION
#include "pulog.h"
#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[]) {
  printf("uhh\n");
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-llDEBUG") == 0) {
      set_loglevel(DEBUG);
    } else if (strcmp(argv[i], "-llWARNING") == 0) {
      set_loglevel(WARNING);
    } else if (strcmp(argv[i], "-llERROR") == 0) {
      set_loglevel(ERROR);
    }
  }
  int test = 2;
  LOG(DEBUG, "Parsed args %d\n", test);
  return 0;
}
