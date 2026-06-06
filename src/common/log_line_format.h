#ifndef LOG_LINE_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "common_types.h"

int format_log_line(const rbuf_entry_t * const entry, 
    const char * const boot_id, char * out_msg, size_t out_msg_sz); 

#ifdef __cplusplus
}
#endif
#endif /* LOG_LINE_FORMAT_H */