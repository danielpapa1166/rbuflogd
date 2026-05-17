#ifndef RBUFLOGD_LOGGER_H
#define RBUFLOGD_LOGGER_H

#include "rbuflogd/producer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the static producer instance with the given name.
 * Must be called once before any log_* macros are used.
 * Returns 0 on success, -1 on failure.
 */
int rbuflogd_logger_init(const char * producer_name);

/**
 * Close and clean up the static producer instance.
 */
void rbuflogd_logger_close(void);

/* Internal helpers — use the macros below instead */
int rbuflogd_logger_debug(const char * category, const char * message);
int rbuflogd_logger_info(const char * category, const char * message);
int rbuflogd_logger_warning(const char * category, const char * message);
int rbuflogd_logger_error(const char * category, const char * message);

#define log_debug(category, message)   rbuflogd_logger_debug((category), (message))
#define log_info(category, message)    rbuflogd_logger_info((category), (message))
#define log_warning(category, message) rbuflogd_logger_warning((category), (message))
#define log_error(category, message)   rbuflogd_logger_error((category), (message))

#ifdef __cplusplus
}
#endif

#endif /* RBUFLOGD_LOGGER_H */
