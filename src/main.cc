#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <bitset>
#include <iostream>

#include "cipher.h"
#include "key_generator.h"

using namespace std;

/* Bit rotation - Key */
const uint8_t bit_rotation[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

static void shift_left(uint64_t *key, const uint8_t num_shifts) {
  assert(num_shifts < 28);
  *key = (*key << num_shifts) | (*key >> (-num_shifts & 27));
}

static void split_keys(const uint8_t *in_block, uint64_t *left_block,
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

static void combine_keys(const uint64_t *left_block,
                         const uint64_t *right_block, uint8_t *out_block) {
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

static void bytes_to_bitset(const uint8_t *bytes, std::bitset<56> *b) {
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

int main() {
  KeyGenerator k;

  const char key[7] = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
  uint8_t sub_keys[16][6];
  memset(sub_keys, 0, 16 * 6);

  uint64_t left_key, right_key;

  bitset<56> start_bits;
  bytes_to_bitset((const uint8_t *)key, &start_bits);

  //------------split----------------
  left_key = start_bits.to_ullong();
  left_key >>= 28;

  right_key = start_bits.to_ullong() & 0xFFFFFFF;

  //------------------------

  int i;
  for (i = 0; i < 16; i++) {
    //-------------shift---------------

    shift_left(&left_key, bit_rotation[i]);
    shift_left(&right_key, bit_rotation[i]);

    bitset<28> left_bits(left_key);
    bitset<28> right_bits(right_key);
    //------------------------

    //------------combine-------------------
    uint8_t combined[7];
    memset(combined, 0, 7);

    bitset<56> combined_bits;

    uint64_t comb = left_bits.to_ullong() << 28 | right_bits.to_ullong();

    int x;
    for (x = 6; x >= 0; x--) {
      combined[x] |= comb >> ((6 - x) * 8);
    }
    bytes_to_bitset(combined, &combined_bits);
    //------------------------------------

    cout << combined_bits.to_string() << endl;
  }
  /*
  cout << start_bits.to_string() << endl;
  cout << left_bits.to_string() << endl;
  cout << "                            " << right_bits.to_string() << endl;
  cout << combined_bits.to_string() << endl;
*/
  return 0;
}
