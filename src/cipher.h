#ifndef CIPHER_H_
#define CIPHER_H_

#include <stdint.h>

class Cipher {
 public:
  Cipher();

  void encrypt(uint8_t *out, const uint8_t *in, const uint8_t *sub_keys[]);

  void decrypt(uint8_t *out, const uint8_t *in, const uint8_t *sub_keys[]);

 private:
  void swapper(uint8_t bytes, uint8_t *left_block, uint8_t *right_block);

  void feistel_function(const uint8_t *in_block, const uint8_t *round_key,
                        uint8_t *out_block);

  void substitute(const uint8_t *in_block, uint8_t *out_block);
};

void permute(const uint8_t in_bytes, const uint8_t out_bytes,
             const uint8_t *in_block, uint8_t *out_block,
             const uint8_t *permute_table);

void split(const uint8_t in_bytes, const uint8_t out_bytes,
           const uint8_t *in_block, uint8_t *left_block, uint8_t *right_block);

void combine(const uint8_t in_bytes, const uint8_t out_bytes,
             const uint8_t *left_block, const uint8_t *right_block,
             uint8_t *out_block);

void exclusive_or(const uint8_t bytes, const uint8_t *first_block,
                  const uint8_t *second_block, uint8_t *out_block);

#endif  // CIPHER_H_
