#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

uint64_t create_mask(unsigned start_bit, unsigned end_bit) {
  uint64_t i, bit_mask = 0;
  for (i = start_bit; i <= end_bit; i++) {
    bit_mask |= 1LL << i;
  }

  return bit_mask;
}

#endif
