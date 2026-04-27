[BITS 32]
section .text

global irq0_stub
global int80_stub

extern irq0_handler_c
extern int80_handler_c

irq0_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq0_handler_c
    add esp, 4
    mov esp, eax

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iretd

int80_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call int80_handler_c
    add esp, 4
    mov esp, eax

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iretd
