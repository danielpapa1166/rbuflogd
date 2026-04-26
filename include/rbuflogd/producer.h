#ifndef RBUFLOGD_PRODUCER_H
#define RBUFLOGD_PRODUCER_H

typedef struct {
  void * state;
} rbuflogd_producer_t;

int rbuflogd_producer_open(rbuflogd_producer_t * producer);
int rbuflogd_producer_log(rbuflogd_producer_t * producer, const char * log_msg);
void rbuflogd_producer_close(rbuflogd_producer_t * producer);

#endif /* RBUFLOGD_PRODUCER_H */
