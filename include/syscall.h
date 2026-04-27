#ifndef NANOKERNEL_SYSCALL_H
#define NANOKERNEL_SYSCALL_H

#include "types.h"

enum {
    SYS_WRITE = 0,
    SYS_SLEEP = 1,
    SYS_GETPID = 2
};

int32_t sys_write(const char* str);
int32_t sys_sleep(uint32_t ticks);
int32_t sys_getpid(void);

#endif
