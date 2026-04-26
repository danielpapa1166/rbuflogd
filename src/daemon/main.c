#define _DEFAULT_SOURCE

#include "common_types.h"
#include "consumer.h"
#include "log_sink.h"
#include "shmem.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define BOOT_ID_PATH "/proc/sys/kernel/random/boot_id"

static int read_boot_id(char * out_boot_id, size_t out_boot_id_sz) {
  FILE * f = NULL;
  size_t len;

  if (out_boot_id == NULL || out_boot_id_sz == 0) {
    return -1;
  }

  f = fopen(BOOT_ID_PATH, "r");
  if (f == NULL) {
    return -1;
  }

  if (fgets(out_boot_id, (int) out_boot_id_sz, f) == NULL) {
    fclose(f);
    return -1;
  }

  fclose(f);
  len = strlen(out_boot_id);
  if (len > 0 && out_boot_id[len - 1] == '\n') {
    out_boot_id[len - 1] = '\0';
  }

  return 0;
}

// note: compile with: 
// rm -rf build && cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build build

// handle user terminate signal to cleanup shared memory
static int terminate_app = 0;

void handle_signal(int signal) {
  (void) signal; // unused parameter
  terminate_app = 1;
}

int main(void) {
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

  if (rbuflogd_log_sink_init() != 0) {
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


  while(!terminate_app) {
    const int res = rbuflogd_consume(rbuf, log_msg, sizeof(log_msg)); 
    if (res == 0) {
      printf("%s\n", log_msg);
      rbuflogd_write_log(log_msg);
    }
    usleep(1); 
  }

  rbuflogd_cleanup_shmem();
  printf("Consumer has terminated.\n");

  return 0;
}