#include "ring_buffer.h"
#include "common_types.h"

#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

int rbuf_is_empty(const rbuf_t * rbuf) {
  if (rbuf == NULL) {
    return 1;
  }

  const size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_relaxed);
  const size_t head = atomic_load_explicit(&rbuf->head, memory_order_acquire);
  return tail == head;
}

int rbuf_is_full(const rbuf_t * rbuf) {
  if (rbuf == NULL) {
    return 0;
  }

  const size_t head = atomic_load_explicit(&rbuf->head, memory_order_relaxed);
  const size_t next_head = (head + 1) % RBUF_SIZE;
  const size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_acquire);
  return next_head == tail;
}

int rbuf_try_push(rbuf_t * rbuf, const rbuf_entry_t * entry) {
  if (rbuf == NULL || entry == NULL) {
    return -1;
  }

  const size_t head = atomic_load_explicit(&rbuf->head, memory_order_relaxed);
  const size_t next_head = (head + 1) % RBUF_SIZE;
  const size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_acquire);

  if (next_head == tail) {
    return -1;
  }

  memcpy(&rbuf->data[head], entry, sizeof(rbuf_entry_t));
  atomic_store_explicit(&rbuf->head, next_head, memory_order_release);
  return 0;
}

int rbuf_try_pop(rbuf_t * rbuf, rbuf_entry_t * out_entry) {
  if (rbuf == NULL || out_entry == NULL) {
    return -1;
  }

  const size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_relaxed);
  const size_t head = atomic_load_explicit(&rbuf->head, memory_order_acquire);

  if (tail == head) {
    return -1;
  }

  memcpy(out_entry, &rbuf->data[tail], sizeof(rbuf_entry_t));
  atomic_store_explicit(&rbuf->tail, (tail + 1) % RBUF_SIZE, memory_order_release);
  return 0;
}

void rbuf_reset(rbuf_t * rbuf) {
  if (rbuf == NULL) {
    return;
  }

  memset(rbuf->data, 0, sizeof(rbuf->data));
  atomic_store_explicit(&rbuf->head, 0, memory_order_release);
  atomic_store_explicit(&rbuf->tail, 0, memory_order_release);
}
