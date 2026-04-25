#ifndef RBUFLOGD_COMMON_TYPES_H
#define RBUFLOGD_COMMON_TYPES_H

#include <stdatomic.h>

#define RBUF_SIZE               (1000) 
#define RBUF_MSG_MAX_LEN        256
#define SHMEM_NAME              "/rbuflogd_shmem"

typedef struct {
  char data[RBUF_SIZE][RBUF_MSG_MAX_LEN];
  atomic_size_t head;
  atomic_size_t tail;
} rbuf_t;


#endif /* RBUFLOGD_COMMON_TYPES_H */

