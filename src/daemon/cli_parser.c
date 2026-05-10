#include "cli_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void print_usage(const char * program_name) {
  printf("Usage: %s [OPTIONS]\n", program_name);
  printf("Options:\n");
  printf("  -p, --log-path <path>        Log file directory (default: %s)\n", DEFAULT_LOG_FILE_PATH);
  printf("  -n, --log-name <name>        Log file name pattern (default: %s)\n", DEFAULT_LOG_FILE_NAME);
  printf("  -s, --log-size <bytes>       Max log file size in bytes (default: %d)\n", DEFAULT_LOG_FILE_SIZE);
  printf("  -b, --buffer-size <size>     Ring buffer size (default: %d)\n", DEFAULT_RING_BUFFER_SIZE);
  printf("  -l, --log-level <level>      Min log level (0=DEBUG, 1=INFO, 2=WARN, 3=ERROR) (default: %d)\n", DEFAULT_MINIMUM_LOG_LEVEL);
  printf("      --log-init-success       Write a daemon self-log on successful init (default: off)\n");
  printf("  -h, --help                   Show this help message\n");
}

static int parse_log_level(const char * level_str, rbuflogd_log_level_t * out_level) {
  if (level_str == NULL || out_level == NULL) {
    return -1;
  }

  char * endptr;
  long level = strtol(level_str, &endptr, 10);

  if (*endptr != '\0' || level < 0 || level > 3) {
    return -1;
  }

  *out_level = (rbuflogd_log_level_t)level;
  return 0;
}

static int parse_size_arg(const char * size_str, size_t * out_size) {
  if (size_str == NULL || out_size == NULL) {
    return -1;
  }

  char * endptr;
  long size = strtol(size_str, &endptr, 10);

  if (*endptr != '\0' || size <= 0) {
    return -1;
  }

  *out_size = (size_t)size;
  return 0;
}

int parse_cli_args(int argc, char * argv[], 
    rbuflogd_cli_config_t * const cli_config_out) {
  if (cli_config_out == NULL) {
    return -1;
  }

  // Set default values
  strncpy(
    cli_config_out->log_file_path, 
    DEFAULT_LOG_FILE_PATH, 
    sizeof(cli_config_out->log_file_path) - 1);
  cli_config_out->log_file_path[sizeof(cli_config_out->log_file_path) - 1] = '\0';
    
  strncpy(
    cli_config_out->log_file_name, 
    DEFAULT_LOG_FILE_NAME, 
    sizeof(cli_config_out->log_file_name) - 1);
  cli_config_out->log_file_name[sizeof(cli_config_out->log_file_name) - 1] = '\0';

  cli_config_out->log_file_size = DEFAULT_LOG_FILE_SIZE;
  cli_config_out->ring_buffer_size = DEFAULT_RING_BUFFER_SIZE;
  cli_config_out->minimum_log_level = DEFAULT_MINIMUM_LOG_LEVEL;
  cli_config_out->log_init_success = DEFAULT_LOG_INIT_SUCCESS;

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    const char * arg = argv[i];

    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      print_usage(argv[0]);
      return 1; // Signal to caller that help was printed
    }
    else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--log-path") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "Error: %s requires an argument\n", arg);
        return -1;
      }
      strncpy(cli_config_out->log_file_path, argv[++i], sizeof(cli_config_out->log_file_path) - 1);
      cli_config_out->log_file_path[sizeof(cli_config_out->log_file_path) - 1] = '\0';
    }
    else if (strcmp(arg, "-n") == 0 || strcmp(arg, "--log-name") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "Error: %s requires an argument\n", arg);
        return -1;
      }
      strncpy(cli_config_out->log_file_name, argv[++i], sizeof(cli_config_out->log_file_name) - 1);
      cli_config_out->log_file_name[sizeof(cli_config_out->log_file_name) - 1] = '\0';
    }
    else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--log-size") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "Error: %s requires an argument\n", arg);
        return -1;
      }
      if (parse_size_arg(argv[++i], &cli_config_out->log_file_size) != 0) {
        fprintf(stderr, "Error: Invalid log file size\n");
        return -1;
      }
    }
    else if (strcmp(arg, "-b") == 0 || strcmp(arg, "--buffer-size") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "Error: %s requires an argument\n", arg);
        return -1;
      }
      if (parse_size_arg(argv[++i], &cli_config_out->ring_buffer_size) != 0) {
        fprintf(stderr, "Error: Invalid ring buffer size\n");
        return -1;
      }
    }
    else if (strcmp(arg, "-l") == 0 || strcmp(arg, "--log-level") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "Error: %s requires an argument\n", arg);
        return -1;
      }
      if (parse_log_level(argv[++i], &cli_config_out->minimum_log_level) != 0) {
        fprintf(stderr, "Error: Invalid log level (0-3)\n");
        return -1;
      }
    }
    else if (strcmp(arg, "--log-init-success") == 0) {
      cli_config_out->log_init_success = 1;
    }
    else {
      fprintf(stderr, "Error: Unknown option '%s'\n", arg);
      return -1;
    }
  }

  return 0;
}
