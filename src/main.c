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
#include <stdint.h>
#include <sys/sysinfo.h>
#include <threads.h>
#define PULOG_IMPLEMENTATION
#include "pulog.h"

#define SPLITTER_IMPLEMENTATION
#include "splitter.h"

#include <assert.h>
#include <string.h>
typedef struct {
  int *array;
  int start;
  int end;
  long long sum;
} TArgs;

// example for using api
int calculateSum(void *arg) {
  TArgs *args = (TArgs *)arg;
  args->sum = 0;
  for (int i = args->start; i < args->end; i++) {
    args->sum += args->array[i];
  }
  return 0;
}

int count_primes(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  return 0;
}

int threadFunc(void *arg) {
  ThreadArgs *threadArgs = (ThreadArgs *)arg;
  threadArgs->result = threadArgs->func(threadArgs);
  return 0;
}

typedef struct par_return {
  void **data;
  size_t num_data;
} par_return;

par_return parallelize(void *data, size_t dataSize, size_t elementSize,
                       void *extraArgs, TaskFunction func) {
  int num_cores = get_nprocs();
  thrd_t threads[num_cores];
  ThreadArgs threadArgs[num_cores];
  void **results = malloc(num_cores * sizeof(void *));

  size_t segmentSize = dataSize / num_cores;
  assert(dataSize > num_cores);
  par_return return_struct = {0};
  for (int i = 0; i < num_cores; i++) {
    size_t offset = i * segmentSize;
    threadArgs[i].dataSegment = (char *)data + offset * elementSize;
    threadArgs[i].segmentSize =
        (i == num_cores - 1) ? dataSize - offset : segmentSize;
    threadArgs[i].extraArgs = extraArgs;
    threadArgs[i].func = func;
    if (thrd_create(&threads[i], threadFunc, &threadArgs[i]) != thrd_success) {
      fprintf(stderr, "Error creating thread %d\n", i);
      free(results);
      return return_struct;
    }
  }

  for (int i = 0; i < num_cores; i++) {
    thrd_join(threads[i], NULL);
    results[i] = threadArgs[i].result;
  }
  return_struct.data = results;
  return_struct.num_data = num_cores;

  return return_struct;
}

void *sumSegment(void *arg) {
  ThreadArgs *threadArgs = (ThreadArgs *)arg;
  int multiplier = *(int *)threadArgs->extraArgs;
  printf("got arg in thread mul %d\n", multiplier);
  // Process the segment of data
  int *dataSegment = (int *)threadArgs->dataSegment;
  long long *sum = malloc(sizeof(long long));
  *sum = 0;

  for (size_t i = 0; i < threadArgs->segmentSize; i++) {
    *sum += dataSegment[i]; //* multiplier;
  }
  return sum;
}

int isPrime(int num) {
  for (int i = 2; i * i <= num; i++) {
    if (num % i == 0) {
      return 0;
    }
  }
  printf("%d is prime\n", num);
  return 1;
}

void *countPrimeSegment(void *arg) {
  ThreadArgs *threadArgs = (ThreadArgs *)arg;

  int *dataSegment = (int *)threadArgs->dataSegment;
  long long *count = malloc(sizeof(long long));
  *count = 0;
  for (size_t i = 0; i < threadArgs->segmentSize; i++) {
    *count += isPrime(dataSegment[i]);
  }
  return count;
}

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

  // Parallelize test
  size_t arraySize = 100000;
  int *array = malloc(arraySize * sizeof(int));
  for (size_t i = 0; i < arraySize; i++) {
    array[i] = i + 1;
  }
  int *multiplier = malloc(sizeof(int));
  *multiplier = 4;
  par_return results =
      parallelize(array, arraySize, sizeof(int), multiplier, sumSegment);
  long long totalSum = 0;
  for (int i = 0; i < results.num_data; i++) {
    totalSum += *(long long *)results.data[i];
    free(results.data[i]); // Free the memory allocated for results
  }

  printf("Total sum: %lld\n", totalSum);
  free(array);
  free(results.data);

  size_t size = 1000000;
  uint32_t *numarray = malloc(size * sizeof(uint32_t));
  for (size_t i = 0; i < size; i++) {
    numarray[i] = i + 2;
  }
  par_return prime_results =
      parallelize(numarray, size, sizeof(uint32_t), NULL, countPrimeSegment);
  long long primeCount = 0;
  for (int i = 0; i < results.num_data; i++) {
    primeCount += *(long long *)prime_results.data[i];
    free(prime_results.data[i]);
  }
  printf("Count of primes from 2-%zu = %lld\n", size + 2, primeCount);
  free(numarray);
  free(prime_results.data);

  return 0;
}
