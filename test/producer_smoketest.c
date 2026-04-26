#define _POSIX_C_SOURCE 200809L

#include "rbuflogd/producer.h"
#include <stdio.h>
#include <string.h>

#define TEST_MSG_COUNT 10
#define TEST_MSG_MAX_LEN 256

int main(void) {
  printf("Starting producer test...\n");
  rbuflogd_producer_t producer = {
    .state = NULL,
  };

  if (rbuflogd_producer_open(&producer, "test_producer") == -1) {
    perror("rbuflogd_producer_open");
    return -1;
  }

  for (int i = 0; i < TEST_MSG_COUNT; i++) {
    char log_msg[TEST_MSG_MAX_LEN];
    snprintf(log_msg, sizeof(log_msg), "test message %d", i);

    if (rbuflogd_producer_log(&producer, RBUF_LOG_LEVEL_INFO, "test_category", log_msg) == -1) {
      printf("Buffer full, stopping at message %d\n", i);
      break;
    }

    printf("Wrote: \"%s\"\n", log_msg);
  }

  rbuflogd_producer_close(&producer);
  return 0;
}
