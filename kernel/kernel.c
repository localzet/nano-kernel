#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "pic.h"
#include "pit.h"
#include "scheduler.h"
#include "tasks.h"
#include "types.h"
#include "vga.h"

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info) {
    (void)multiboot_info;

    vga_clear();
    vga_write("NanoKernel OS\n");

    if (multiboot_magic == 0x2BADB002) {
        vga_write("Booted via GRUB Multiboot\n");
    } else {
        vga_write("Invalid multiboot magic: ");
        vga_write_hex(multiboot_magic);
        vga_write("\n");
    }

    vga_write("Initializing GDT... ");
    gdt_init();
    vga_write("OK\n");

    vga_write("Initializing IDT... ");
    interrupts_init();
    vga_write("OK\n");

    vga_write("Initializing PIC... ");
    pic_init();
    vga_write("OK\n");

    vga_write("Initializing PIT... ");
    pit_init(100);
    vga_write("OK\n");

    vga_write("Initializing keyboard... ");
    keyboard_init();
    vga_write("OK\n");

    vga_write("Initializing syscalls... OK\n");

    scheduler_init();
    tasks_init();

    interrupts_enable();

    vga_write("Starting scheduler...\n\n");
    scheduler_start();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
