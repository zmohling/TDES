#ifndef IO_H_
#define IO_H_

#include <errno.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
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

void load_buffer_from_disk(std::string path, uint8_t **buffer,
                           uint64_t *length) {
  FILE *in_file_ptr;
  if (!(in_file_ptr = fopen(path.c_str(), "rb"))) {
    fprintf(stderr, "Could not open file %s. ERROR: %d", path.c_str(), errno);
    exit(-1);
  }

  fseek(in_file_ptr, 0, SEEK_END);
  *length = ftell(in_file_ptr);
  rewind(in_file_ptr);

  if (!(*buffer = (uint8_t *)malloc((*length + 1) * sizeof(uint8_t)))) {
    fprintf(stderr, "Insufficient memory for %s. ERROR: %d", path.c_str(),
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
    fprintf(stderr, "Could not open file %s. ERROR: %d", path.c_str(), errno);
    exit(-1);
  }

  fwrite(*buffer, *length, 1, out_file_ptr);

  free(*buffer);

  fclose(out_file_ptr);
}

void PBKDF2_HMAC_SHA_512_string(const char *pass, const unsigned char *salt,
                                int32_t iterations, uint32_t outputBytes,
                                char *hexResult) {
  unsigned int i;
  unsigned char digest[outputBytes];
  PKCS5_PBKDF2_HMAC(pass, strlen(pass), salt, strlen((const char *)salt),
                    iterations, EVP_sha512(), outputBytes, digest);
  for (i = 0; i < sizeof(digest); i++)
    sprintf(hexResult + (i * 2), "%02x", 255 & digest[i]);
}

void prompt_password(std::string *out_password) {
  static struct termios oldt, newt;
  int i = 0;
  int c;

  /* saving the old settings */
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;

  newt.c_lflag &= ~(ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  uint8_t SIZE = 32;
  std::string password, confirmed_password;

  std::cout << "*** Warning: Decrypting with the wrong password can cause file "
               "corruption ***"
            << std::endl;

  std::cout << "Enter a password: ";

  i = 0;
  while ((c = getchar()) != '\n' && c != EOF && i < SIZE) {
    password.push_back((char)c);
  }
  password.push_back((char)'\0');

  std::cout << std::endl << "Confirm password: ";

  i = 0;
  while ((c = getchar()) != '\n' && c != EOF && i < SIZE) {
    confirmed_password.push_back((char)c);
  }
  confirmed_password.push_back((char)'\0');

  /*
    char ERASE_CUR_LINE[6] = "\33[2K", MOVE_UP_ONE_LINE[6] = "\033[A",
         CARRIAGE_RETURN[2] = "\r";

    std::cout << ERASE_CUR_LINE << MOVE_UP_ONE_LINE << ERASE_CUR_LINE
              << MOVE_UP_ONE_LINE << CARRIAGE_RETURN;
  */

  std::cout << std::endl;

  /*resetting our old STDIN_FILENO*/
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  if (password.compare(confirmed_password) == 0) {
    *out_password = password;
  } else {
    std::cout << std::endl << "Aborting: Passwords do not match." << std::endl;
    exit(-1);
  }
}

void get_key(uint8_t key[8]) {
  std::string password;
  prompt_password(&password);

  const char *pass = password.c_str();

  if (!(PKCS5_PBKDF2_HMAC(pass, strlen(pass), NULL, 0, 1000, EVP_sha512(), 8,
                          key))) {
    fprintf(stderr, "Error while calculated password hash. ERROR: %d", errno);
    exit(-1);
  }
}

#endif  // IO_H_
