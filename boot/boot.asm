[BITS 16]
[ORG 0x7C00]

KERNEL_LOAD_SEG   equ 0x0000
KERNEL_LOAD_OFF   equ 0x1000
KERNEL_SECTORS    equ 64

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl

    mov si, boot_msg
    call print_string

    ; Read kernel from disk: cylinder 0, head 0, sector 2
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [boot_drive]
    mov bx, KERNEL_LOAD_OFF
    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    int 0x13
    jc disk_error

    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:protected_mode

print_string:
    mov ah, 0x0E
.next:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .next
.done:
    ret

disk_error:
    mov si, disk_err_msg
    call print_string
    hlt
    jmp $

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp 0x08:KERNEL_LOAD_OFF

gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

boot_drive:   db 0
boot_msg:     db "NanoKernel booting...", 0
disk_err_msg: db "Disk read error", 0

times 510-($-$$) db 0
dw 0xAA55
