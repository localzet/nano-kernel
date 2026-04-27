[BITS 32]

section .text
global context_restore

; Restore saved interrupt frame and continue with iretd.
context_restore:
    mov esp, [esp + 4]

    pop gs
    pop fs
    pop es
    pop ds

    popa
    add esp, 8
    iretd
