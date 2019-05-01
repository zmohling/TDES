#include "key_generator.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <bitset>

#include "cipher.h"

/* Permuted Choice 1 - Key Schedule */
const uint8_t PC1[] = {57, 49, 41, 33, 25, 17, 9,  1,  58, 50, 42, 34, 26, 18,
                       10, 2,  59, 51, 43, 35, 27, 19, 11, 3,  60, 52, 44, 36,
                       63, 55, 47, 39, 31, 23, 15, 7,  62, 54, 46, 38, 30, 22,
                       14, 6,  61, 53, 45, 37, 29, 21, 13, 5,  28, 20, 12, 4};

/* Permuted Choice 2 - Key Schedule */
const uint8_t PC2[] = {14, 17, 11, 24, 1,  5,  3,  28, 15, 6,  21, 10,
                       23, 19, 12, 4,  26, 8,  16, 7,  27, 20, 13, 2,
                       41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
                       44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32};

/* Bit rotation - Key */
const uint8_t bit_rotation[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

KeyGenerator::KeyGenerator() {}

void KeyGenerator::generate(const char *key_with_parities,
                            uint8_t round_keys[16][6]) {
  uint8_t T1[7], T2[7];
  uint64_t left_key, right_key;

  permute(8, 7, (const uint8_t *)key_with_parities, T1, PC1);

  split_keys(T1, &left_key, &right_key);

  int i;
  for (i = 0; i < 16; i++) {
    shift_left(&left_key, bit_rotation[i]);

    shift_left(&right_key, bit_rotation[i]);

    combine_keys(&left_key, &right_key, T2);

    permute(7, 6, T2, round_keys[i], PC2);
  }
}

void KeyGenerator::shift_left(uint64_t *key, const uint8_t num_shifts) {
  assert(num_shifts < 28);
  *key = (*key << num_shifts) | (*key >> (-num_shifts & 27));
}

void KeyGenerator::split_keys(const uint8_t *in_block, uint64_t *left_block,
                              uint64_t *right_block) {
  uint8_t i, j, bit, key_size = 28, in_bytes = 7;

  for (i = 0; i < in_bytes; i++) {
    uint8_t cur = in_block[i];

    for (j = 0; j < CHAR_BIT; j++) {
      bit = ((in_bytes - 1) - i) * CHAR_BIT + j;

      if (bit >= key_size)
        *left_block |= (cur & 1LL) << (bit - key_size);
      else
        *right_block |= (cur & 1LL) << (bit);

      cur >>= 1;
    }
  }
}

void KeyGenerator::combine_keys(const uint64_t *left_block,
                                const uint64_t *right_block,
                                uint8_t *out_block) {
  int i, j;
  uint8_t bit, key_size = 28, out_bytes = 7;

  for (i = (out_bytes - 1); i >= 0; i--) {
    uint8_t *out_byte = &out_block[i];

    for (j = 0; j < CHAR_BIT; j++) {
      bit = ((out_bytes - 1) - i) * CHAR_BIT + j;

      if (bit < key_size) {
        *out_byte |= ((*right_block >> bit) & 1LL) << (bit % 8);
      } else {
        *out_byte |= ((*left_block >> (bit - 28)) & 1LL) << (bit % 8);
      }
    }
  }
}
