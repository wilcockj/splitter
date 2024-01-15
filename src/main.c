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

#define SPLITTER_IMPLEMENTATION
#include "splitter.h"

#include <assert.h>
#include <string.h>
int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-llDEBUG") == 0) {
      set_loglevel(DEBUG);
    } else if (strcmp(argv[i], "-llWARNING") == 0) {
      set_loglevel(WARNING);
    } else if (strcmp(argv[i], "-llERROR") == 0) {
      set_loglevel(ERROR);
    }
  }
  unsigned int num_buckets = 4;
  unsigned int num_elements = 16;
  split_info info = split_data(num_elements, num_buckets);
  assert(info.nominal_amt == 4);
  assert(info.last_amt == 4);
  printf("num of els = %zu nominal_amt = %zu\n", info.num_elements,
         info.nominal_amt);
  num_elements = 18;
  info = split_data(num_elements, num_buckets);
  printf("num of els = %zu nominal_amt = %d last_amt = %d\n", info.num_elements,
         info.nominal_amt, info.last_amt);
  LOG(DEBUG, "test debug log\n");
  return 0;
}
