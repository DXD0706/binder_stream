#ifndef DFES_BINDER_H
#define DFES_BINDER_H

#define MAX_DFES_BINDER_MSG_LEN 4096

typedef struct raw_msg_t {
    uint32 msgno;
    uint32 len;
    uint8  buf[MAX_DFES_BINDER_MSG_LEN];
} raw_msg_t;


typedef struct raw_msg_half_t {
    uint32 len;
    uint8  buf[MAX_DFES_BINDER_MSG_LEN];
} raw_msg_half_t;

#endif

