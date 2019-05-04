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

#ifndef IO_IO_H_
#define IO_IO_H_

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

int is_original_file(const char *filename) {
  struct stat st;
  int result = stat(filename, &st);
  return result == 0;
}

void load_buffer_from_disk(std::string path, uint8_t **buffer,
                           uint64_t *length) {
  FILE *in_file_ptr;
  if (!(in_file_ptr = fopen(path.c_str(), "rb"))) {
    fprintf(stderr, "Could not open file %s. ERROR: %d\n", path.c_str(), errno);
    exit(-1);
  }

  fseek(in_file_ptr, 0, SEEK_END);
  *length = ftell(in_file_ptr);
  rewind(in_file_ptr);

  int PKCS5_PADDING = 8 - (*length % 8);

  if (!(*buffer =
            (uint8_t *)malloc((*length + PKCS5_PADDING) * sizeof(uint8_t)))) {
    fprintf(stderr, "Insufficient memory for %s. ERROR: %d\n", path.c_str(),
            errno);
    exit(-1);
  }

  fread(*buffer, *length, 1, in_file_ptr);

  fclose(in_file_ptr);
}

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

void get_key(uint8_t key[8], int mode) {
  std::string password;
  prompt_password(&password, mode);

  const char *pass = password.c_str();

  /* Derive key from password using PBKDF2 with SHA512 */
  if (!(PKCS5_PBKDF2_HMAC(pass, strlen(pass), NULL, 0, 1000, EVP_sha512(), 8,
                          key))) {
    fprintf(stderr, "Error while deriving key from password. ERROR: %d", errno);
    exit(-1);
  }
}

void print_progress(int _progress, int mode) {
  static int progress = 0, init = 0;

  /* Prevent printing if percentage hasn't changed */
  if (_progress == progress)
    return;
  else
    progress = _progress;

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

#endif  // IO_IO_H_
