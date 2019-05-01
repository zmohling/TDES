#include "key_generator.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <bitset>
#include <iostream>

#include "cipher.h"

using namespace std;

/* Prototypes */
static void bytes_to_bitset56(const uint8_t *bytes, std::bitset<56> *b);

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
  uint64_t left_key = 0, right_key = 0;

  /* init */
  memset(round_keys, 0, 16 * 6);
  memset(T1, 0, 7);

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

void KeyGenerator::split_keys(const uint8_t *key, uint64_t *left_key,
                              uint64_t *right_key) {
  bitset<56> parity_dropped_key;
  bytes_to_bitset56((const uint8_t *)key, &parity_dropped_key);

  *left_key = (parity_dropped_key.to_ullong() >> 28) & 0xFFFFFFF;

  *right_key = parity_dropped_key.to_ullong() & 0xFFFFFFF;
}

void KeyGenerator::combine_keys(const uint64_t *left_key,
                                const uint64_t *right_key,
                                uint8_t *combined_key) {
  memset(combined_key, 0, 7);

  uint64_t combined_int64 = *left_key << 28 | (*right_key & 0xFFFFFFF);

  int byte;
  for (byte = 6; byte >= 0; byte--)
    combined_key[byte] |= combined_int64 >> ((6 - byte) * 8);
}

static void bytes_to_bitset56(const uint8_t *bytes, std::bitset<56> *b) {
  for (int i = 0; i < 7; ++i) {
    uint8_t cur = bytes[6 - i];
    int offset = i * CHAR_BIT;

    for (int bit = 0; bit < CHAR_BIT; ++bit) {
      (*b)[offset] = cur & 1;
      ++offset;
      cur >>= 1;
    }
  }
}
