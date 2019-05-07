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

#include "tdes.h"

#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../lib/ThreadPool.h"
#include "cipher.h"
#include "io.h"
#include "key_generator.h"

static std::mutex queue_mtx;
static std::mutex map_mtx;

/* DES Cipher */
static Cipher cipher;

/* DES Key Generator */
static KeyGenerator keygen;

/* Subkeys of the 3 keys */
static uint8_t K1[16][6], K2[16][6], K3[16][6];

/* File pointers to our source and destination files */
static FILE *in_file, *out_file;

/* Circular buffer */
static uint8_t *buffer;

/* Pointers which point to a chunk of data in buffer. R points to the *
 * next chunk to read data into from disk, while W points to the next *
 * chunk which to read from the buffer and write to disk.             */
static uint32_t R = 0, W = 0;

static uint64_t in_file_length, out_file_length, read_length, write_length;
static uint64_t num_operations;

/* Queue of pointers to 8-byte blocks to be encrypted/decrypted. Queue*
 * gets populated by the read_task.                                   */
static std::priority_queue<uint8_t *> read_queue;

/* Map of pointer-to-chunk keys, will value struct that accounts for *
 * the numbers of jobs (encrypting a block) to complete and callback *
 * for a block in the chunk. When callback_container.num_callbacks ==*
 * callback_container.num_expected_callbacks, entry will be erased   *
 * from the map, and the chunk will be written to disk.              */
static std::map<uint8_t *, callback_container> write_map;

/* Add padding the last block of the file. To PKCS#5 specification.  */
static void add_PKCS5_padding(uint8_t *block) {
  uint8_t i, PKCS5_PADDING = 8 - (in_file_length % 8);
  for (i = 0; i < PKCS5_PADDING; i++) {
    block[(8 - PKCS5_PADDING) + i] = PKCS5_PADDING;
  }
}

/* Averages lengths and counters and calls IO's progress function.  */
static void update_progress(int mode) {
  float OPER_WEIGHT = 0.7, READ_WEIGHT = 0.15, WRITE_WEIGHT = 0.15;

  float percentage =
      (OPER_WEIGHT * ((float)num_operations /
                      ((float)in_file_length / (float)BLOCK_SIZE)) +
       READ_WEIGHT * ((float)read_length / (float)in_file_length) +
       WRITE_WEIGHT * ((float)write_length / (float)out_file_length));

  print_progress(percentage * 100.0, mode);
}

/* Driving function. Calls IO functions to derive keys from user's    *
 * password, opens files, allocates buffer. Contains loop for reading *
 * in data, adding jobs to the thread pool, and writing data.         */
void run(int mode, std::string *in_file_name, std::string *out_file_name) {
  open_file(&in_file, *in_file_name, "rb", &in_file_length);
  open_file(&out_file, *out_file_name, "wb", NULL);

  init_keys(&keygen, K1, K2, K3, mode);

  print_progress(0, mode);

  /* Adjust write_length if encrypting for padding */
  if (mode == 0) {
    /* round to even 8-byte block size */
    out_file_length =
        in_file_length + (BLOCK_SIZE - (in_file_length % BLOCK_SIZE));
  } else {
    out_file_length = in_file_length;
  }

  /* Allocate memory for our 16-chunk, 2^16 bit circular buffer */
  if (!(buffer =
            (uint8_t *)malloc(BUFFER_SIZE * NUM_BUFFERS * sizeof(uint8_t)))) {
    fprintf(stderr, "Insufficient memory. ERROR: %d\n", errno);
    exit(-1);
  }

  /* Thread pool for encryption and decryption operations */
  ThreadPool pool(8);

  static int init = 0;
  while (true) {
    if (((R % 16) != (W % 16) && (read_length < in_file_length)) || !init) {
      init = 1;

      uint32_t start_byte = ((R % NUM_BUFFERS) * BUFFER_SIZE);
      uint32_t end_byte = ((R * BUFFER_SIZE) + BUFFER_SIZE > out_file_length)
                              ? start_byte + (out_file_length % BUFFER_SIZE)
                              : start_byte + BUFFER_SIZE;

      /* Utilize thread pool for reading to save on thread-creation costs */
      auto read = pool.enqueue(read_task, buffer + start_byte,
                               (uint32_t)(end_byte - start_byte));
      read.get();

      /* Add callback container. Expected callbacks is equal to the number *
       * of threads we'll spawn for this chunk. (BUFFER_SIZE / 8)          */
      callback_container c;
      c.num_callbacks = 0;
      c.num_expected_callbacks =
          (uint32_t)(((end_byte - start_byte) - 1) / BLOCK_SIZE +
                     1);  // round-up integer division

      /* Add padding to last block of file. This padding ensures that the *
       * total new file length will evenly divide into BLOCK_SIZE. Padding*
       * is to PKCS#5 specification.                                      */
      if ((out_file_length - (R * BUFFER_SIZE) < BUFFER_SIZE) && mode == 0) {
        add_PKCS5_padding(buffer + start_byte +
                          ((c.num_expected_callbacks - 1) * BLOCK_SIZE));
      }

      /* Add pointer to chunk and callback to map */
      write_map[buffer + start_byte] = c;

      R++;

      read_length += (uint32_t)(end_byte - start_byte);
    }

    /* Add any queued pointers (to a 8-byte block in buffer) to threadpool *
     * job queue.                                                          */
    while (!read_queue.empty()) {
      if (mode == 0)
        pool.enqueue(encrypt_task, read_queue.top());
      else
        pool.enqueue(decrypt_task, read_queue.top());

      read_queue.pop();
    }

    /* Check if threads spawned for encryting/decrypting chunk W have all *
     * completed by ensuring num_callbacks == num_expected_callbacks. If  *
     * so, write completed chunk W to disk. If decrypting, change         *
     * num_bytes to reflect de-padding the last 8-byte block.             */

    callback_container callback =
        write_map[&buffer[(W % NUM_BUFFERS) * BUFFER_SIZE]];

    if (callback.num_callbacks == callback.num_expected_callbacks) {
      uint32_t num_bytes = callback.num_expected_callbacks * 8;

      /* If last block, de-pad by shortening the length of the write     *
       * operation.                                                      */
      if (((W * BUFFER_SIZE) + num_bytes == out_file_length) && mode == 1) {
        num_bytes -= buffer[(in_file_length % (BUFFER_SIZE * NUM_BUFFERS)) - 1];
      }

      /* Utilize thread pool for writing to save on thread-creation costs*/
      auto write = pool.enqueue(
          write_task, buffer + ((W % NUM_BUFFERS) * BUFFER_SIZE), num_bytes);
      write.get();

      W++;

      write_length += num_bytes;

      /* If reading has stopped, W (write) will eventually catch-up to   *
       * R (read) location. That means we're done.                       */
      if (W == R) break;
    }

    update_progress(mode);
  }

  print_progress(100, mode);

  free(buffer);

  fclose(in_file);

  fclose(out_file);
}

/* Initialize set of keys for Triple DES. Derives the cumulative 24    *
 * bytes from user's password.                                         */
void init_keys(KeyGenerator *keygen, uint8_t K1[16][6], uint8_t K2[16][6],
               uint8_t K3[16][6], int mode) {
  std::string password;

  prompt_password(&password, mode);

  uint8_t K[24];

  /* Derive key from password using PBKDF2 with SHA512 */
  if (!(PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), NULL, 0, 100000,
                          EVP_sha512(), 24, K))) {
    fprintf(stderr, "Error while deriving key from password. ERROR: %d", errno);
    exit(-1);
  }

  /* Generate set of 16 subkeys from the set of 8-byte keys */
  keygen->generate(K, K1);
  keygen->generate(K + 8, K2);
  keygen->generate(K + 16, K3);
}

/* Read a chunk into the circular buffer. For each block read, add a     *
 * pointer to the read_queue.                                            */
void read_task(uint8_t *buffer, uint32_t num_bytes) {
  /* Bounds checking */
  int read_size;
  if (read_length + num_bytes <= in_file_length) {
    read_size = num_bytes;
  } else {
    read_size = in_file_length - read_length;
  }

  if (!fread(buffer, read_size, 1, in_file)) {
    printf("Error: could not read block starting at %lu. %s.\n", read_length,
           strerror(errno));
    exit(-7);
  }

  /* Add pointers to each block to read_queue */
  queue_mtx.lock();

  uint32_t i = 0;
  for (i = 0; i < num_bytes; i += 8) {
    read_queue.push(&buffer[i]);
  }

  queue_mtx.unlock();
}

/* Write a chunk to disk from the cirular queue. On completiion, delete *
 * pointer to that chunk.                                               */
void write_task(uint8_t *buffer, uint32_t num_bytes) {
  if (!fwrite(buffer, num_bytes, 1, out_file)) {
    printf("Error: could not write block starting at %lu. %s.\n", write_length,
           strerror(errno));
    exit(-7);
  }

  /* Erase chunk-pointer key from write_map */
  map_mtx.lock();

  write_map.erase(buffer);

  map_mtx.unlock();
}

/* Encrypt the block. On completion, remove block from read_queue and *
 * add increment the num_callbacks member of the callback_container   *
 * value of write_map.                                                */
void encrypt_task(uint8_t *block) {
  uint8_t T1[8], T2[8];

  cipher.encrypt(T1, block, K1);
  cipher.decrypt(T2, T1, K2);
  cipher.encrypt(block, T2, K3);

  map_mtx.lock();

  write_map[buffer + (((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE)]
      .num_callbacks++;

  num_operations++;

  map_mtx.unlock();
}

/* Decrypt the block. On completion, remove block from read_queue and *
 * add increment the num_callbacks member of the callback_container   *
 * value of write_map.                                                */
void decrypt_task(uint8_t *block) {
  uint8_t T1[8], T2[8];

  cipher.decrypt(T1, block, K3);
  cipher.encrypt(T2, T1, K2);
  cipher.decrypt(block, T2, K1);

  map_mtx.lock();

  write_map[buffer + (((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE)]
      .num_callbacks++;

  num_operations++;

  map_mtx.unlock();
}
