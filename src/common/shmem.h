#ifndef RBUFLOGD_SHMEM_H
#define RBUFLOGD_SHMEM_H

#include "rbuflogd/common_types.h"

int rbuflogd_init_shmem(rbuf_t ** out_rbuf); 
int rbuflogd_cleanup_shmem(void);

#endif // RBUFLOGD_SHMEM_H