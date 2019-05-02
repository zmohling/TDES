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

#ifndef CIPHER_H_
#define CIPHER_H_

#include <stdint.h>

class Cipher {
 public:
  Cipher();

  void encrypt(uint8_t *out, const uint8_t *in, const uint8_t sub_keys[16][6]);

  void decrypt(uint8_t *out, const uint8_t *in, const uint8_t sub_keys[16][6]);

 private:
  void swapper(uint8_t bytes, uint8_t *left_block, uint8_t *right_block);

  void feistel_function(const uint8_t *in_block, const uint8_t *round_key,
                        uint8_t *out_block);

  void substitute(const uint8_t *in_block, uint8_t *out_block);
};

void permute(const uint8_t in_bytes, const uint8_t out_bytes,
             const uint8_t *in_block, uint8_t *out_block,
             const uint8_t *permute_table);

void split(const uint8_t in_bytes, const uint8_t out_bytes,
           const uint8_t *in_block, uint8_t *left_block, uint8_t *right_block);

void combine(const uint8_t in_bytes, const uint8_t out_bytes,
             const uint8_t *left_block, const uint8_t *right_block,
             uint8_t *out_block);

void exclusive_or(const uint8_t bytes, const uint8_t *first_block,
                  const uint8_t *second_block, uint8_t *out_block);

#endif  // CIPHER_H_
