#include "rbuflogd_consumer.h"
#include <stdatomic.h>
#include <stdio.h>

static const char * LOG_FILE_PATH = "rbuflogd.log";
static FILE *log_file = NULL;

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg) {
  size_t tail = atomic_load_explicit(&rbuf->tail, memory_order_relaxed); 
  size_t head = atomic_load_explicit(&rbuf->head, memory_order_acquire);

  if (tail == head) {
    // Buffer is empty
    return -1; 
  }

  // read and save message: 
  snprintf(out_msg, RBUF_MSG_MAX_LEN, "%s", rbuf->data[tail]);

  printf("Read from buffer: \"%s\" (tail -> %zu)\n", out_msg, tail);

  // update tail
  atomic_store_explicit(
    &rbuf->tail, 
    (tail + 1) % RBUF_SIZE, 
    memory_order_release);

  return 0;
}

int rbuflogd_write_log(const char * log_msg) {

  if(log_file == NULL) {
    log_file = fopen(LOG_FILE_PATH, "a");
    if (log_file == NULL) {
      return -1; // Failed to open log file
    }
  }

  fprintf(log_file, "%s\n", log_msg);
  fflush(log_file);
  return 0;
}
