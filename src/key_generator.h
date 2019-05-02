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
