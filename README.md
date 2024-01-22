# Thread Splitter

Single header library that aims to make [Embarrassingly_parallel](https://en.wikipedia.org/wiki/Embarrassingly_parallel)
problems as easy as possible to parallelize.

Example of using thread splitter to count primes 
```c
#define THREAD_SPLITTER_IMPLEMENTATION
#include "thread_splitter.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int isPrime(int num) {
  for (int i = 2; i * i <= num; i++) {
    if (num % i == 0) {
      return 0;
    }
  }
  return 1;
}

void *countPrimeSegment(void *arg) {
  // get number of primes in this array segment
  ThreadArgs *threadArgs = (ThreadArgs *)arg;

  int *dataSegment = (int *)threadArgs->dataSegment;
  uint64_t *count = malloc(sizeof(uint64_t));
  *count = 0;
  for (size_t i = 0; i < threadArgs->segmentSize; i++) {
    *count += isPrime(dataSegment[i]);
  }
  return count;
}

int main(int argc, char *argv[]) {
  uint32_t prime_top = 1000000;
  size_t size = prime_top;
  uint32_t *numarray = malloc(size * sizeof(uint32_t));
  // populate array
  for (size_t i = 0; i < size; i++) {
    numarray[i] = i + 2;
  }

  thread_split_ret prime_results = thread_split(
      numarray, size, sizeof(uint32_t), NULL, countPrimeSegment, 0);
  uint64_t primeCount = 0;
  for (int i = 0; i < prime_results.num_data; i++) {
    primeCount += *(uint64_t *)prime_results.data[i];
    free(prime_results.data[i]);
  }
  printf("Count of primes from 2-%zu = %d\n", size + 2, primeCount);
  free(numarray);
  free(prime_results.data);

  return 0;
}
```
Output on my 16 core desktop
size of thread 0 segment is 62500
size of thread 1 segment is 62500
size of thread 2 segment is 62500
size of thread 3 segment is 62500
size of thread 4 segment is 62500
size of thread 5 segment is 62500
size of thread 6 segment is 62500
size of thread 7 segment is 62500
size of thread 8 segment is 62500
size of thread 9 segment is 62500
size of thread 10 segment is 62500
size of thread 11 segment is 62500
size of thread 12 segment is 62500
size of thread 13 segment is 62500
size of thread 14 segment is 62500
size of thread 15 segment is 62500
Count of primes from 2-1000002 = 78498


