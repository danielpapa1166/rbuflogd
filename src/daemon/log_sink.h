#ifndef RBUFLOGD_LOG_SINK_H
#define RBUFLOGD_LOG_SINK_H

#include "cli_config.h"

int rbuflogd_log_sink_init(const rbuflogd_cli_config_t * const cli_config);
int rbuflogd_write_log(const char * log_msg);
int rbuflogd_log_sink_cleanup(void);

#endif /* RBUFLOGD_LOG_SINK_H */
