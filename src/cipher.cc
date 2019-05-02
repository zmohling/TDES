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

#include "cipher.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bitset>
#include <iostream>
#include <vector>

using namespace std;

/* Prototypes */
static void bytes_to_bitset48(const uint8_t *bytes, std::bitset<48> *b);

/* Initial purmutation */
const uint8_t IP[] = {58, 50, 42, 34, 26, 18, 10, 2,  60, 52, 44, 36, 28,
                      20, 12, 4,  62, 54, 46, 38, 30, 22, 14, 6,  64, 56,
                      48, 40, 32, 24, 16, 8,  57, 49, 41, 33, 25, 17, 9,
                      1,  59, 51, 43, 35, 27, 19, 11, 3,  61, 53, 45, 37,
                      29, 21, 13, 5,  63, 55, 47, 39, 31, 23, 15, 7};

/* Final purmutation */
const uint8_t FP[] = {40, 8,  48, 16, 56, 24, 64, 32, 39, 7,  47, 15, 55,
                      23, 63, 31, 38, 6,  46, 14, 54, 22, 62, 30, 37, 5,
                      45, 13, 53, 21, 61, 29, 36, 4,  44, 12, 52, 20, 60,
                      28, 35, 3,  43, 11, 51, 19, 59, 27, 34, 2,  42, 10,
                      50, 18, 58, 26, 33, 1,  41, 9,  49, 17, 57, 25};

/* Expansion function */
const uint8_t E[] = {32, 1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
                     8,  9,  10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
                     16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
                     24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1};

/* Permutation */
const uint8_t P[] = {16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23,
                     26, 5, 18, 31, 10, 2,  8,  24, 14, 32, 27,
                     3,  9, 19, 13, 30, 6,  22, 11, 4,  25};

/* Substitution boxes */
const uint8_t S1[] = {14, 4,  13, 1, 2,  15, 11, 8,  3,  10, 6,  12, 5,
                      9,  0,  7,  0, 15, 7,  4,  14, 2,  13, 1,  10, 6,
                      12, 11, 9,  5, 3,  8,  4,  1,  14, 8,  13, 6,  2,
                      11, 15, 12, 9, 7,  3,  10, 5,  0,  15, 12, 8,  2,
                      4,  9,  1,  7, 5,  11, 3,  14, 10, 0,  6,  13};

const uint8_t S2[] = {15, 1,  8,  14, 6,  11, 3, 4,  9,  7,  2,  13, 12,
                      0,  5,  10, 3,  13, 4,  7, 15, 2,  8,  14, 12, 0,
                      1,  10, 6,  9,  11, 5,  0, 14, 7,  11, 10, 4,  13,
                      1,  5,  8,  12, 6,  9,  3, 2,  15, 13, 8,  10, 1,
                      3,  15, 4,  2,  11, 6,  7, 12, 0,  5,  14, 9};

const uint8_t S3[] = {10, 0,  9,  14, 6,  3,  15, 5,  1,  13, 12, 7,  11,
                      4,  2,  8,  13, 7,  0,  9,  3,  4,  6,  10, 2,  8,
                      5,  14, 12, 11, 15, 1,  13, 6,  4,  9,  8,  15, 3,
                      0,  11, 1,  2,  12, 5,  10, 14, 7,  1,  10, 13, 0,
                      6,  9,  8,  7,  4,  15, 14, 3,  11, 5,  2,  12};

const uint8_t S4[] = {7,  13, 14, 3,  0,  6,  9,  10, 1,  2, 8,  5,  11,
                      12, 4,  15, 13, 8,  11, 5,  6,  15, 0, 3,  4,  7,
                      2,  12, 1,  10, 14, 9,  10, 6,  9,  0, 12, 11, 7,
                      13, 15, 1,  3,  14, 5,  2,  8,  4,  3, 15, 0,  6,
                      10, 1,  13, 8,  9,  4,  5,  11, 12, 7, 2,  14};

const uint8_t S5[] = {2,  12, 4, 1,  7,  10, 11, 6, 8,  5,  3,  15, 13,
                      0,  14, 9, 14, 11, 2,  12, 4, 7,  13, 1,  5,  0,
                      15, 10, 3, 9,  8,  6,  4,  2, 1,  11, 10, 13, 7,
                      8,  15, 9, 12, 5,  6,  3,  0, 14, 11, 8,  12, 7,
                      1,  14, 2, 13, 6,  15, 0,  9, 10, 4,  5,  3};

const uint8_t S6[] = {12, 1,  10, 15, 9,  2,  6,  8,  0,  13, 3, 4, 14,
                      7,  5,  11, 10, 15, 4,  2,  7,  12, 9,  5, 6, 1,
                      13, 14, 0,  11, 3,  8,  9,  14, 15, 5,  2, 8, 12,
                      3,  7,  0,  4,  10, 1,  13, 11, 6,  4,  3, 2, 12,
                      9,  5,  15, 10, 11, 14, 1,  7,  6,  0,  8, 13};

const uint8_t S7[] = {4,  11, 2,  14, 15, 0,  8, 13, 3,  12, 9,  7,  5,
                      10, 6,  1,  13, 0,  11, 7, 4,  9,  1,  10, 14, 3,
                      5,  12, 2,  15, 8,  6,  1, 4,  11, 13, 12, 3,  7,
                      14, 10, 15, 6,  8,  0,  5, 9,  2,  6,  11, 13, 8,
                      1,  4,  10, 7,  9,  5,  0, 15, 14, 2,  3,  12};

const uint8_t S8[] = {13, 2,  8, 4,  6,  15, 11, 1,  10, 9, 3, 14, 5,
                      0,  12, 7, 1,  15, 13, 8,  10, 3,  7, 4, 12, 5,
                      6,  11, 0, 14, 9,  2,  7,  11, 4,  1, 9, 12, 14,
                      2,  0,  6, 10, 13, 15, 3,  5,  8,  2, 1, 14, 7,
                      4,  10, 8, 13, 15, 12, 9,  0,  3,  5, 6, 11};

Cipher::Cipher() {}

void Cipher::encrypt(uint8_t *out, const uint8_t *in,
                     const uint8_t sub_keys[16][6]) {
  uint8_t T[8], left_block[4], right_block[4], T1[4], T3[8];

  permute(8, 8, in, T, IP);

  split(8, 4, T, left_block, right_block);

  int round;
  for (round = 0; round < 16; round++) {
    feistel_function(right_block, sub_keys[round], T1);

    exclusive_or(4, left_block, T1, left_block);

    if (round != 15) {
      swapper(4, left_block, right_block);
    }
  }

  combine(4, 8, left_block, right_block, T3);

  permute(8, 8, T3, out, FP);
}

void Cipher::decrypt(uint8_t *out, const uint8_t *in,
                     const uint8_t sub_keys[16][6]) {
  uint8_t T[8], left_block[4], right_block[4], T1[4], T3[8];

  permute(8, 8, in, T, IP);

  split(8, 4, T, left_block, right_block);

  int round;
  for (round = 15; round >= 0; round--) {
    feistel_function(right_block, sub_keys[round], T1);

    exclusive_or(4, left_block, T1, left_block);

    if (round != 0) {
      swapper(4, left_block, right_block);
    }
  }

  combine(4, 8, left_block, right_block, T3);

  permute(8, 8, T3, out, FP);
}

void Cipher::swapper(uint8_t bytes, uint8_t *left_block, uint8_t *right_block) {
  int i;
  for (i = 0; i < bytes; i++) {
    uint8_t temp = left_block[i];
    left_block[i] = right_block[i];
    right_block[i] = temp;
  }
}

void Cipher::feistel_function(const uint8_t *in_block, const uint8_t *round_key,
                              uint8_t *out_block) {
  uint8_t T1[6], T2[6], T3[4];

  permute(4, 6, in_block, T1, E);  // expansion

  exclusive_or(6, T1, round_key, T2);

  substitute(T2, T3);

  permute(4, 4, T3, out_block, P);
}

void Cipher::substitute(const uint8_t *in_block, uint8_t *out_block) {
  const uint8_t *substitution_boxes[] = {S1, S2, S3, S4, S5, S6, S7, S8};

  std::bitset<48> bits;
  bytes_to_bitset48(in_block, &bits);

  int i, j = 0, bit = 47;
  for (i = 0; i < 8; i++, bit -= 6) {
    uint8_t row = (bits[bit] * 2) + (bits[bit - 5] * 1);
    uint8_t column = (bits[bit - 1] * 8) + (bits[bit - 2] * 4) +
                     (bits[bit - 3] * 2) + (bits[bit - 4] * 1);

    if (i % 2 == 0) {
      out_block[j] = 0;
      out_block[j] |= (((substitution_boxes[i][(row * 16) + column])) << 4);
    } else {
      out_block[j] |= ((substitution_boxes[i][(row * 16) + column]));
      j++;
    }
  }
}

void permute(const uint8_t in_bytes, const uint8_t out_bytes,
             const uint8_t *in_block, uint8_t *out_block,
             const uint8_t *permute_table) {
  uint8_t byte, bit;
  for (byte = 0; byte < out_bytes; ++byte) {
    uint8_t x, t = 0;
    for (bit = 0; bit < 8; ++bit) {
      x = *permute_table - 1;
      permute_table++;
      t <<= 1;
      if ((in_block[x / 8]) & (0x80 >> (x % 8))) {
        t |= 0x01;
      }
    }
    out_block[byte] = t;
  }
}

void split(const uint8_t in_bytes, const uint8_t out_bytes,
           const uint8_t *in_block, uint8_t *left_block, uint8_t *right_block) {
  if ((in_bytes / out_bytes != 2) || (in_bytes % 2 != 0) ||
      (out_bytes % 2 != 0)) {
    throw "Invalid split arguments";
  }

  uint8_t byte;
  for (byte = 0; byte < in_bytes; byte++) {
    if (byte < (in_bytes / 2)) {
      left_block[byte] = in_block[byte];
    } else {
      right_block[byte - (in_bytes / 2)] = in_block[byte];
    }
  }
}

void combine(const uint8_t in_bytes, const uint8_t out_bytes,
             const uint8_t *left_block, const uint8_t *right_block,
             uint8_t *out_block) {
  if ((in_bytes * 2) != out_bytes || (in_bytes % 2 != 0) ||
      (out_bytes % 2 != 0)) {
    throw "Invalid combine arguments";
  }

  uint8_t byte;
  for (byte = 0; byte < out_bytes; byte++) {
    if (byte < (out_bytes / 2)) {
      out_block[byte] = left_block[byte];
    } else {
      out_block[byte] = right_block[byte - (out_bytes / 2)];
    }
  }
}

void exclusive_or(const uint8_t bytes, const uint8_t *first_block,
                  const uint8_t *second_block, uint8_t *out_block) {
  uint8_t byte;
  for (byte = 0; byte < bytes; byte++) {
    out_block[byte] = first_block[byte] ^ second_block[byte];
  }
}

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
