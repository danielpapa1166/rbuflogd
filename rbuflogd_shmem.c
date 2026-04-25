#define _POSIX_C_SOURCE 200809L
#include "rbuflogd_shmem.h"
#include "rbuflogd_common_types.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

static int shmem_fd = -1;


int rbuflogd_init_shmem(rbuf_t ** out_rbuf) {
  shmem_fd = shm_open(SHMEM_NAME, O_RDWR | O_CREAT, 0666);
  if (shmem_fd == -1) {
    return -1; // Failed to create shared memory
  }

  // Set size of shared memory: 
  ftruncate(shmem_fd, sizeof(rbuf_t)); 

  // Map shared memory to process address space
  rbuf_t * rbuf = mmap(
    NULL, 
    sizeof(rbuf_t), 
    PROT_READ | PROT_WRITE, 
    MAP_SHARED, 
    shmem_fd, 
    0);

  if (rbuf == MAP_FAILED) {
    return -1; // Failed to map shared memory
  }

  // clear memory: 
  memset(rbuf, 0, sizeof(rbuf_t));

  if (out_rbuf) {
    *out_rbuf = rbuf;
  }

  return 0;
}

int rbuflogd_cleanup_shmem() {
  if (shmem_fd != -1) {
    close(shmem_fd);
    shm_unlink(SHMEM_NAME);
  }
  return 0;
}