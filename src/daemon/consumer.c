#define _POSIX_C_SOURCE 200809L

#include "consumer.h"
#include "common_types.h"
#include "ring_buffer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define RBUF_BOOT_ID_PLACEHOLDER "dummy-boot-id"

static const char * level_to_string(rbuflogd_log_level_t level) {
  switch (level) {
    case RBUF_LOG_LEVEL_DEBUG:
      return "DEBUG";
    case RBUF_LOG_LEVEL_INFO:
      return "INFO";
    case RBUF_LOG_LEVEL_WARNING:
      return "WARNING";
    case RBUF_LOG_LEVEL_ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
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
  strftime(out, out_sz, "%Y:%m:%d %H:%M:%S", &tm_buf);
  snprintf(out + strlen(out), out_sz - strlen(out), ".%03ld", ms);
}

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg, size_t out_msg_sz) {
  rbuf_entry_t entry;
  const char * boot_id = RBUF_BOOT_ID_PLACEHOLDER;
  uint64_t mono_ms;
  char time_buf[RBUF_TIMESTAMP_STR_MAX_CHARS + 1];

  if (rbuf == NULL || out_msg == NULL || out_msg_sz == 0) {
    return -1;
  }

  if (rbuf_try_pop(rbuf, &entry) != 0) {
    return -1;
  }

  format_realtime_ns(entry.realtime_ns, time_buf, sizeof(time_buf));
  mono_ms = entry.monotonic_ns / 1000000ULL;

  snprintf(
    out_msg,
    out_msg_sz,
    "%s [mono_ms=%llu] [boot_id=%s] [%s] [%*.*s] [%*.*s] %s",
    time_buf,
    (unsigned long long) mono_ms,
    boot_id,
    level_to_string(entry.level),
    RBUF_PRODUCER_ID_DISPLAY_CHARS,
    RBUF_PRODUCER_ID_DISPLAY_CHARS,
    entry.producer_name,
    RBUF_CATEGORY_DISPLAY_CHARS,
    RBUF_CATEGORY_DISPLAY_CHARS,
    entry.category,
    entry.msg);

  return 0;
}
