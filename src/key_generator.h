#ifndef KEY_GENERATOR_H_
#define KEY_GENERATOR_H_

#include <stdint.h>

#include "cipher.h"

class KeyGenerator {
 public:
  KeyGenerator();

  void generate(const uint8_t *key_with_parities, uint8_t round_keys[16][6]);

  void shift_left(uint64_t *key, const uint8_t num_shifts);

  void split_keys(const uint8_t *in_block, uint64_t *left_block,
                  uint64_t *right_block);

  void combine_keys(const uint64_t *left_block, const uint64_t *right_block,
                    uint8_t *out_block);

 private:
};

#endif  // KEY_GENERATOR_H_
