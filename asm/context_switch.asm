[BITS 32]
section .text

global start_first_task

; void start_first_task(uint32_t task_esp);
start_first_task:
    mov esp, [esp + 4]
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iretd
