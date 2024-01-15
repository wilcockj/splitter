#ifndef THREAD_SPLITTER_H
#define THREAD_SPLITTER_H

#include <stdlib.h>
#include <sys/sysinfo.h>
#include <threads.h>

typedef void *(*ThreadSplitFunction)(void *);

typedef struct {
  void *dataSegment;
  size_t segmentSize;
  void *extraArgs;
  void *result;
  ThreadSplitFunction func;
} ThreadArgs;

typedef struct thread_split_ret {
  void **data;
  size_t num_data;
} thread_split_ret;

thread_split_ret thread_split(void *data, size_t dataSize, size_t elementSize,
                              void *Args, ThreadSplitFunction func, uint16_t num_threads);
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
                              void *Args, ThreadSplitFunction func, uint16_t num_threads) {
  int num_cores = get_nprocs();
  if(num_threads!= 0){
    num_cores = num_threads;
  }
  thrd_t threads[num_cores];
  ThreadArgs threadArgs[num_cores];
  void **results = malloc(num_cores * sizeof(void *));

  size_t segmentSize = dataSize / num_cores;
  // get remainder so it can be spread among
  // the threads
  int segmentRemainder = dataSize % num_cores;

  assert(dataSize > num_cores &&
         "your data is shorter than the number of cores");
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
#endif

#endif // !THREAD_SPLITTER_H
