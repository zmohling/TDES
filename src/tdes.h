#ifndef TDES_H_
#define TDES_H_

#include <string>

#include "cipher.h"
#include "key_generator.h"

class TDES {
 private:
  Cipher cipher;
  KeyGenerator keygen;

  /* (3) 64-bit keys required by TDES */
  uint8_t K[24];

  /* Subkeys of the 3 keys */
  uint8_t K1[16][6], K2[16][6], K3[16][6];

  int mode;
  std::string in_file_name, out_file_name;

  void init_keys(int mode);
  void encrypt(uint8_t *in_block, uint8_t *out_block);
  void decrypt(uint8_t *in_block, uint8_t *out_block);
  void PKCS5_padding(uint8_t *in_block, uint8_t *out_block, int mode);

 public:
  TDES();
  ~TDES();

  /* Driving function. The crypto function accepts the parsed user inputs from *
   * main and applies the DES cryptography algorithm. The process loads bytes  *
   * into a buffer, encrypts or decrypts them, and writes them to a new file. */
  void run(int mode, std::string *in_file_name, std::string *out_file_name);
};

#endif  // TDES_H_
