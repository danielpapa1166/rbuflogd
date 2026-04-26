#ifndef RBUFLOGD_COMMON_TYPES_H
#define RBUFLOGD_COMMON_TYPES_H

#include <stdatomic.h>
#include <stdint.h>
#include "rbuflogd/pub_common_types.h"

#define RBUF_SIZE               (1024) 
#define RBUF_MSG_MAX_LEN        256
#define RBUF_LOG_CATEGORY_LEN   8
#define SHMEM_NAME              "/rbuflogd_shmem"

typedef struct {
  uint64_t                realtime_ns;
  uint64_t                monotonic_ns;
  char                    producer_name[RBUF_PROD_ID_MAX_LEN];
  rbuflogd_log_level_t    level;
  char                    category[RBUF_LOG_CATEGORY_LEN];
  char                    msg[RBUF_MSG_MAX_LEN];
} rbuf_entry_t;

typedef struct {
  rbuf_entry_t data[RBUF_SIZE];
  atomic_size_t head;
  atomic_size_t tail;
} rbuf_t;


#endif /* RBUFLOGD_COMMON_TYPES_H */

