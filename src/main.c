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

  LOG(DEBUG, "test debug log\n");

  // Parallelize test
  size_t arraySize = 100000;
  int *array = malloc(arraySize * sizeof(int));
  for (size_t i = 0; i < arraySize; i++) {
    array[i] = i + 1;
  }
  int *multiplier = malloc(sizeof(int));
  *multiplier = 4;
  thread_split_ret results =
      thread_split(array, arraySize, sizeof(int), multiplier, sumSegment);
  long long totalSum = 0;
  for (int i = 0; i < results.num_data; i++) {
    totalSum += *(long long *)results.data[i];
    free(results.data[i]); // Free the memory allocated for results
  }

  printf("Total sum: %lld\n", totalSum);
  free(array);
  free(results.data);
  free(multiplier);

  size_t size = 1000000;
  uint32_t *numarray = malloc(size * sizeof(uint32_t));
  // populate array
  for (size_t i = 0; i < size; i++) {
    numarray[i] = i + 2;
  }

  thread_split_ret prime_results =
      thread_split(numarray, size, sizeof(uint32_t), NULL, countPrimeSegment);
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
