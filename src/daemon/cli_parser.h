#ifndef RBUFLOGD_CLI_PARSER_H
#define RBUFLOGD_CLI_PARSER_H

#include "cli_config.h"

int parse_cli_args(int argc, char * argv[], 
    rbuflogd_cli_config_t * const cli_config_out); 

#endif 