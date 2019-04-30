#include <stdio.h>

#include "cipher.h"

int main(int argc, char *argv[]) {
  char char_arr[4] = {'t', 'e', 's', 't'};

  uint8_t left_block[2], right_block[2];

  split(4, 2, (uint8_t *)char_arr, left_block, right_block);

  printf("left: %c%c, right: %c%c", left_block[0], left_block[1],
         right_block[0], right_block[1]);

  return 0;
}
