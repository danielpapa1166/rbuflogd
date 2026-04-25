#include "rbuflogd_common_types.h"
#include "rbuflogd_consumer.h"
#include "rbuflogd_shmem.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// handle user terminate signal to cleanup shared memory
static int terminate_app = 0;

void handle_signal(int signal) {
  (void) signal; // unused parameter
  terminate_app = 1;
}

int main() {

  // Register signal handler for SIGINT and SIGTERM
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  rbuf_t * rbuf = NULL;
  char log_msg[RBUF_MSG_MAX_LEN];
  // init shared memory:
  int res = rbuflogd_init_shmem(&rbuf);
  if (res != 0) {
    printf("Failed to initialize shared memory\n");
    return -1; // Failed to initialize shared memory   
  }


  while(!terminate_app) {
    sleep(1); 
    printf("Consumer is running...\n");

    const int res = rbuflogd_consume(rbuf, log_msg); 
    if (res == 0) {
      printf("Consumed log: %s\n", log_msg);
      rbuflogd_write_log(log_msg);
    }
    
  }

  rbuflogd_cleanup_shmem();
  printf("Consumer has terminated.\n");

  return 0;
}