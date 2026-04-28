#include "fs.h"
#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "multiboot.h"
#include "paging.h"
#include "pic.h"
#include "pit.h"
#include "scheduler.h"
#include "tasks.h"
#include "types.h"
#include "vga.h"

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info) {
    const multiboot_info_t* mbi = (const multiboot_info_t*)(uintptr_t)multiboot_info;
    uint32_t initrd_mod_start = 0;
    uint32_t initrd_mod_end = 0;

    if (multiboot_magic == 0x2BADB002 &&
        mbi != (const multiboot_info_t*)0 &&
        (mbi->flags & MULTIBOOT_INFO_MODS) != 0 &&
        mbi->mods_count > 0) {
        const multiboot_module_t* mods = (const multiboot_module_t*)(uintptr_t)mbi->mods_addr;
        initrd_mod_start = mods[0].mod_start;
        initrd_mod_end = mods[0].mod_end;
    }

    vga_clear();
    vga_write("NanoKernel OS\n");

    if (multiboot_magic == 0x2BADB002) {
        vga_write("Booted via GRUB Multiboot\n");
    } else {
        vga_write("Invalid multiboot magic: ");
        vga_write_hex(multiboot_magic);
        vga_write("\n");
    }

    if (multiboot_magic == 0x2BADB002 &&
        mbi != (const multiboot_info_t*)0 &&
        (mbi->flags & MULTIBOOT_INFO_MODS) != 0 &&
        mbi->mods_count > 0) {
        const multiboot_module_t* mods = (const multiboot_module_t*)(uintptr_t)mbi->mods_addr;
        initrd_start = mods[0].mod_start;
        initrd_end = mods[0].mod_end;
        has_initrd_module = true;
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

    vga_write("Initializing paging... ");
    paging_init(initrd_mod_end);
    vga_write("OK\n");

    vga_write("Initializing PIT... ");
    pit_init(100);
    vga_write("OK\n");

    vga_write("Initializing keyboard... ");
    keyboard_init();
    vga_write("OK\n");

    vga_write("Initializing initrd FS... ");
    if (initrd_mod_end > initrd_mod_start) {
        fs_init(initrd_mod_start, initrd_mod_end);
        vga_write("OK\n");
    } else {
        fs_init(0, 0);
        vga_write("NO MODULE\n");
    }

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
