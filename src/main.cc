#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <bitset>
#include <iostream>

#include "cipher.h"
#include "key_generator.h"

using namespace std;

std::string sub_keys_str[16] = {
    "111000001011111001100110000100110010101010000010",
    "111000001011011001110110000100000010001100000111",
    "111001001101011001110110101101100000000010000100",
    "111001101101001101110010010000000010001111000011",
    "101011101101001101110011001101101010000000001001",
    "101011110101001101011011011000100001010101000010",
    "001011110101001111011001000011001010000100101010",
    "000111110101100111011001011001000101110001000000",
    "000111110100100111011001010010101001100001000000",
    "000111110110100110011101110000001100010100111000",
    "000111110010110110001101000010010001111000001000",
    "010110110010110010101101110110000101000000110000",
    "110110011010110010101100000000010100101000101100",
    "110100001010111010101110100100000011100010010000",
    "111100001011111000100110101000010000001000110101",
    "111100001011111000100110101000110100001010000000"};

static void bytes_to_bitset48(const uint8_t *bytes, std::bitset<48> *b) {
  for (int i = 0; i < 6; ++i) {
    uint8_t cur = bytes[5 - i];
    int offset = i * CHAR_BIT;

    for (int bit = 0; bit < CHAR_BIT; ++bit) {
      (*b)[offset] = cur & 1;
      ++offset;
      cur >>= 1;
    }
  }
}

static void bytes_to_bitset64(const uint8_t *bytes, std::bitset<64> *b) {
  for (int i = 0; i < 8; ++i) {
    uint8_t cur = bytes[7 - i];
    int offset = i * CHAR_BIT;

    for (int bit = 0; bit < CHAR_BIT; ++bit) {
      (*b)[offset] = cur & 1;
      ++offset;
      cur >>= 1;
    }
  }
}

static void test_keys(uint8_t sub_keys[16][6]) {
  int i;
  for (i = 0; i < 16; i++) {
    bitset<48> sub_key_bits;
    bytes_to_bitset48(sub_keys[i], &sub_key_bits);
    string sub_key = sub_key_bits.to_string();

    if (sub_key.compare(sub_keys_str[i]) != 0) {
      string error = " - Key generation error: " + sub_key + " should be " +
                     sub_keys_str[i] + ".";
      cout << i << error << endl;
      throw error;
    }
  }
}
int main() {
  Cipher c;

  KeyGenerator k;

  const char key[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  const char plaintext[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  uint8_t ciphertext[8];

  bitset<64> original_bits;
  bytes_to_bitset64((const uint8_t *)plaintext, &original_bits);
  cout << "Original: " << original_bits.to_string() << endl;

  uint8_t sub_keys[16][6];
  k.generate(key, sub_keys);

  c.encrypt(ciphertext, (const uint8_t *)plaintext, sub_keys);

  bitset<64> encrypted_bits;
  bytes_to_bitset64(ciphertext, &encrypted_bits);
  cout << "Encrypted: " << encrypted_bits.to_string() << endl;

  return 0;
}
