#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include "rbuflogd/producer.h"
#include "common_types.h"
#include "ring_buffer.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  int shmem_fd;
  rbuf_t * rbuf;
} producer_state_t;

static int get_time_ns(clockid_t clock_id, uint64_t * out_ns) {
  struct timespec ts;

  if (out_ns == NULL) {
    return -1;
  }

  if (clock_gettime(clock_id, &ts) != 0) {
    return -1;
  }

  *out_ns = ((uint64_t) ts.tv_sec * 1000000000ULL) + (uint64_t) ts.tv_nsec;
  return 0;
}

int rbuflogd_producer_open(rbuflogd_producer_t * producer, const char * producer_name) {
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

  snprintf(producer->producer_name, RBUF_PROD_ID_MAX_LEN, "%s", producer_name);
  producer->state = state;

  return 0;
}

int rbuflogd_producer_log(rbuflogd_producer_t * producer, 
  rbuflogd_log_level_t level, const char * category, const char * log_msg) {
      
  if (producer == NULL || producer->state == NULL || log_msg == NULL) {
    return -1;
  }

  producer_state_t * state = (producer_state_t *) producer->state;

  rbuf_entry_t entry;
  if (get_time_ns(CLOCK_REALTIME, &entry.realtime_ns) != 0) {
    return -1;
  }

  if (get_time_ns(CLOCK_MONOTONIC, &entry.monotonic_ns) != 0) {
    return -1;
  }

  snprintf(entry.producer_name, RBUF_PROD_ID_MAX_LEN, "%s", producer->producer_name);
  entry.level = level;
  snprintf(entry.category, RBUF_LOG_CATEGORY_LEN, "%s", category);
  snprintf(entry.msg, RBUF_MSG_MAX_LEN, "%s", log_msg);

  printf("Producer \"%s\" logging: \"%s\"\n", producer->producer_name, log_msg);

  return rbuf_try_push(state->rbuf, &entry);
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
