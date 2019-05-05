#include "tdes.h"

#include <thread>
#include <vector>

#include "ThreadPool.h"
#include "cipher.h"
#include "io.h"
#include "key_generator.h"

TDES::TDES(){};

TDES::~TDES(){};

void TDES::run(int mode, std::string *in_file_name,
               std::string *out_file_name) {
  uint8_t *read_buffer;
  uint64_t length, cur_length = 0;
  uint64_t progress = 0;

  init_keys(mode);

  load_buffer_from_disk(*in_file_name, &read_buffer, &length);

  uint8_t PKCS5_PADDING = 8 - (length % 8);
  uint8_t *write_buffer =
      (uint8_t *)malloc((length + PKCS5_PADDING) * sizeof(uint8_t));

  using nbsdx::concurrent::ThreadPool;

  ThreadPool<10> pool;

  print_progress(0, mode);

  while ((cur_length + 8) <= length) {
    if (mode == 0) {
      pool.AddJob([this, read_buffer, cur_length, write_buffer, progress, mode,
                   length]() {
        encrypt(&read_buffer[cur_length], &write_buffer[cur_length]);
        print_progress((((float)cur_length) / ((float)length)) * 100, mode);
      });
    } else {
      pool.AddJob([this, read_buffer, cur_length, write_buffer, progress, mode,
                   length]() {
        decrypt(&read_buffer[cur_length], &write_buffer[cur_length]);
        print_progress((((float)cur_length) / ((float)length)) * 100, mode);
      });
    }

    cur_length += 8;
  }

  pool.JoinAll();

  /* PKCS#5 Padding on last block */
  if (mode == 0) {
    int i;
    for (i = 0; i < PKCS5_PADDING; i++) {
      read_buffer[cur_length + (8 - PKCS5_PADDING) + i] = PKCS5_PADDING;
    }

    length += PKCS5_PADDING;

    encrypt(&read_buffer[cur_length], &write_buffer[cur_length]);

  } else {
    length -= write_buffer[cur_length - 1];
  }

  free(read_buffer);

  write_buffer_to_disk(*out_file_name, &write_buffer, &length);

  print_progress(100, mode);
}

void TDES::encrypt(uint8_t *in_block, uint8_t *out_block) {
  uint8_t T1[8], T2[8];

  cipher.encrypt(T1, in_block, K1);
  cipher.decrypt(T2, T1, K2);
  cipher.encrypt(out_block, T2, K3);
}

void TDES::decrypt(uint8_t *in_block, uint8_t *out_block) {
  uint8_t T1[8], T2[8];

  cipher.decrypt(T1, in_block, K3);
  cipher.encrypt(T2, T1, K2);
  cipher.decrypt(out_block, T2, K1);
}

void TDES::init_keys(int mode) {
  std::string password;

  prompt_password(&password, mode);

  /* Derive key from password using PBKDF2 with SHA512 */
  if (!(PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), NULL, 0, 100000,
                          EVP_sha512(), 24, K))) {
    fprintf(stderr, "Error while deriving key from password. ERROR: %d", errno);
    exit(-1);
  }

  keygen.generate(K, K1);
  keygen.generate(K + 8, K2);
  keygen.generate(K + 16, K3);
}
