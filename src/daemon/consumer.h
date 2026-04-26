#ifndef RBUFLOGD_CONSUMER_H
#define RBUFLOGD_CONSUMER_H

#include <stddef.h>
#include "common_types.h"

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg, size_t out_msg_sz);

#endif /* RBUFLOGD_CONSUMER_H */