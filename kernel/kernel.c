#include "console.h"
#include "interrupts.h"
#include "scheduler.h"
#include "user_program.h"

static volatile uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;
static uint16_t cursor_row = 0;
static uint16_t cursor_col = 0;

typedef struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

static uint64_t gdt[3];

static void gdt_init(void) {
    gdt[0] = 0x0000000000000000ULL;
    gdt[1] = 0x00CF9A000000FFFFULL;
    gdt[2] = 0x00CF92000000FFFFULL;

    gdt_ptr_t ptr;
    ptr.limit = (uint16_t)(sizeof(gdt) - 1);
    ptr.base = (uint32_t)&gdt;

    __asm__ volatile ("lgdt %0" : : "m"(ptr));
    __asm__ volatile (
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        :
        :
        : "ax"
    );
}

static void advance_cursor(void) {
    if (cursor_col >= 80) {
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= 25) {
        cursor_row = 0;
    }
}

void console_clear(void) {
    for (uint16_t r = 0; r < 25; r++) {
        for (uint16_t c = 0; c < 80; c++) {
            VGA_BUFFER[r * 80 + c] = (uint16_t)(' ' | (0x07 << 8));
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

void console_putc(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= 25) {
            cursor_row = 0;
        }
        return;
    }

    VGA_BUFFER[cursor_row * 80 + cursor_col] = (uint16_t)((uint8_t)c | (0x07 << 8));
    cursor_col++;
    advance_cursor();
}

void console_write(const char* str) {
    while (*str != '\0') {
        console_putc(*str++);
    }
}

void console_write_dec(unsigned int value) {
    char buf[16];
    int i = 0;
    if (value == 0) {
        console_putc('0');
        return;
    }
    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (i > 0) {
        console_putc(buf[--i]);
    }
}

void kmain(void) {
    console_clear();
    console_write("[Kernel] Initialized\n");

    gdt_init();
    idt_init();
    pit_init(100);

    scheduler_init();
    scheduler_add_task(1, user_program_a);
    scheduler_add_task(2, user_program_b);

    interrupts_enable();
    scheduler_start();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
