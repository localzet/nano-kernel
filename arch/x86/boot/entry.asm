[BITS 32]

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global kernel_entry
extern kernel_main

kernel_entry:
    cli
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
