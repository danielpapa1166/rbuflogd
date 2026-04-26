#include "consumer.h"
#include "common_types.h"
#include "ring_buffer.h"
#include <stdio.h>
#include <string.h>

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg) {

  rbuf_entry_t entry;

  if (rbuf_try_pop(rbuf, &entry) != 0) {
    // Buffer is empty
    return -1;
  }

  // assembly log message:
  memcpy(out_msg, entry.msg, RBUF_MSG_MAX_LEN);

  printf("Consumed log entry from producer \"%s\" with category \"%s\": \"%s\"\n", 
    entry.producer_name, entry.category, out_msg);

  printf("Read from buffer: \"%s\"\n", out_msg);

  return 0;
}
