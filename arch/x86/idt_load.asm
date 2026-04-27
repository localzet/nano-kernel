[BITS 32]

section .text
global idt_load

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret
