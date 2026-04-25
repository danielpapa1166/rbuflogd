#ifndef RBUFLOGD_CONSUMER_H
#define RBUFLOGD_CONSUMER_H

#include "rbuflogd_common_types.h"

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg);
int rbuflogd_write_log(const char * log_msg); 

#endif /* RBUFLOGD_CONSUMER_H */