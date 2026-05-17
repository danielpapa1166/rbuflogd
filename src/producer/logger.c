#include "rbuflogd/logger.h"
#include <stdlib.h>

static rbuflogd_producer_t * s_producer = NULL;

int rbuflogd_logger_init(const char * producer_name) {
  if (s_producer != NULL) {
    return -1;
  }

  s_producer = calloc(1, sizeof(*s_producer));
  if (s_producer == NULL) {
    return -1;
  }

  if (rbuflogd_producer_open(s_producer, producer_name) != 0) {
    free(s_producer);
    s_producer = NULL;
    return -1;
  }

  return 0;
}

void rbuflogd_logger_close(void) {
  if (s_producer == NULL) {
    return;
  }

  rbuflogd_producer_close(s_producer);
  free(s_producer);
  s_producer = NULL;
}

static int log_if_ready(rbuflogd_log_level_t level, const char * category, const char * message) {
  if (s_producer == NULL || s_producer->state == NULL) {
    return -1;
  }
  return rbuflogd_producer_log(s_producer, level, category, message);
}

int rbuflogd_logger_debug(const char * category, const char * message) {
  return log_if_ready(RBUF_LOG_LEVEL_DEBUG, category, message);
}

int rbuflogd_logger_info(const char * category, const char * message) {
  return log_if_ready(RBUF_LOG_LEVEL_INFO, category, message);
}

int rbuflogd_logger_warning(const char * category, const char * message) {
  return log_if_ready(RBUF_LOG_LEVEL_WARNING, category, message);
}

int rbuflogd_logger_error(const char * category, const char * message) {
  return log_if_ready(RBUF_LOG_LEVEL_ERROR, category, message);
}
