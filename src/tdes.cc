#include "tdes.h"

#include <chrono>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "ThreadPool.h"
#include "cipher.h"
#include "io.h"
#include "key_generator.h"

std::mutex mtx;  // mutex for queues

void encrypt_task(uint8_t *block, Cipher *cipher,
                  std::priority_queue<uint8_t *> *queue, uint8_t K1[16][6],
                  uint8_t K2[16][6], uint8_t K3[16][6]) {
  uint8_t T1[8], T2[8];

  cipher->encrypt(T1, block, K1);
  cipher->decrypt(T2, T1, K2);
  cipher->encrypt(block, T2, K3);

  mtx.lock();
  queue->push(block);
  mtx.unlock();
}

void decrypt_task(uint8_t *block, Cipher *cipher,
                  std::priority_queue<uint8_t *> *queue, uint8_t K1[16][6],
                  uint8_t K2[16][6], uint8_t K3[16][6]) {
  uint8_t T1[8], T2[8];

  cipher->decrypt(T1, block, K3);
  cipher->encrypt(T2, T1, K2);
  cipher->decrypt(block, T2, K1);

  mtx.lock();
  queue->push(block);
  mtx.unlock();
}

void read_task(FILE **file, uint8_t *buffer,
               std::priority_queue<uint8_t *> *queue, uint32_t num_bytes,
               uint64_t *current_length, uint64_t *total_length) {
  int read_size;
  if (*current_length + num_bytes <= *total_length) {
    read_size = num_bytes;
  } else {
    read_size = *total_length - *current_length;
  }

  if (!fread(buffer, read_size, 1, *file)) {
    printf("Error: could not read block starting at %lu. %s.\n",
           *current_length, strerror(errno));
    exit(-7);
  }

  uint32_t i = 0;
  for (i = 0; i < num_bytes; i += 8) {
    queue->push(&buffer[i]);
  }
}

void write_task(FILE **file, uint8_t *buffer,
                std::priority_queue<uint8_t *> *queue, uint32_t num_bytes,
                uint64_t *current_length, uint64_t *total_length) {
  /*
  int write_size;
  if (*current_length + num_bytes <= *total_length) {
    write_size = num_bytes;
  } else {
    write_size = *total_length - *current_length;
  }*/

  if (!fwrite(buffer, *total_length, 1, *file)) {
    printf("Error: could not write block starting at %lu. %s.\n",
           *current_length, strerror(errno));
    exit(-7);
  }

  uint32_t i = 0;
  for (i = 0; i < num_bytes; i += 8) {
    queue->pop();
  }
}

void run(int mode, std::string *in_file_name, std::string *out_file_name) {
  Cipher cipher;
  KeyGenerator keygen;

  /* (3) 64-bit keys required by TDES */

  /* Subkeys of the 3 keys */
  uint8_t K1[16][6], K2[16][6], K3[16][6];

  FILE *in_file, *out_file;

  uint8_t *buffer;
  uint64_t total_length, current_length;
  uint32_t R = 0, W = 0;

  std::priority_queue<uint8_t *> read_queue, write_queue;

  init_keys(&keygen, K1, K2, K3, mode);
  // uint8_t PKCS5_PADDING = 8 - (length % 8);

  open_file(&in_file, *in_file_name, "rb", &total_length);
  open_file(&out_file, *out_file_name, "wb", NULL);
  /*
    if (!(buffer = (uint8_t *)malloc(total_length * sizeof(uint8_t)))) {
      fprintf(stderr, "Insufficient memory. ERROR: %d\n", errno);
      exit(-1);
    }
  */
  if (!(buffer =
            (uint8_t *)malloc(BUFFER_SIZE * NUM_BUFFERS * sizeof(uint8_t)))) {
    fprintf(stderr, "Insufficient memory. ERROR: %d\n", errno);
    exit(-1);
  }
  /*
    {
      ThreadPool pool(10);

      auto result = pool.enqueue(read_task, &in_file, &buffer, &read_queue,
                                 total_length, &current_length, &total_length);
      result.get();

      while ((current_length + 8) <= total_length) {
        if (mode == 0) {
          pool.enqueue(encrypt_task, &buffer[current_length], &cipher, K1, K2,
                       K3);
        } else {
          pool.enqueue(decrypt_task, &buffer[current_length], &cipher, K1, K2,
                       K3);
        }

        current_length += 8;
      }
    }
    int i;
    for (i = 0; i < total_length; i++) {
      printf("%c", buffer[i]);
    }

  write_task(&out_file, &buffer, &read_queue, total_length, &current_length,
             &total_length);

  */
  {
    static int init = 0;

    ThreadPool pool(8);
    while (current_length < total_length) {
      if (R % 16 != W % 16 || !init) {
        init = 1;
        uint32_t start_byte = ((R % NUM_BUFFERS) * BUFFER_SIZE);
        uint32_t end_byte = (start_byte + (2 * BUFFER_SIZE) > total_length)
                                ? total_length
                                : start_byte + (2 * BUFFER_SIZE);
        std::cout << "Loading bytes " << start_byte << " through " << end_byte
                  << std::endl;
        std::thread read(read_task, &in_file, buffer + start_byte, &read_queue,
                         (uint32_t)(end_byte - start_byte), &current_length,
                         &total_length);
        read.join();

        std::cout << "Write pointers: " << write_queue.size() << std::endl;

        R += 2;
      }

      if (!read_queue.empty()) {
        while (!read_queue.empty()) {
          pool.enqueue(encrypt_task, (uint8_t *)read_queue.top(), &cipher,
                       &write_queue, K1, K2, K3);
          read_queue.pop();
          current_length += 8;
        }
      }

      // TODO: Write Logic
    }
  }
  std::cout << "Write pointers: " << write_queue.size() << std::endl;
  /*write_task(&out_file, buffer, &read_queue, total_length, &current_length,
             &total_length);
             */
}

void init_keys(KeyGenerator *keygen, uint8_t K1[16][6], uint8_t K2[16][6],
               uint8_t K3[16][6], int mode) {
  std::string password;

  prompt_password(&password, mode);

  uint8_t K[24];
  // memset(K, 0, 24);

  /* Derive key from password using PBKDF2 with SHA512 */
  if (!(PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), NULL, 0, 100000,
                          EVP_sha512(), 24, K))) {
    fprintf(stderr, "Error while deriving key from password. ERROR: %d", errno);
    exit(-1);
  }

  keygen->generate(K, K1);
  keygen->generate(K + 8, K2);
  keygen->generate(K + 16, K3);
}
