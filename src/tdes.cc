#include "tdes.h"

#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "ThreadPool.h"
#include "cipher.h"
#include "io.h"
#include "key_generator.h"

static std::mutex mtx;  // mutex for queues

static Cipher cipher;
static KeyGenerator keygen;

/* Subkeys of the 3 keys */
static uint8_t K1[16][6], K2[16][6], K3[16][6];

static FILE *in_file, *out_file;

static uint8_t *buffer;
static uint64_t file_length, write_length, processed_length;
static uint32_t R = 0, W = 0;

static std::priority_queue<uint8_t *> read_queue;
static std::map<uint8_t *, callback_container> write_queue;

static void add_PKCS5_padding(uint8_t *block) {
  uint8_t PKCS5_PADDING = 8 - (file_length % 8);

  int i;
  for (i = 0; i < PKCS5_PADDING; i++) {
    block[(8 - PKCS5_PADDING) + i] = PKCS5_PADDING;
  }
}

void run(int mode, std::string *in_file_name, std::string *out_file_name) {
  init_keys(&keygen, K1, K2, K3, mode);

  open_file(&in_file, *in_file_name, "rb", &file_length);
  open_file(&out_file, *out_file_name, "wb", NULL);

  if (mode == 0) {
    write_length =
        file_length +
        (BLOCK_SIZE - (file_length % BLOCK_SIZE));  // round to even block size
  } else {
    write_length = file_length;
  }

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
    while (true) {
      if (((R % 16) != (W % 16) && processed_length < file_length) || !init) {
        init = 1;

        uint32_t start_byte = ((R % NUM_BUFFERS) * BUFFER_SIZE);
        uint32_t end_byte = (start_byte + BUFFER_SIZE > write_length)
                                ? write_length
                                : start_byte + BUFFER_SIZE;

        std::cout << "Loading bytes " << start_byte << " through " << end_byte
                  << std::endl;

        std::thread read(read_task, buffer + start_byte,
                         (uint32_t)(end_byte - start_byte));
        read.join();

        /* Add callback container. Expected callbacks is equal to the number *
         * of threads we'll spawn for this chunk. (BUFFER_SIZE / 8)          */
        callback_container c;
        c.num_callbacks = 0;
        c.num_expected_callbacks =
            (uint32_t)(((end_byte - start_byte) - 1) / BLOCK_SIZE +
                       1);  // round-up integer division

        if ((write_length - (R * BUFFER_SIZE) < BUFFER_SIZE) && mode == 0) {
          add_PKCS5_padding(buffer + start_byte +
                            ((c.num_expected_callbacks - 1) * BLOCK_SIZE));
        }

        write_queue[buffer + start_byte] = c;

        /*
                std::cout << "Chunk " << start_byte << "'s Expected Callbacks: "
                          << write_queue[buffer +
           start_byte].num_expected_callbacks
                          << std::endl;
        */
        R++;
        processed_length += (uint32_t)(end_byte - start_byte);
      }

      while (!read_queue.empty()) {
        /*
        pool.enqueue(
            encrypt_task, (uint8_t *)read_queue.top(), &cipher,
            (std::map<const uint8_t *, callback_container> *)&write_queue, K1,
            K2, K3);
            */
        if (mode == 0)
          pool.enqueue(encrypt_task, (uint8_t *)read_queue.top());
        else
          pool.enqueue(decrypt_task, (uint8_t *)read_queue.top());

        read_queue.pop();
      }

      if (write_queue.size() > 0 &&
          write_queue[&buffer[(W % NUM_BUFFERS) * BUFFER_SIZE]].num_callbacks ==
              write_queue[&buffer[(W % NUM_BUFFERS) * BUFFER_SIZE]]
                  .num_expected_callbacks) {
        std::cout << "Writing blocks: " << (W % NUM_BUFFERS) * BUFFER_SIZE
                  << " through "
                  << (W % NUM_BUFFERS) * BUFFER_SIZE +
                         (write_queue[&buffer[(W % NUM_BUFFERS) * BUFFER_SIZE]]
                              .num_expected_callbacks *
                          8)
                  << std::endl;

        uint32_t num_bytes =
            write_queue[&buffer[(W % NUM_BUFFERS) * BUFFER_SIZE]]
                .num_expected_callbacks *
            8;

        if ((((W % 16) * BUFFER_SIZE) + num_bytes == file_length) &&
            mode == 1) {
          num_bytes -= buffer[file_length - 1];
          write_length -= buffer[file_length - 1];
        }

        std::thread write(write_task, buffer + ((W % 16) * BUFFER_SIZE),
                          num_bytes);
        write.join();

        W++;

        if (W == R) break;
      }
      // TODO: Write Logic
    }
  }
  // std::cout << "Write pointers: " << write_queue.size() << std::endl;
}

void read_task(uint8_t *buffer, uint32_t num_bytes) {
  int read_size;
  if (processed_length + num_bytes <= file_length) {
    read_size = num_bytes;
  } else {
    read_size = file_length - processed_length;
  }

  if (!fread(buffer, read_size, 1, in_file)) {
    printf("Error: could not read block starting at %lu. %s.\n",
           processed_length, strerror(errno));
    exit(-7);
  }

  mtx.lock();
  uint32_t i = 0;
  for (i = 0; i < num_bytes; i += 8) {
    read_queue.push(&buffer[i]);
  }
  mtx.unlock();
}

void write_task(uint8_t *buffer, uint32_t num_bytes) {
  if (!fwrite(buffer, num_bytes, 1, out_file)) {
    printf("Error: could not write block starting at %lu. %s.\n",
           processed_length, strerror(errno));
    exit(-7);
  }

  mtx.lock();
  write_queue.erase(buffer);
  mtx.unlock();
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

void encrypt_task(uint8_t *block) {
  uint8_t T1[8], T2[8];

  cipher.encrypt(T1, block, K1);
  cipher.decrypt(T2, T1, K2);
  cipher.encrypt(block, T2, K3);

  mtx.lock();
  write_queue[buffer + (((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE)]
      .num_callbacks++;
  /*
  std::cout << "Encrypted and Added " << (block - buffer) << " to write queue"
            << std::endl;
            */
  /*
  std::cout
      << "Callbacks increased to "
      << write_queue[buffer + (((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE)]
             .num_callbacks
      << " at " << ((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE << std::endl;
      */
  mtx.unlock();
}

void decrypt_task(uint8_t *block) {
  uint8_t T1[8], T2[8];

  cipher.decrypt(T1, block, K3);
  cipher.encrypt(T2, T1, K2);
  cipher.decrypt(block, T2, K1);

  mtx.lock();
  write_queue[buffer + (((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE)]
      .num_callbacks++;
  /*
  std::cout << "Encrypted and Added " << (block - buffer) << " to write queue"
            << std::endl;
            */
  /*
  std::cout
      << "Callbacks increased to "
      << write_queue[buffer + (((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE)]
             .num_callbacks
      << " at " << ((block - buffer) / BUFFER_SIZE) * BUFFER_SIZE << std::endl;
      */
  mtx.unlock();
}
