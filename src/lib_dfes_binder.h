#ifndef LIB_DFES_BINDER_H
#define LIB_DFES_BINDER_H

#define MAX_DFES_BINDER_MSG_BODY_LEN 4000
typedef struct dfes_binder_message_t {
    uint32 msgno;
    size_t len;
    uint8  buf[MAX_DFES_BINDER_MSG_BODY_LEN];
} dfes_binder_message_t;

#endif
