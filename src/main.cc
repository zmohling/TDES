/*
 *  Copyright (C) 2019 Zachary Mohling
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <bitset>
#include <ctime>
#include <iostream>
#include <vector>

#include "tdes.h"

static bool does_option_exist(char **begin, char **end,
                              const std::string &option) {
  return std::find(begin, end, option) != end;
}

int main(int argc, char *argv[]) {
  // encryption_test();
  // return 0;

  if (argc != 4) {
    fprintf(stderr, "Incorrect usage: tdes [-enc|-dec] <source> <dest>\n");
    return -1;
  }

  int mode = 0;  // 0 for encrypt, 1 for decrypt
  std::string in_file_name(argv[2]), out_file_name(argv[3]);

  if (does_option_exist(argv, argv + argc, "-enc") ||
      does_option_exist(argv, argv + argc, "--encrypt")) {
    mode = 0;
  } else if (does_option_exist(argv, argv + argc, "-dec") ||
             does_option_exist(argv, argv + argc, "--decrypt")) {
    mode = 1;
  } else {
    fprintf(stderr, "Incorrect usage: tdes [-enc|-dec] <source> <dest>\n");
    return -2;
  }

  /* Check if output file is original file */
  if (strcmp(in_file_name.c_str(), out_file_name.c_str()) == 0) {
    fprintf(stderr, "Aborting. Refusing to overwrite original file: %s\n",
            in_file_name.c_str());
    exit(-1);
  }

  run(mode, &in_file_name, &out_file_name);
}

/*
 *
// Validation code
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

std::string encrypted_val =
    "0010101010001101011010011101111010011101010111111101111111111001";

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

static void encryption_test() {
  Cipher c;
  KeyGenerator k;

  const uint8_t key[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  // uint8_t plaintext[16] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
  //                         'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
  const char plaintext[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  uint8_t ciphertext[8];

  uint8_t sub_keys[16][6];
  k.generate(key, sub_keys);

  std::bitset<64> original_bits;
  bytes_to_bitset64((const uint8_t *)plaintext, &original_bits);
  // std::cout << "Original : " << original_bits.to_string() << std::endl;

  //-----------encryption-------------------
  c.encrypt(ciphertext, (const uint8_t *)plaintext, sub_keys);
  std::bitset<64> encrypted_bits;
  bytes_to_bitset64(ciphertext, &encrypted_bits);

  // std::cout << "Encrypted: " << encrypted_bits.to_string() << std::endl;

  if (encrypted_bits.to_string().compare(encrypted_val) != 0) {
    std::string error = "Encryption error: " + encrypted_bits.to_string() +
                        "should be " + encrypted_val + ".";
    std::cout << error << std::endl;
    throw error;
  }

  //------------decryption--------------

  uint8_t plain[8];
  c.decrypt(plain, (const uint8_t *)ciphertext, sub_keys);
  std::bitset<64> decrypted_bits;
  bytes_to_bitset64(plain, &decrypted_bits);

  // std::cout << "Decrypted: " << decrypted_bits.to_string() << std::endl;

  if (original_bits != decrypted_bits) {
    throw "Bits don't match";
  }

  int i;
  for (i = 0; i < 16; i++) {
    std::bitset<48> sub_key_bits;
    bytes_to_bitset48(sub_keys[i], &sub_key_bits);
    std::string sub_key = sub_key_bits.to_string();

    if (sub_key.compare(sub_keys_str[i]) != 0) {
      std::string error = " - Key generation error: " + sub_key +
                          " should be " + sub_keys_str[i] + ".";
      std::cout << i << error << std::endl;
      throw error;
    }
  }
}
*/
