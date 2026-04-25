#define _POSIX_C_SOURCE 200809L

#include "rbuflogd/producer.h"
#include "ring_buffer.h"

#include <fcntl.h>
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

  return rbuf_try_push(producer->rbuf, log_msg);
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
