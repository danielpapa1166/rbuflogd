#ifndef RBUFLOGD_CLI_PARSER_H
#define RBUFLOGD_CLI_PARSER_H

#include "rbuflogd/pub_common_types.h"
#include <stddef.h>

#define DEFAULT_LOG_FILE_PATH           "./"
#define DEFAULT_LOG_FILE_NAME           "rbuflogd_%03u.log"
#define DEFAULT_LOG_FILE_SIZE           (1024 * 1024) // 1 MiB
#define DEFAULT_RING_BUFFER_SIZE        1000
#define DEFAULT_MINIMUM_LOG_LEVEL       ((rbuflogd_log_level_t)(RBUF_LOG_LEVEL_INFO))

typedef struct {
    char log_file_path[256];
    char log_file_name[256];
    size_t log_file_size;
    size_t ring_buffer_size;
    rbuflogd_log_level_t minimum_log_level;
} rbuflogd_cli_config_t;


int parse_cli_args(int argc, char * argv[], 
    rbuflogd_cli_config_t * const cli_config_out); 

#endif 