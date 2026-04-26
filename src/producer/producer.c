#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include "rbuflogd/producer.h"
#include "common_types.h"
#include "ring_buffer.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  int shmem_fd;
  rbuf_t * rbuf;
} producer_state_t;

static void copy_bounded_text(char * dst, size_t dst_len, const char * src) {
  size_t i = 0;

  if (dst == NULL || dst_len == 0) {
    return;
  }

  memset(dst, 0, dst_len);
  if (src == NULL) {
    return;
  }

  while (i < dst_len && src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
}

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
  if (producer == NULL || producer_name == NULL) {
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

  copy_bounded_text(producer->producer_name, sizeof(producer->producer_name), producer_name);
  producer->state = state;

  return 0;
}

int rbuflogd_producer_log(rbuflogd_producer_t * producer, 
  rbuflogd_log_level_t level, const char * category, const char * log_msg) {
      
  if (producer == NULL || producer->state == NULL || category == NULL || log_msg == NULL) {
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

  copy_bounded_text(entry.producer_name, sizeof(entry.producer_name), producer->producer_name);
  entry.level = level;
  copy_bounded_text(entry.category, sizeof(entry.category), category);
  snprintf(entry.msg, RBUF_MSG_MAX_LEN, "%s", log_msg);

  printf(
    "Producer \"%.*s\" logging: \"%s\"\n",
    RBUF_PRODUCER_ID_DISPLAY_CHARS,
    producer->producer_name,
    log_msg);

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
