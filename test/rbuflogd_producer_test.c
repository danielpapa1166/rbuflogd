#define _POSIX_C_SOURCE 200809L

#include "rbuflogd_common_types.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define TEST_MSG_COUNT 10

int main(void) {
  int fd = shm_open(SHMEM_NAME, O_RDWR, 0);
  if (fd == -1) {
    perror("shm_open");
    return -1;
  }

  rbuf_t *rbuf = mmap(NULL, sizeof(rbuf_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  if (rbuf == MAP_FAILED) {
    perror("mmap");
    return -1;
  }

  for (int i = 0; i < TEST_MSG_COUNT; i++) {
    size_t head = atomic_load_explicit(&rbuf->head, memory_order_relaxed);
    size_t next_head = (head + 1) % RBUF_SIZE;
    size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_acquire);

    if (next_head == tail) {
      printf("Buffer full, stopping at message %d\n", i);
      break;
    }

    snprintf(rbuf->data[head], RBUF_MSG_MAX_LEN, "test message %d", i);

    atomic_store_explicit(&rbuf->head, next_head, memory_order_release);
    printf("Wrote: \"%s\" (head -> %zu)\n", rbuf->data[head], next_head);
  }

  munmap(rbuf, sizeof(rbuf_t));
  return 0;
}
