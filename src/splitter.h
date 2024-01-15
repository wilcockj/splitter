#ifndef SPLITTER_H
#define SPLITTER_H

#include <stdlib.h>
#include <sys/sysinfo.h>
#include <threads.h>
typedef struct split_info {
  size_t num_used_buckets;
  unsigned int nominal_amt;
  unsigned int last_amt;
  size_t num_elements;
  unsigned int num_buckets;
} split_info;

typedef struct split_args {
  void *list_start;
  size_t bucket_size;
  size_t return_size;
  void *args;
} split_args;

typedef struct split_return {
  void **ret_data_list;
  size_t num_returns;
} split_return;

typedef void *(*TaskFunction)(void *);

typedef struct {
  void *dataSegment;
  size_t segmentSize;
  void *extraArgs;
  void *result;
  TaskFunction func;
} ThreadArgs;

split_info split_data(unsigned int num_elements, unsigned int num_buckets);
#ifdef SPLITTER_IMPLEMENTATION
#include <assert.h>
split_info split_data(unsigned int num_elements, unsigned int num_buckets) {
  // find a way to use more efficient splitting
  // algo , where you spread the remainder over the buckets
  // rather than adding to last bucket
  //
  // for exmaple in this method with 3 buckets and 11 elements
  // you have a remainder of 2 and can spread as 4,4,3
  // 3 buckets 10 elements remainder 1 can spread as 4,3,3
  // increment for this first N%T threads
  split_info info = {0};
  info.num_buckets = num_buckets;
  info.num_elements = num_elements;
  // check if buckets is greater than elements
  assert(num_buckets != 0 && "trying to split into 0 buckets");
  assert(num_elements != 0 && "trying to split 0 elements");
  if (num_buckets > num_elements) {
    info.nominal_amt = 1;
    info.last_amt = 1;
    info.num_used_buckets = num_elements;
  }
  unsigned int remainder = num_elements % num_buckets;
  unsigned int divided = num_elements / num_buckets;
  info.nominal_amt = divided;
  info.last_amt = divided + remainder;
  info.num_used_buckets = num_buckets;
  return info;
}

// change to return a result type that says
// if there was error
/*
split_return split_work(void *list, size_t size_el, size_t num_el,
                        thrd_start_t func, split_args args) {

  int num_cores = get_nprocs();
  // allocate number of threads equal to core
  split_info splits = split_data(num_el, num_cores);
  thread_t *threads = malloc(splits.num_used_buckets * sizeof(thread_t));

  for (int i = 0; i < splits.num_used_buckets; i++) {
    if (thrd_create(&threads[i], func, &args) != thrd_success) {
      fprintf(stderr, "Error creating thread\n");
      return NULL;
    }
  }
}
*/

#endif

#endif // !SPLITTER_H
