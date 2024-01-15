#ifndef THREAD_SPLITTER_H
#define THREAD_SPLITTER_H

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
typedef void *(*ThreadSplitFunction)(void *);

typedef struct {
  void *dataSegment;
  size_t segmentSize;
  void *extraArgs;
  void *result;
  TaskFunction func;
} ThreadArgs;

typedef struct thread_split_ret {
  void **data;
  size_t num_data;
} thread_split_ret;

split_info split_data(unsigned int num_elements, unsigned int num_buckets);
#ifdef THREAD_SPLITTER_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int threadFunc(void *arg) {
  ThreadArgs *threadArgs = (ThreadArgs *)arg;
  threadArgs->result = threadArgs->func(threadArgs);
  return 0;
}
thread_split_ret thread_split(void *data, size_t dataSize, size_t elementSize,
                              void *Args, ThreadSplitFunction func) {
  int num_cores = get_nprocs();
  thrd_t threads[num_cores];
  ThreadArgs threadArgs[num_cores];
  void **results = malloc(num_cores * sizeof(void *));

  size_t segmentSize = dataSize / num_cores;
  // get remainder so it can be spread among
  // the threads
  int segmentRemainder = dataSize % num_cores;

  assert(dataSize > num_cores);
  thread_split_ret return_struct = {0};
  size_t offset = 0;
  for (int i = 0; i < num_cores; i++) {
    threadArgs[i].dataSegment = (char *)data + offset * elementSize;

    // spread the work better, so average segment size is
    // less
    if (segmentRemainder-- > 0) {
      threadArgs[i].segmentSize =
          (i == num_cores - 1) ? dataSize - offset : segmentSize + 1;
    } else {
      threadArgs[i].segmentSize =
          (i == num_cores - 1) ? dataSize - offset : segmentSize;
    }
    threadArgs[i].extraArgs = Args;
    threadArgs[i].func = func;
    printf("size of thread %d segment is %d\n", i, threadArgs[i].segmentSize);
    if (thrd_create(&threads[i], threadFunc, &threadArgs[i]) != thrd_success) {
      fprintf(stderr, "Error creating thread %d\n", i);
      free(results);
      return return_struct;
    }
    offset += threadArgs[i].segmentSize;
  }

  for (int i = 0; i < num_cores; i++) {
    thrd_join(threads[i], NULL);
    results[i] = threadArgs[i].result;
  }
  return_struct.data = results;
  return_struct.num_data = num_cores;

  return return_struct;
}

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

#endif // !THREAD_SPLITTER_H
