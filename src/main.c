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

// TODO: add lots of regression tests to make sure we are getting good results
// and keep same api

// TODO: try to improve how the data is casted to avoid wrong data type bugs

#include <stdint.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <threads.h>
#define PULOG_IMPLEMENTATION
#include "pulog.h"

#define THREAD_SPLITTER_IMPLEMENTATION
#include "thread_splitter.h"

#include <assert.h>
#include <string.h>

void *sumSegment(void *arg) {
  ThreadArgs *threadArgs = (ThreadArgs *)arg;
  int multiplier = *(int *)threadArgs->extraArgs;
  // printf("got arg in thread mul %d\n", multiplier);
  //  Process the segment of data
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
  return 1;
}

void *countPrimeSegment(void *arg) {
  ThreadArgs *threadArgs = (ThreadArgs *)arg;

  int *dataSegment = (int *)threadArgs->dataSegment;
  uint64_t *count = malloc(sizeof(uint64_t));
  *count = 0;
  for (size_t i = 0; i < threadArgs->segmentSize; i++) {
    *count += isPrime(dataSegment[i]);
  }
  LOG(DEBUG, "Thread %d is finished with counting primes\n",
      threadArgs->thread_num);
  return count;
}

int main(int argc, char *argv[]) {
  uint32_t prime_top = 0;
  uint16_t num_threads = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-llDEBUG") == 0) {
      set_loglevel(DEBUG);
    } else if (strcmp(argv[i], "-llWARNING") == 0) {
      set_loglevel(WARNING);
    } else if (strcmp(argv[i], "-llERROR") == 0) {
      set_loglevel(ERROR);
    } else if (strcmp(argv[i], "--prime_top") == 0) {
      if (argc >= i + 1) {
        prime_top = strtol(argv[i + 1], NULL, 10);
        LOG(DEBUG, "Setting prime top to use to %d\n", prime_top);
      }
    } else if (strcmp(argv[i], "--threads") == 0) {
      if (argc >= i + 1) {
        num_threads = strtol(argv[i + 1], NULL, 10);
        LOG(DEBUG, "Setting number of threads to use to %d\n", num_threads);
      }
    }
  }

  LOG(DEBUG, "test debug log\n");

  // Parallelize test
  size_t arraySize = 100000;
  int *array = malloc(arraySize * sizeof(int));
  for (size_t i = 0; i < arraySize; i++) {
    array[i] = i + 1;
  }
  int *multiplier = malloc(sizeof(int));
  *multiplier = 4;
  thread_split_ret results = thread_split(array, arraySize, sizeof(int),
                                          multiplier, sumSegment, num_threads);
  long long totalSum = 0;
  for (int i = 0; i < results.num_data; i++) {
    totalSum += *(long long *)results.data[i];
    free(results.data[i]); // Free the memory allocated for results
  }

  printf("Total sum: %lld\n", totalSum);
  free(array);
  free(results.data);
  free(multiplier);

  if (prime_top == 0) {
    printf("did not supply number for prime counting\n");
    return 1;
  }
  size_t size = prime_top;
  uint32_t *numarray = malloc(size * sizeof(uint32_t));
  // populate array
  for (size_t i = 0; i < size; i++) {
    numarray[i] = i + 2;
  }

  thread_split_ret prime_results = thread_split(
      numarray, size, sizeof(uint32_t), NULL, countPrimeSegment, num_threads);
  uint64_t primeCount = 0;
  for (int i = 0; i < results.num_data; i++) {
    primeCount += *(uint64_t *)prime_results.data[i];
    free(prime_results.data[i]);
  }
  printf("Count of primes from 2-%zu = %d\n", size + 2, primeCount);
  free(numarray);
  free(prime_results.data);

  return 0;
}
