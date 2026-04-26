#include "log_sink.h"

#include <stdio.h>
#include <string.h>

#define LOG_FILE_MAX_BYTES (1024U * 1024U)
#define LOG_FILE_INDEX_MAX 1000U

static FILE *log_file = NULL;
static unsigned current_log_index = 0;
static size_t current_log_size = 0;

static int open_log_file(unsigned index) {
  char path[32];

  if (snprintf(path, sizeof(path), "rbuflogd_%03u.log", index) < 0) {
    return -1;
  }

  if (log_file != NULL) {
    fclose(log_file);
    log_file = NULL;
  }

  log_file = fopen(path, "w");
  if (log_file == NULL) {
    return -1;
  }

  current_log_size = 0;
  return 0;
}

static int rotate_log_file(void) {
  current_log_index = (current_log_index + 1U) % LOG_FILE_INDEX_MAX;
  return open_log_file(current_log_index);
}

int rbuflogd_write_log(const char * log_msg) {
  size_t line_bytes;

  if (log_msg == NULL) {
    return -1;
  }

  if (log_file == NULL && open_log_file(current_log_index) != 0) {
    return -1;
  }

  line_bytes = strlen(log_msg) + 1U; // message + newline

  if (current_log_size > 0 && (current_log_size + line_bytes) > LOG_FILE_MAX_BYTES) {
    if (rotate_log_file() != 0) {
      return -1;
    }
  }

  if (fprintf(log_file, "%s\n", log_msg) < 0) {
    return -1;
  }

  if (fflush(log_file) != 0) {
    return -1;
  }

  current_log_size += line_bytes;
  return 0;
}
