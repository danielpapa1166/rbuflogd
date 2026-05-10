#ifndef RBUFLOGD_COMMON_TYPES_H
#define RBUFLOGD_COMMON_TYPES_H

#include <stdatomic.h>
#include <stdint.h>
#include "cli_config.h"
#include "rbuflogd/pub_common_types.h"

#define RBUF_LOG_CATEGORY_LEN             8
#define RBUF_FORMATTED_LOG_MAX_LEN        256

/* Format: "%s [mono_ms=%llu] [boot_id=%s] [%s] [%s] [%s] %s" */
#define RBUF_TIMESTAMP_STR_MAX_CHARS      23          /* YYYY.MM.DD HH:MM:SS.mmm */
#define RBUF_MONO_MS_MAX_CHARS            20          /* max uint64 decimal digits */
#define RBUF_BOOT_ID_MAX_CHARS            8           /* first 8 characters of Linux /proc boot_id UUID, (~4 billion) */
#define RBUF_LEVEL_MAX_CHARS              8           /* " WARNING" / " UNKNOWN" */
#define RBUF_PRODUCER_ID_DISPLAY_CHARS    RBUF_PROD_ID_MAX_LEN
#define RBUF_CATEGORY_DISPLAY_CHARS       RBUF_LOG_CATEGORY_LEN
#define RBUF_FORMAT_FIXED_CHARS           32          /* literals, brackets and spaces */

#define RBUF_FORMATTED_LOG_MAX_CHARS      (RBUF_FORMATTED_LOG_MAX_LEN - 1)

#define RBUF_FORMAT_PREFIX_MAX_CHARS \
  (RBUF_TIMESTAMP_STR_MAX_CHARS + RBUF_MONO_MS_MAX_CHARS + \
  RBUF_BOOT_ID_MAX_CHARS + RBUF_LEVEL_MAX_CHARS + \
  RBUF_PRODUCER_ID_DISPLAY_CHARS + \
   RBUF_CATEGORY_DISPLAY_CHARS + RBUF_FORMAT_FIXED_CHARS)

#define RBUF_MSG_MAX_CHARS                (RBUF_FORMATTED_LOG_MAX_CHARS - RBUF_FORMAT_PREFIX_MAX_CHARS)
#define RBUF_MSG_MAX_LEN                  (RBUF_MSG_MAX_CHARS + 1)

#if RBUF_MSG_MAX_CHARS < 1
#error "RBUF_FORMATTED_LOG_MAX_LEN is too small for log metadata overhead"
#endif

#define SHMEM_NAME                        "/rbuflogd_shmem"

typedef struct {
  uint64_t                realtime_ns;
  uint64_t                monotonic_ns;
  char                    producer_name[RBUF_PROD_ID_MAX_LEN];
  rbuflogd_log_level_t    level;
  char                    category[RBUF_LOG_CATEGORY_LEN];
  char                    msg[RBUF_MSG_MAX_LEN];
} rbuf_entry_t;

typedef struct {
  atomic_size_t seq;
  rbuf_entry_t entry;
} rbuf_slot_t;

typedef struct {
  rbuf_slot_t slots[DEFAULT_RING_BUFFER_SIZE]; // todo make this configurable 
  atomic_size_t head;
  atomic_size_t tail;
} rbuf_t;


#endif /* RBUFLOGD_COMMON_TYPES_H */

