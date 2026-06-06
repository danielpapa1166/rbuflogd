#include "rbuflogd/pub_common_types.h"
#define _POSIX_C_SOURCE 200809L

#include "consumer.h"
#include "common_types.h"
#include "ring_buffer.h"
#include "log_line_format.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define RBUF_CONSUME_WAIT_TIMEOUT_MS 100

#define RBUF_BOOT_ID_FALLBACK " unknown"

static char boot_id_cache[RBUF_BOOT_ID_MAX_CHARS + 1] = RBUF_BOOT_ID_FALLBACK;
static rbuflogd_log_level_t minimum_log_level = DEFAULT_MINIMUM_LOG_LEVEL;


void rbuflogd_consumer_set_boot_id(const char * boot_id) {
  if (boot_id == NULL || boot_id[0] == '\0') {
    snprintf(boot_id_cache, sizeof(boot_id_cache), "%s", RBUF_BOOT_ID_FALLBACK);
    return;
  }

  snprintf(boot_id_cache, sizeof(boot_id_cache), "%s", boot_id);
}

void rbuflogd_set_minimum_log_level(rbuflogd_log_level_t level) {
  minimum_log_level = level;
}


int rbuflogd_format_internal_log(rbuf_entry_t * const entry,    
    char * const out_msg, size_t out_msg_sz) {

  struct timespec realtime_ts;
  struct timespec monotonic_ts;

  if (clock_gettime(CLOCK_REALTIME, &realtime_ts) != 0) {
    return -1;
  }

  if (clock_gettime(CLOCK_MONOTONIC, &monotonic_ts) != 0) {
    return -1;
  }

  entry->realtime_ns = ((uint64_t) realtime_ts.tv_sec * 1000000000ULL) + (uint64_t) realtime_ts.tv_nsec;
  entry->monotonic_ns = ((uint64_t) monotonic_ts.tv_sec * 1000000000ULL) + (uint64_t) monotonic_ts.tv_nsec;

  return format_log_line(entry, boot_id_cache, out_msg, out_msg_sz);
}

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg, size_t out_msg_sz) {
  rbuf_entry_t entry;

  if (rbuf == NULL || out_msg == NULL || out_msg_sz == 0) {
    return -1;
  }

  if (rbuf_try_pop(rbuf, &entry) != 0) {
    if (rbuf_wait_for_data(rbuf, RBUF_CONSUME_WAIT_TIMEOUT_MS) != 0) {
      return -1;
    }

    if (rbuf_try_pop(rbuf, &entry) != 0) {
      return -1;
    }
  }

  if (entry.level < minimum_log_level) {
    return -1;
  }

  return format_log_line(&entry, boot_id_cache, out_msg, out_msg_sz);
}
