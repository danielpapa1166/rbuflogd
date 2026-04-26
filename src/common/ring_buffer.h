#ifndef RBUFLOGD_RING_BUFFER_H
#define RBUFLOGD_RING_BUFFER_H

#include "common_types.h"

int rbuf_is_empty(const rbuf_t * rbuf);
int rbuf_is_full(const rbuf_t * rbuf);
int rbuf_try_push(rbuf_t * rbuf, const rbuf_entry_t * entry);
int rbuf_try_pop(rbuf_t * rbuf, rbuf_entry_t * out_entry);
void rbuf_reset(rbuf_t * rbuf);

#endif /* RBUFLOGD_RING_BUFFER_H */
