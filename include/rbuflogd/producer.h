#ifndef RBUFLOGD_PRODUCER_H
#define RBUFLOGD_PRODUCER_H

#include "rbuflogd/pub_common_types.h"


typedef struct {
  char   producer_name[RBUF_PROD_ID_MAX_LEN];
  void * state;
} rbuflogd_producer_t;

int rbuflogd_producer_open(rbuflogd_producer_t * producer, const char * producer_name);
int rbuflogd_producer_log(rbuflogd_producer_t * producer, 
  rbuflogd_log_level_t level, const char * category, const char * log_msg);
void rbuflogd_producer_close(rbuflogd_producer_t * producer);

#endif /* RBUFLOGD_PRODUCER_H */
