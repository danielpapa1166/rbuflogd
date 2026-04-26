#define _POSIX_C_SOURCE 200809L

#include "rbuflogd/producer.h"
#include "common_types.h"
#include "ring_buffer.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {
  int shmem_fd;
  rbuf_t * rbuf;
} producer_state_t;

int rbuflogd_producer_open(rbuflogd_producer_t * producer) {
  if (producer == NULL) {
    return -1;
  }

  if (producer->state != NULL) {
    return -1;
  }

  producer_state_t * state = calloc(1, sizeof(*state));
  if (state == NULL) {
    return -1;
  }
  state->shmem_fd = -1;

  state->shmem_fd = shm_open(SHMEM_NAME, O_RDWR, 0);
  if (state->shmem_fd == -1) {
    free(state);
    return -1;
  }

  state->rbuf = mmap(
    NULL,
    sizeof(rbuf_t),
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    state->shmem_fd,
    0);

  if (state->rbuf == MAP_FAILED) {
    close(state->shmem_fd);
    state->shmem_fd = -1;
    state->rbuf = NULL;
    free(state);
    return -1;
  }

  producer->state = state;

  return 0;
}

int rbuflogd_producer_log(rbuflogd_producer_t * producer, const char * log_msg) {
  if (producer == NULL || producer->state == NULL || log_msg == NULL) {
    return -1;
  }

  producer_state_t * state = (producer_state_t *) producer->state;

  return rbuf_try_push(state->rbuf, log_msg);
}

void rbuflogd_producer_close(rbuflogd_producer_t * producer) {
  if (producer == NULL || producer->state == NULL) {
    return;
  }

  producer_state_t * state = (producer_state_t *) producer->state;

  if (state->rbuf != NULL) {
    munmap(state->rbuf, sizeof(rbuf_t));
    state->rbuf = NULL;
  }

  if (state->shmem_fd != -1) {
    close(state->shmem_fd);
    state->shmem_fd = -1;
  }

  free(state);
  producer->state = NULL;
}
