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
#include <iostream>
#include <vector>

#include "cipher.h"
#include "io.h"
#include "key_generator.h"

static bool does_option_exist(char **begin, char **end,
                              const std::string &option) {
  return std::find(begin, end, option) != end;
}

/* Driving function. The crypto function accepts the parsed user inputs from *
 * main and applies the DES cryptography algorithm. The process loads bytes  *
 * into a buffer, encrypts or decrypts them, and writes them to a new file.  */
static int crypto(int mode, std::string *in_file_name,
                  std::string *out_file_name) {
  Cipher c;
  KeyGenerator k;

  /* Get password from the user, derive a 64-bit key from the input with     *
   * PBKDF2, and generate the 16 sub-keys. (One for each round).             */
  uint8_t key[8];
  uint8_t sub_keys[16][6];
  get_key(key, mode);
  k.generate(key, sub_keys);

  uint8_t *read_buffer;
  uint64_t length, cur_length = 0;
  uint64_t progress = 0;

  load_buffer_from_disk(*in_file_name, &read_buffer, &length);

  uint8_t *write_buffer = (uint8_t *)malloc((length + 1) * sizeof(uint8_t));

  while ((cur_length + 8) <= length) {
    if (mode == 0) {
      c.encrypt(&write_buffer[cur_length], &read_buffer[cur_length], sub_keys);
    } else {
      c.decrypt(&write_buffer[cur_length], &read_buffer[cur_length], sub_keys);
    }

    cur_length += 8;

    progress = (((float)cur_length) / ((float)length)) * 100;
    print_progress(progress, mode);
  }

  free(read_buffer);

  write_buffer_to_disk(*out_file_name, &write_buffer, &length);

  print_progress(100, mode);

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Incorrect usage: tdes [-enc|-dec] <source> <dest>\n");
    return -1;
  }

  int mode = 0;  // 0 for encrypt, 1 for decrypt
  std::string in_file_name(argv[2]), out_file_name(argv[3]);

  if (does_option_exist(argv, argv + argc, "-enc")) {
    mode = 0;
  } else if (does_option_exist(argv, argv + argc, "-dec")) {
    mode = 1;
  } else {
    fprintf(stderr, "Incorrect usage: tdes [-enc|-dec] <source> <dest>\n");
    return -2;
  }

  if (is_original_file(out_file_name.c_str())) {
    fprintf(stderr, "Aborting. This file already exists: %s\n",
            out_file_name.c_str());
    exit(-1);
  }

  crypto(mode, &in_file_name, &out_file_name);
}
