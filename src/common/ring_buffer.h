#ifndef RBUFLOGD_RING_BUFFER_H
#define RBUFLOGD_RING_BUFFER_H

#include "common_types.h"

int rbuf_is_empty(const rbuf_t * rbuf);
int rbuf_is_full(const rbuf_t * rbuf);
int rbuf_try_push(rbuf_t * rbuf, const char * msg);
int rbuf_try_pop(rbuf_t * rbuf, char * out_msg);
void rbuf_reset(rbuf_t * rbuf);

#endif /* RBUFLOGD_RING_BUFFER_H */
