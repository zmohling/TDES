#ifndef DES_H_
#define DES_H_

#include <stdint.h>

class DES {
 public:
  DES();

  void encrypt(void *out, const void *in, const void *key);

  void decrypt(void *out, const void *in, const void *key);

 private:
  void permute(const uint8_t in_size, const uint8_t out_size,
               const uint8_t *in_block, uint8_t *out_block,
               const uint8_t *permute_table);

  void mixer(uint8_t *left_block, uint8_t *right_block,
             const uint8_t *round_key);

  void swapper(uint8_t *left_block, uint8_t *right_block);

  void feistel_function(const uint8_t *in_block, const uint8_t *round_key,
                        uint8_t *out_block);

  void substitute(const uint8_t *in_block, uint8_t *out_block);
};

#endif  // DES_H_
