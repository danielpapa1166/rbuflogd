#ifndef RBUFLOGD_SHMEM_H
#define RBUFLOGD_SHMEM_H

#include "rbuflogd_common_types.h"

int rbuflogd_init_shmem(rbuf_t ** out_rbuf); 
int rbuflogd_cleanup_shmem();

#endif // RBUFLOGD_SHMEM_H