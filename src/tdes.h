#ifndef TDES_H_
#define TDES_H_

#include <map>
#include <queue>
#include <string>
#include <vector>

#include "cipher.h"
#include "key_generator.h"

typedef struct callback_container {
  uint32_t num_callbacks;
  uint32_t num_expected_callbacks;
} callback_container;

void init_keys(KeyGenerator *keygen, uint8_t K1[16][6], uint8_t K2[16][6],
               uint8_t K3[16][6], int mode);
void PKCS5_padding(uint8_t *in_block, uint8_t *out_block, int mode);

void encrypt_task(uint8_t *block);
void decrypt_task(uint8_t *block);
void write_task(FILE **file, uint8_t *buffer,
                std::map<uint8_t *, callback_container> *write_queue,
                uint32_t num_bytes, uint64_t *current_length,
                uint64_t *total_length);
void read_task(FILE **file, uint8_t *buffer,
               std::priority_queue<uint8_t *> *queue, uint32_t num_bytes,
               uint64_t *current_length, uint64_t *total_length);
void thread_dispatcher(void (*operation)(uint8_t *, uint8_t *));

/* Driving function. The crypto function accepts the parsed user inputs from *
 * main and applies the DES cryptography algorithm. The process loads bytes  *
 * into a buffer, encrypts or decrypts them, and writes them to a new file. */
void run(int mode, std::string *in_file_name, std::string *out_file_name);

#endif  // TDES_H_
