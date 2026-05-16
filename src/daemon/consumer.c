#include "rbuflogd/pub_common_types.h"
#define _POSIX_C_SOURCE 200809L

#include "consumer.h"
#include "common_types.h"
#include "ring_buffer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define RBUF_CONSUME_WAIT_TIMEOUT_MS 100

#define RBUF_BOOT_ID_FALLBACK " unknown"

static char boot_id_cache[RBUF_BOOT_ID_MAX_CHARS + 1] = RBUF_BOOT_ID_FALLBACK;
static rbuflogd_log_level_t minimum_log_level = DEFAULT_MINIMUM_LOG_LEVEL;

static const char * level_to_string(rbuflogd_log_level_t level) {
  switch (level) {
    case RBUF_LOG_LEVEL_DEBUG:
      return "   DEBUG";
    case RBUF_LOG_LEVEL_INFO:
      return "    INFO";
    case RBUF_LOG_LEVEL_WARNING:
      return " WARNING";
    case RBUF_LOG_LEVEL_ERROR:
      return "   ERROR";
    default:
      return " UNKNOWN";
  }
}

static void format_realtime_ns(uint64_t realtime_ns, char * out, size_t out_sz) {
  time_t sec = (time_t) (realtime_ns / 1000000000ULL);
  long ms = (long) ((realtime_ns % 1000000000ULL) / 1000000ULL);
  struct tm tm_buf;

  if (out == NULL || out_sz == 0) {
    return;
  }

  localtime_r(&sec, &tm_buf);
  strftime(out, out_sz, "%Y.%m.%d %H:%M:%S", &tm_buf);
  snprintf(out + strlen(out), out_sz - strlen(out), ".%03ld", ms);
}

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

static int format_log_line(
  uint64_t realtime_ns,
  uint64_t monotonic_ns,
  rbuflogd_log_level_t level,
  const char * producer_name,
  const char * category,
  const char * msg,
  char * out_msg,
  size_t out_msg_sz) {
  uint64_t mono_ms;
  char time_buf[RBUF_TIMESTAMP_STR_MAX_CHARS + 1];

  if (producer_name == NULL || category == NULL || msg == NULL || out_msg == NULL || out_msg_sz == 0) {
    return -1;
  }

  if (level < minimum_log_level) {
    return -1;
  }

  format_realtime_ns(realtime_ns, time_buf, sizeof(time_buf));
  mono_ms = monotonic_ns / 1000000ULL;

  snprintf(
    out_msg,
    out_msg_sz,
    "%s [%8llu ms] [%s] [%s] [%*.*s] [%*.*s] %s",
    time_buf,
    (unsigned long long) mono_ms,
    boot_id_cache,
    level_to_string(level),
    RBUF_PRODUCER_ID_DISPLAY_CHARS,
    RBUF_PRODUCER_ID_DISPLAY_CHARS,
    producer_name,
    RBUF_CATEGORY_DISPLAY_CHARS,
    RBUF_CATEGORY_DISPLAY_CHARS,
    category,
    msg);

  return 0;
}

int rbuflogd_format_internal_log(
  rbuflogd_log_level_t level,
  const char * producer_name,
  const char * category,
  const char * msg,
  char * out_msg,
  size_t out_msg_sz) {
  struct timespec realtime_ts;
  struct timespec monotonic_ts;
  uint64_t realtime_ns;
  uint64_t monotonic_ns;

  if (clock_gettime(CLOCK_REALTIME, &realtime_ts) != 0) {
    return -1;
  }

  if (clock_gettime(CLOCK_MONOTONIC, &monotonic_ts) != 0) {
    return -1;
  }

  realtime_ns = ((uint64_t) realtime_ts.tv_sec * 1000000000ULL) + (uint64_t) realtime_ts.tv_nsec;
  monotonic_ns = ((uint64_t) monotonic_ts.tv_sec * 1000000000ULL) + (uint64_t) monotonic_ts.tv_nsec;

  return format_log_line(
    realtime_ns,
    monotonic_ns,
    level,
    producer_name,
    category,
    msg,
    out_msg,
    out_msg_sz);
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

  return format_log_line(
    entry.realtime_ns,
    entry.monotonic_ns,
    entry.level,
    entry.producer_name,
    entry.category,
    entry.msg,
    out_msg,
    out_msg_sz);
}
