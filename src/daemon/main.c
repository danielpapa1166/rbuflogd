#define _DEFAULT_SOURCE

#include "common_types.h"
#include "log_line_format.h"
#include "consumer.h"
#include "log_sink.h"
#include "shmem.h"
#include "cli_parser.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
// note: compile with: 
// rm -rf build && cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build build

#define BOOT_ID_PATH "/proc/sys/kernel/random/boot_id"
static int read_boot_id(char * out_boot_id, size_t out_boot_id_sz); 
void handle_signal(int signal); 

// handle user terminate signal to cleanup shared memory
static volatile sig_atomic_t terminate_app = 0;


int main(int argc, char * argv[]) {

  rbuf_entry_t entry; 
  format_log_line_to_log_entry(TEST_LINE, &entry); 
  printf("Parsed log entry:\n");
  printf("  realtime_ns: %lu\n", entry.realtime_ns);
  printf("  monotonic_ns: %lu\n", entry.monotonic_ns);
  printf("  producer_name: '%s'\n", entry.producer_name); 
  printf("  level: %d\n", entry.level);

  rbuflogd_cli_config_t cli_config;
  const int cli_res = parse_cli_args(argc, argv, &cli_config);
  if (cli_res != 0) {
    return -1;
  }


  char boot_id[RBUF_BOOT_ID_MAX_CHARS + 1] = {0};

  // Register signal handler for SIGINT and SIGTERM
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  if (read_boot_id(boot_id, sizeof(boot_id)) != 0) {
    printf("Warning: failed to read boot id, using fallback\n");
    rbuflogd_consumer_set_boot_id(NULL);
  }
  else {
    rbuflogd_consumer_set_boot_id(boot_id);
  }

  rbuflogd_set_minimum_log_level(cli_config.minimum_log_level);

  if (rbuflogd_log_sink_init(&cli_config) != 0) {
    printf("Failed to initialize log sink\n");
    return -1;
  }

  rbuf_t * rbuf = NULL;
  char log_msg[RBUF_FORMATTED_LOG_MAX_LEN];
  // init shared memory:
  int res = rbuflogd_init_shmem(&rbuf);
  if (res != 0) {
    printf("Failed to initialize shared memory\n");
    return -1; // Failed to initialize shared memory   
  }

  if (cli_config.log_init_success) {
    rbuf_entry_t entry = {
      .level = RBUF_LOG_LEVEL_INFO, 
      .producer_name = "rbuflogd", 
      .category = "Init", 
      .msg = "rbuflogd initialized successfully"
    };

    if (rbuflogd_format_internal_log(
        &entry, 
        log_msg,
        sizeof(log_msg)) == 0) {
      rbuflogd_write_log(log_msg);
    }
  }
  while (!terminate_app) {
    const int consume_res = rbuflogd_consume(rbuf, log_msg, sizeof(log_msg));
    if (consume_res == 0) {
      rbuflogd_write_log(log_msg);
    }
  }

  rbuflogd_cleanup_shmem();
  rbuflogd_log_sink_cleanup();

  return 0;
}



static int read_boot_id(char * out_boot_id, size_t out_boot_id_sz) {
  FILE * f = NULL;
  char temp_buf[64] = {0};
  size_t len;

  if (out_boot_id == NULL || out_boot_id_sz == 0) {
    return -1;
  }

  f = fopen(BOOT_ID_PATH, "r");
  if (f == NULL) {
    return -1;
  }

  if (fgets(temp_buf, sizeof(temp_buf), f) == NULL) {
    fclose(f);
    return -1;
  }

  fclose(f);
  len = strlen(temp_buf);
  if (len > 0 && temp_buf[len - 1] == '\n') {
    temp_buf[len - 1] = '\0';
  }

  // Only take the first out_boot_id_sz-1 characters
  snprintf(out_boot_id, out_boot_id_sz, "%.*s", (int)(out_boot_id_sz - 1), temp_buf);

  return 0;
}

void handle_signal(int signal) {
  (void) signal; // unused parameter
  terminate_app = 1;
}