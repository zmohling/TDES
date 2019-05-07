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
#include <ctime>
#include <iostream>
#include <vector>

#include "tdes.h"

static bool does_option_exist(char **begin, char **end,
                              const std::string &option) {
  return std::find(begin, end, option) != end;
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

  /*
    if (is_original_file(out_file_name.c_str())) {
      fprintf(stderr, "Aborting. This file already exists: %s\n",
              out_file_name.c_str());
      exit(-1);
    }
  */

  run(mode, &in_file_name, &out_file_name);
}
