#include "log_sink.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#define LOG_FILE_MAX_BYTES        (1024U * 1024U)
#define LOG_FILE_INDEX_MAX        1000U
#define LOG_FILE_NAME_FORMAT      "rbuflogd_%03u.log"

static FILE *log_file = NULL;
static unsigned current_log_index = 0;
static size_t current_log_size = 0;
static int log_sink_initialized = 0;

static int build_log_file_path(char * out_path, size_t out_path_sz, unsigned index) {
  if (out_path == NULL || out_path_sz == 0) {
    return -1;
  }
  const int res = snprintf(
    out_path, 
    out_path_sz, 
    LOG_FILE_NAME_FORMAT, 
    index); 

  if (res < 0) {
    return -1;
  }

  return 0;
}

static int find_log_start(unsigned * out_index, size_t * out_size, const char ** out_mode) {
  int found_any = 0;
  unsigned highest_index = 0;
  struct stat highest_stat;
  memset(&highest_stat, 0, sizeof(highest_stat));

  if (out_index == NULL || out_size == NULL || out_mode == NULL) {
    return -1;
  }

  for (unsigned i = 0; i < LOG_FILE_INDEX_MAX; i++) {
    char path[32];
    struct stat st;
    const int path_build_res = build_log_file_path(
      path, sizeof(path), i); 

    if (path_build_res != 0) {
      return -1;
    }

    const int stat_res = stat(path, &st);
    if (stat_res != 0) {
      continue;
    }

    if (!found_any || i > highest_index) {
      highest_index = i;
      highest_stat = st;
      found_any = 1;
    }
  }

  if (!found_any) {
    *out_index = 0;
    *out_size = 0;
    *out_mode = "w";
    return 0;
  }

  if ((size_t) highest_stat.st_size < LOG_FILE_MAX_BYTES) {
    *out_index = highest_index;
    *out_size = (size_t) highest_stat.st_size;
    *out_mode = "a";

    return 0;
  }

  *out_index = (highest_index + 1U) % LOG_FILE_INDEX_MAX;
  *out_size = 0;
  *out_mode = "w";
  return 0;
}

static int open_log_file(unsigned index, const char * mode, size_t initial_size) {
  char path[32];

  const int res = build_log_file_path(
    path, sizeof(path), index);
  if (res != 0) {
    return -1;
  }

  if (mode == NULL) {
    return -1;
  }

  if (log_file != NULL) {
    fclose(log_file);
    log_file = NULL;
  }

  log_file = fopen(path, mode);
  if (log_file == NULL) {
    return -1;
  }

  current_log_index = index;
  current_log_size = initial_size;
  return 0;
}

static int rotate_log_file(void) {
  current_log_index = (current_log_index + 1U) % LOG_FILE_INDEX_MAX;
  return open_log_file(current_log_index, "w", 0);
}

int rbuflogd_log_sink_init(void) {
  unsigned index;
  size_t size;
  const char * mode;

  int res; 

  res = find_log_start(&index, &size, &mode); 
  if (res != 0) {
    return -1;
  }

  res = open_log_file(index, mode, size); 
  if (res != 0) {
    return -1;
  }

  log_sink_initialized = 1;
  return 0;
}

int rbuflogd_write_log(const char * log_msg) {
  size_t line_bytes;
  int res; 

  if (log_msg == NULL) {
    return -1;
  }

  if (!log_sink_initialized) {
    res = rbuflogd_log_sink_init();
    if(res != 0) {
      return -1;
    }
  }

  if (log_file == NULL) {
    res = open_log_file(
      current_log_index,
      "a", 
      current_log_size);
    if(res != 0) {
      return -1;
    }
  }

  line_bytes = strlen(log_msg) + 1U; // message + newline

  if (current_log_size > 0 && (current_log_size + line_bytes) > LOG_FILE_MAX_BYTES) {
    res = rotate_log_file();
    if(res != 0) {
      return -1;
    }
  }

  res = fprintf(log_file, "%s\n", log_msg);
  if (res < 0) {
    return -1;
  }

  res = fflush(log_file);
  if (res != 0) {
    return -1;
  }

  current_log_size += line_bytes;
  return 0;
}
