#define _POSIX_C_SOURCE 200809L // for time defs, define before any system header
#include "log_line_format.h"

#include "common_types.h"
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define RBUF_STR_HELPER(x) #x
#define RBUF_STR(x) RBUF_STR_HELPER(x)

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

static void trim_spaces(char * s) {
  char * start;
  char * end;

  if (s == NULL || s[0] == '\0') {
    return;
  }

  start = s;
  while (*start != '\0' && isspace((unsigned char) *start)) {
    start++;
  }

  if (start != s) {
    memmove(s, start, strlen(start) + 1U);
  }

  if (s[0] == '\0') {
    return;
  }

  end = s + strlen(s) - 1U;
  while (end >= s && isspace((unsigned char) *end)) {
    *end = '\0';
    end--;
  }
}

static int parse_level(const char * level_str, rbuflogd_log_level_t * level_out) {
  if (level_str == NULL || level_out == NULL) {
    return -1;
  }

  if (strcmp(level_str, "DEBUG") == 0) {
    *level_out = RBUF_LOG_LEVEL_DEBUG;
    return 0;
  }
  if (strcmp(level_str, "INFO") == 0) {
    *level_out = RBUF_LOG_LEVEL_INFO;
    return 0;
  }
  if (strcmp(level_str, "WARNING") == 0) {
    *level_out = RBUF_LOG_LEVEL_WARNING;
    return 0;
  }
  if (strcmp(level_str, "ERROR") == 0) {
    *level_out = RBUF_LOG_LEVEL_ERROR;
    return 0;
  }

  return -1;
}

static int parse_realtime_ns(const char * timestamp, uint64_t * realtime_ns_out) {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int millis;
  int parsed;
  struct tm tm_buf;
  time_t sec;

  if (timestamp == NULL || realtime_ns_out == NULL) {
    return -1;
  }

  parsed = sscanf(
    timestamp,
    "%4d.%2d.%2d %2d:%2d:%2d.%3d",
    &year,
    &month,
    &day,
    &hour,
    &minute,
    &second,
    &millis);
  if (parsed != 7) {
    return -1;
  }

  memset(&tm_buf, 0, sizeof(tm_buf));
  tm_buf.tm_year = year - 1900;
  tm_buf.tm_mon = month - 1;
  tm_buf.tm_mday = day;
  tm_buf.tm_hour = hour;
  tm_buf.tm_min = minute;
  tm_buf.tm_sec = second;
  tm_buf.tm_isdst = -1;

  sec = mktime(&tm_buf);
  if (sec == (time_t) -1) {
    return -1;
  }

  *realtime_ns_out = ((uint64_t) sec * 1000000000ULL) + ((uint64_t) millis * 1000000ULL);
  return 0;
}

static void copy_string_field(char * dst, size_t dst_sz, const char * src) {
  size_t src_len;
  size_t copy_len;

  if (dst == NULL || dst_sz == 0) {
    return;
  }

  if (src == NULL) {
    dst[0] = '\0';
    return;
  }

  src_len = strlen(src);
  copy_len = (src_len < (dst_sz - 1U)) ? src_len : (dst_sz - 1U);
  memcpy(dst, src, copy_len);
  dst[copy_len] = '\0';
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


int format_log_entry_to_log_line(const rbuf_entry_t * const entry, const char * const boot_id, char * out_msg, size_t out_msg_sz) {
  if (entry == NULL || boot_id == NULL || out_msg == NULL || out_msg_sz == 0) {
    return -1;
  }
  uint64_t mono_ms;
  char time_buf[RBUF_TIMESTAMP_STR_MAX_CHARS + 1];

  format_realtime_ns(entry->realtime_ns, time_buf, sizeof(time_buf));
  mono_ms = entry->monotonic_ns / 1000000ULL;

  snprintf(
    out_msg,
    out_msg_sz,
    LINE_FORMAT, 
    time_buf,
    (unsigned long long) mono_ms,
    boot_id,
    level_to_string(entry->level),
    RBUF_PRODUCER_ID_DISPLAY_CHARS,
    RBUF_PRODUCER_ID_DISPLAY_CHARS,
    entry->producer_name,
    RBUF_CATEGORY_DISPLAY_CHARS,
    RBUF_CATEGORY_DISPLAY_CHARS,
    entry->category,
    entry->msg);

  return 0;
}


int format_log_line_to_log_entry(const char * const log_line, rbuf_entry_t * const entry) {
  char timestamp_buf[RBUF_TIMESTAMP_STR_MAX_CHARS + 1];
  unsigned long long mono_ms;
  char boot_id_buf[RBUF_BOOT_ID_MAX_CHARS + 1];
  char level_buf[RBUF_LEVEL_MAX_CHARS + 1];
  char producer_buf[RBUF_PRODUCER_ID_DISPLAY_CHARS + 1];
  char category_buf[RBUF_CATEGORY_DISPLAY_CHARS + 1];
  char message_buf[RBUF_MSG_MAX_LEN];
  int parsed;
  int msg_offset;

  if (log_line == NULL || entry == NULL) {
    return -1;
  }

  msg_offset = 0;
  parsed = sscanf(
    log_line,
    "%23[0-9. :] [%20llu ms] [%8[^]]] [%8[^]]] [%8[^]]] [%8[^]]] %n",
    timestamp_buf,
    &mono_ms,
    boot_id_buf,
    level_buf,
    producer_buf,
    category_buf,
    &msg_offset);

  if (parsed != 6 || msg_offset <= 0) {
    return -1;
  }

  copy_string_field(message_buf, sizeof(message_buf), log_line + (size_t) msg_offset);

  trim_spaces(level_buf);
  trim_spaces(producer_buf);
  trim_spaces(category_buf);

  if (parse_realtime_ns(timestamp_buf, &entry->realtime_ns) != 0) {
    return -1;
  }

  entry->monotonic_ns = (uint64_t) mono_ms * 1000000ULL;

  if (parse_level(level_buf, &entry->level) != 0) {
    return -1;
  }

  copy_string_field(entry->producer_name, sizeof(entry->producer_name), producer_buf);
  copy_string_field(entry->category, sizeof(entry->category), category_buf);
  copy_string_field(entry->msg, sizeof(entry->msg), message_buf);

  (void) boot_id_buf;
  return 0;
}