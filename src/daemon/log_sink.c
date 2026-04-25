#include "log_sink.h"

#include <stdio.h>

static const char * LOG_FILE_PATH = "rbuflogd.log";
static FILE *log_file = NULL;

int rbuflogd_write_log(const char * log_msg) {

  if(log_file == NULL) {
    log_file = fopen(LOG_FILE_PATH, "a");
    if (log_file == NULL) {
      return -1; // Failed to open log file
    }
  }

  fprintf(log_file, "%s\n", log_msg);
  fflush(log_file);
  return 0;
}
