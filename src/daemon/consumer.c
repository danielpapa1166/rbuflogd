#include "consumer.h"
#include "ring_buffer.h"
#include <stdio.h>

int rbuflogd_consume(rbuf_t * rbuf, char * out_msg) {
  if (rbuf_try_pop(rbuf, out_msg) != 0) {
    // Buffer is empty
    return -1;
  }

  printf("Read from buffer: \"%s\"\n", out_msg);

  return 0;
}
