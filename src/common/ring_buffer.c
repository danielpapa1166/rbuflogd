#include "ring_buffer.h"
#include "common_types.h"

#include <stdint.h>
#include <stdatomic.h>
#include <string.h>

int rbuf_is_empty(const rbuf_t * rbuf) {
  if (rbuf == NULL) {
    return 1;
  }

  const size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_relaxed);
  const rbuf_slot_t * slot = &rbuf->slots[tail % DEFAULT_RING_BUFFER_SIZE];
  const size_t seq = atomic_load_explicit(&slot->seq, memory_order_acquire);
  const intptr_t dif = (intptr_t) seq - (intptr_t) (tail + 1);

  return dif < 0;
}

int rbuf_is_full(const rbuf_t * rbuf) {
  if (rbuf == NULL) {
    return 0;
  }

  const size_t head = atomic_load_explicit(&rbuf->head, memory_order_relaxed);
  const rbuf_slot_t * slot = &rbuf->slots[head % DEFAULT_RING_BUFFER_SIZE];
  const size_t seq = atomic_load_explicit(&slot->seq, memory_order_acquire);
  const intptr_t dif = (intptr_t) seq - (intptr_t) head;

  return dif < 0;
}

int rbuf_try_push(rbuf_t * rbuf, const rbuf_entry_t * entry) {
  size_t head;
  rbuf_slot_t * slot;

  if (rbuf == NULL || entry == NULL) {
    return -1;
  }

  for (;;) {
    size_t seq;
    intptr_t dif;

    head = atomic_load_explicit(&rbuf->head, memory_order_relaxed);
    slot = &rbuf->slots[head % DEFAULT_RING_BUFFER_SIZE];
    seq = atomic_load_explicit(&slot->seq, memory_order_acquire);
    dif = (intptr_t) seq - (intptr_t) head;

    if (dif < 0) {
      return -1;
    }

    if (dif == 0 && atomic_compare_exchange_weak_explicit(
      &rbuf->head,
      &head,
      head + 1,
      memory_order_relaxed,
      memory_order_relaxed)) {
      break;
    }
  }

  memcpy(&slot->entry, entry, sizeof(rbuf_entry_t));
  atomic_store_explicit(&slot->seq, head + 1, memory_order_release);
  return 0;
}

int rbuf_try_pop(rbuf_t * rbuf, rbuf_entry_t * out_entry) {
  const size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_relaxed);
  rbuf_slot_t * slot;
  size_t seq;
  intptr_t dif;

  if (rbuf == NULL || out_entry == NULL) {
    return -1;
  }

  slot = &rbuf->slots[tail % DEFAULT_RING_BUFFER_SIZE];
  seq = atomic_load_explicit(&slot->seq, memory_order_acquire);
  dif = (intptr_t) seq - (intptr_t) (tail + 1);

  if (dif < 0) {
    return -1;
  }

  memcpy(out_entry, &slot->entry, sizeof(rbuf_entry_t));
  atomic_store_explicit(&slot->seq, tail + DEFAULT_RING_BUFFER_SIZE, memory_order_release);
  atomic_store_explicit(&rbuf->tail, tail + 1, memory_order_relaxed);

  return 0;
}

void rbuf_reset(rbuf_t * rbuf) {
  if (rbuf == NULL) {
    return;
  }

  memset(rbuf->slots, 0, sizeof(rbuf->slots));
  for (size_t i = 0; i < DEFAULT_RING_BUFFER_SIZE; i++) {
    atomic_store_explicit(&rbuf->slots[i].seq, i, memory_order_relaxed);
  }

  atomic_store_explicit(&rbuf->head, 0, memory_order_relaxed);
  atomic_store_explicit(&rbuf->tail, 0, memory_order_relaxed);
}
