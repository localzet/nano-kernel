[BITS 32]
section .text

global sys_write
global sys_sleep
global sys_getpid

; int sys_write(const char* str);
sys_write:
    mov eax, 0
    mov ebx, [esp + 4]
    int 0x80
    ret

; int sys_sleep(int ticks);
sys_sleep:
    mov eax, 1
    mov ebx, [esp + 4]
    int 0x80
    ret

; int sys_getpid(void);
sys_getpid:
    mov eax, 2
    int 0x80
    ret
