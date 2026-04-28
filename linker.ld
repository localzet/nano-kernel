ENTRY(kernel_entry)

SECTIONS
{
    . = 1M;
    kernel_start = .;

    .multiboot ALIGN(4) : {
        KEEP(*(.multiboot))
    }

    .text ALIGN(4K) : {
        *(.text*)
    }

    .rodata ALIGN(4K) : {
        *(.rodata*)
    }

    .data ALIGN(4K) : {
        *(.data*)
    }

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss*)
    }

    kernel_end = .;
}
