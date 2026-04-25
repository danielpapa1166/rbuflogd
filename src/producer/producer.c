#define _POSIX_C_SOURCE 200809L

#include "rbuflogd/producer.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int rbuflogd_producer_open(rbuflogd_producer_t * producer) {
  if (producer == NULL) {
    return -1;
  }

  producer->shmem_fd = shm_open(SHMEM_NAME, O_RDWR, 0);
  if (producer->shmem_fd == -1) {
    return -1;
  }

  producer->rbuf = mmap(
    NULL,
    sizeof(rbuf_t),
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    producer->shmem_fd,
    0);

  if (producer->rbuf == MAP_FAILED) {
    close(producer->shmem_fd);
    producer->shmem_fd = -1;
    producer->rbuf = NULL;
    return -1;
  }

  return 0;
}

int rbuflogd_producer_log(rbuflogd_producer_t * producer, const char * log_msg) {
  if (producer == NULL || producer->rbuf == NULL || log_msg == NULL) {
    return -1;
  }

  size_t head = atomic_load_explicit(&producer->rbuf->head, memory_order_relaxed);
  size_t next_head = (head + 1) % RBUF_SIZE;
  size_t tail = atomic_load_explicit(&producer->rbuf->tail, memory_order_acquire);

  if (next_head == tail) {
    return -1;
  }

  snprintf(producer->rbuf->data[head], RBUF_MSG_MAX_LEN, "%s", log_msg);
  atomic_store_explicit(&producer->rbuf->head, next_head, memory_order_release);

  return 0;
}

void rbuflogd_producer_close(rbuflogd_producer_t * producer) {
  if (producer == NULL) {
    return;
  }

  if (producer->rbuf != NULL) {
    munmap(producer->rbuf, sizeof(rbuf_t));
    producer->rbuf = NULL;
  }

  if (producer->shmem_fd != -1) {
    close(producer->shmem_fd);
    producer->shmem_fd = -1;
  }
}
