[BITS 32]
section .text
global kernel_entry
extern kmain
extern __bss_start
extern __bss_end

kernel_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    cld
    rep stosb

    call kmain

.halt:
    cli
    hlt
    jmp .halt
