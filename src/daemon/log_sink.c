#include "log_sink.h"
#include "cli_config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>

#define LOG_FILE_PATH_MAX_LEN     520U

static FILE *log_file = NULL;
static unsigned current_log_index = 0;
static size_t current_log_size = 0;
static int log_sink_initialized = 0;
static size_t configured_log_file_max_bytes = DEFAULT_LOG_FILE_SIZE;
static unsigned configured_log_file_index_max = DEFAULT_RING_BUFFER_SIZE;
static char configured_log_file_path[sizeof(((rbuflogd_cli_config_t *)0)->log_file_path)] = DEFAULT_LOG_FILE_PATH;
static char configured_log_file_name[sizeof(((rbuflogd_cli_config_t *)0)->log_file_name)] = DEFAULT_LOG_FILE_NAME;

static int build_log_file_path(char * out_path, size_t out_path_sz, unsigned index) {
  char file_name[sizeof(configured_log_file_name)];
  size_t path_len;
  int file_name_res;

  if (out_path == NULL || out_path_sz == 0) {
    return -1;
  }

  file_name_res = snprintf(file_name, sizeof(file_name), configured_log_file_name, index);
  if (file_name_res < 0 || (size_t) file_name_res >= sizeof(file_name)) {
    return -1;
  }

  path_len = strlen(configured_log_file_path);
  if (path_len > 0 && configured_log_file_path[path_len - 1] == '/') {
    if (snprintf(out_path, out_path_sz, "%s%s", configured_log_file_path, file_name) < 0) {
      return -1;
    }
    return 0;
  }

  if (snprintf(out_path, out_path_sz, "%s/%s", configured_log_file_path, file_name) < 0) {
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

  for (unsigned i = 0; i < configured_log_file_index_max; i++) {
    char path[LOG_FILE_PATH_MAX_LEN];
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

  if ((size_t) highest_stat.st_size < configured_log_file_max_bytes) {
    *out_index = highest_index;
    *out_size = (size_t) highest_stat.st_size;
    *out_mode = "a";

    return 0;
  }

  *out_index = (highest_index + 1U) % configured_log_file_index_max;
  *out_size = 0;
  *out_mode = "w";
  return 0;
}

static int open_log_file(unsigned index, const char * mode, size_t initial_size) {
  char path[LOG_FILE_PATH_MAX_LEN];

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
  current_log_index = (current_log_index + 1U) % configured_log_file_index_max;
  return open_log_file(current_log_index, "w", 0);
}

int rbuflogd_log_sink_init(const rbuflogd_cli_config_t * const cli_config) {
  unsigned index;
  size_t size;
  const char * mode;

  int res; 

  if (cli_config == NULL) {
    return -1;
  }

  if (cli_config->log_file_size == 0 || cli_config->ring_buffer_size == 0 ||
      cli_config->log_file_path[0] == '\0' || cli_config->log_file_name[0] == '\0') {
    return -1;
  }

  if (cli_config->ring_buffer_size > (size_t) UINT_MAX) {
    return -1;
  }

  configured_log_file_max_bytes = cli_config->log_file_size;
  configured_log_file_index_max = (unsigned) cli_config->ring_buffer_size;

  strncpy(configured_log_file_path, cli_config->log_file_path, sizeof(configured_log_file_path) - 1);
  configured_log_file_path[sizeof(configured_log_file_path) - 1] = '\0';

  strncpy(configured_log_file_name, cli_config->log_file_name, sizeof(configured_log_file_name) - 1);
  configured_log_file_name[sizeof(configured_log_file_name) - 1] = '\0';

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
    return -1;
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

  if (current_log_size > 0 && (current_log_size + line_bytes) > configured_log_file_max_bytes) {
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

int rbuflogd_log_sink_cleanup(void) {
  if (log_file != NULL) {
    fclose(log_file);
    log_file = NULL;
  }

  log_sink_initialized = 0;
  return 0;
}
