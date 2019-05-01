#include <gtest/gtest.h>
#include "../src/key_generator.h"

TEST(LeftShiftTest, SplitKeyTest1) {
  uint8_t block[4] = {1, 2, 3, 4};
  uint64_t *left_block = 0, *right_block = 0;

  KeyGenerator k;
  k.split_keys((const uint8_t *)block, (uint64_t *)left_block,
               (uint64_t *)right_block);

  // EXPECT_EQ(, 1);
}
