#ifndef SYSCALL_H
#define SYSCALL_H

int sys_write(const char* str);
int sys_sleep(int ticks);
int sys_getpid(void);

#endif
