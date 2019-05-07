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

#ifndef TDES_H_
#define TDES_H_

#include <string>

#include "key_generator.h"

#define BUFFER_SIZE 4096
#define NUM_BUFFERS 16

typedef struct callback_container {
  uint32_t num_callbacks;
  uint32_t num_expected_callbacks;
} callback_container;

/* Driving function. The crypto function accepts the parsed user inputs from *
 * main and applies the DES cryptography algorithm. The process loads bytes  *
 * into a buffer, encrypts or decrypts them, and writes them to a new file. */
void run(int mode, std::string *in_file_name, std::string *out_file_name);

void encrypt_task(uint8_t *block);
void decrypt_task(uint8_t *block);
void read_task(uint8_t *buffer, uint32_t num_bytes);
void write_task(uint8_t *buffer, uint32_t num_bytes);

void init_keys(KeyGenerator *keygen, uint8_t K1[16][6], uint8_t K2[16][6],
               uint8_t K3[16][6], int mode);

#endif  // TDES_H_
