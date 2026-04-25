#ifndef RBUFLOGD_CONSUMER_H
#define RBUFLOGD_CONSUMER_H

#include "rbuflogd/common_types.h"

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg);

#endif /* RBUFLOGD_CONSUMER_H */