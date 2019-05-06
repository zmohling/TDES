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

#ifndef IO_H_
#define IO_H_

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#define BUFFER_SIZE 4096
#define NUM_BUFFERS 16

int is_original_file(const char *filename) {
  struct stat st;
  int result = stat(filename, &st);
  return result == 0;
}

void open_file(FILE **file, std::string path, const char *mode,
               uint64_t *total_length) {
  if (!(*file = fopen(path.c_str(), mode))) {
    fprintf(stderr, "Could not open file %s. ERROR: %d\n", path.c_str(), errno);
    exit(-1);
  }

  if (strcmp(mode, "rb") == 0) {
    fseek(*file, 0, SEEK_END);
    *total_length = ftell(*file);
    rewind(*file);
  }
}

void read_into_buffer_block(FILE **file, uint8_t **buffer, uint8_t num_bytes,
                            uint64_t *current_length, uint64_t *total_length) {
  uint32_t size = 0;

  if (*current_length + num_bytes <= *total_length) {
    size = BUFFER_SIZE;
  } else {
    size = *total_length - *current_length;
  }

  if (!fread(*buffer, size, 1, *file)) {
    fprintf(stderr, "Error: could not load block starting at %lu. ERROR: %d\n",
            *current_length, errno);
    exit(-7);
  }

  // fclose(in_file_ptr);
}

void write_buffer_to_disk(FILE **file, uint8_t **buffer, uint8_t num_bytes,
                          uint64_t *current_length) {
  static int init = 0;

  if (init == 0) {
    if (!(*buffer = (uint8_t *)malloc((BUFFER_SIZE) * sizeof(uint8_t)))) {
      fprintf(stderr, "Insufficient memory. ERROR: %d\n", errno);
      exit(-1);
    }

    init = 1;
  }

  if (!fwrite(*buffer, num_bytes, 1, *file)) {
    fprintf(stderr, "Error: could not write block starting at %lu. ERROR: %d\n",
            *current_length, errno);
    exit(-8);
  }

  // fclose(in_file_ptr);
}
/*
void write_buffer_to_disk(std::string path, uint8_t **buffer,
                          uint64_t *length) {
  FILE *out_file_ptr;
  if (!(out_file_ptr = fopen(path.c_str(), "wb"))) {
    fprintf(stderr, "Could not open file %s. ERROR: %d\n", path.c_str(), errno);
    exit(-1);
  }

  fwrite(*buffer, *length, 1, out_file_ptr);

  free(*buffer);

  fclose(out_file_ptr);
}
*/
static void toggle_visible_input() {
  static struct termios oldt, newt;
  static bool stalled = false;

  if (stalled) {
    /*resetting our old STDIN_FILENO*/
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    stalled = !stalled;
  } else {
    /* saving the old settings */
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    stalled = !stalled;
  }
}

void prompt_password(std::string *out_password, int mode) {
  int c, password_size = 32, i = 0;
  std::string password, confirmed_password;

  toggle_visible_input();

  if (mode == 0) {
    std::cout << "*** Warning: The encrypted file will be unrecoverable "
                 "without this password ***"
              << std::endl;
  } else {
    std::cout
        << "*** Warning: Decrypting with the incorrect password can cause file "
           "corruption ***"
        << std::endl;
  }

  /* Prompt user for password and parse/store it */
  std::cout << "Enter a password: ";
  for (i = 0; (c = getchar()) != '\n' && c != EOF && i < password_size; i++) {
    password.push_back((char)c);
  }
  password.push_back((char)'\0');

  /* Prompt user to confirm password and parse/store it */
  std::cout << std::endl << "Confirm password: ";
  for (i = 0; (c = getchar()) != '\n' && c != EOF && i < password_size; i++) {
    confirmed_password.push_back((char)c);
  }
  confirmed_password.push_back((char)'\0');

  std::cout << std::endl;
  toggle_visible_input();

  /* Confirm that passwords match */
  if (password.compare(confirmed_password) == 0) {
    *out_password = password;
  } else {
    std::cout << std::endl << "Aborting. Passwords do not match." << std::endl;
    exit(-1);
  }
}

void print_progress(int _progress, int mode) {
  static int progress = 0, init = 0;

  /* Prevent printing if percentage hasn't changed */
  if (_progress <= progress || _progress % 2 != 0)
    return;
  else {
    progress = _progress;
    std::cout << "\r";
    std::cout.flush();
  }

  /* Toggle user input */
  if (init == 0) {
    toggle_visible_input();
    init = 1;
  }

  if (mode == 0)
    std::cout << "Encrypting - [";
  else
    std::cout << "Decrypting - [";

  int barWidth = 60;
  int pos = barWidth * (progress / 100.0);

  for (int i = 0; i < barWidth; ++i) {
    if (i < pos)
      std::cout << "=";
    else if (i == pos)
      std::cout << ">";
    else
      std::cout << " ";
  }

  std::cout << "] " << progress << "%\r";
  std::cout.flush();

  if (progress == 100) {
    std::cout << std::endl;
    toggle_visible_input();
  }
}

#endif  // IO_H_
