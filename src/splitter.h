#ifndef SPLITTER_H
#define SPLITTER_H

#include <stdlib.h>
typedef struct split_info {
  size_t num_used_buckets;
  unsigned int nominal_amt;
  unsigned int last_amt;
  size_t num_elements;
  unsigned int num_buckets;
} split_info;

split_info split_data(unsigned int num_elements, unsigned int num_buckets);
#ifdef SPLITTER_IMPLEMENTATION
#include <assert.h>
split_info split_data(unsigned int num_elements, unsigned int num_buckets) {
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
  return info;
}

// void split_work(void *list, size_t size_el, size_t num_el, function,...args)
#endif

#endif // !SPLITTER_H
