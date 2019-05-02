#ifndef IO_H_
#define IO_H_

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <string>

void load_buffer_from_disk(std::string path, uint8_t **buffer,
                           uint64_t *length) {
  FILE *in_file_ptr;
  if (!(in_file_ptr = fopen(path.c_str(), "rb"))) {
    fprintf(stderr, "Could not open file %s. ERROR: %d", path.c_str(), errno);
  }

  fseek(in_file_ptr, 0, SEEK_END);
  *length = ftell(in_file_ptr);
  rewind(in_file_ptr);

  *buffer = (uint8_t *)malloc((*length + 1) * sizeof(uint8_t));
  fread(*buffer, *length, 1, in_file_ptr);

  fclose(in_file_ptr);
}

void write_buffer_to_disk(std::string path, uint8_t **buffer,
                          uint64_t *length) {
  FILE *out_file_ptr;
  if (!(out_file_ptr = fopen(path.c_str(), "wb"))) {
    fprintf(stderr, "Could not open file %s. ERROR: %d", path.c_str(), errno);
  }

  fwrite(*buffer, *length, 1, out_file_ptr);

  free(*buffer);

  fclose(out_file_ptr);
}
//
// void prompt_key(uint8_t key[8]) {
//  static struct termios oldt, newt;
//  int i = 0;
//  int c;
//
//  /* saving the old settings */
//  tcgetattr(STDIN_FILENO, &oldt);
//  newt = oldt;
//
//  newt.c_lflag &= ~(ECHO);
//
//  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
//
//  uint8_t SIZE = 32, password[32];
//
//  while ((c = getchar()) != '\n' && c != EOF && i < SIZE) {
//    password[i++] = c;
//  }
//  password[i] = '\0';
//
//  /*resetting our old STDIN_FILENO*/
//  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
//}
//
#endif  // IO_H_
