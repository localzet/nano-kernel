#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x07

static volatile uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static uint32_t cursor_row = 0;
static uint32_t cursor_col = 0;

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)((uint8_t)c | ((uint16_t)color << 8));
}

static void vga_scroll(void) {
    uint32_t row;
    uint32_t col;

    for (row = 1; row < VGA_HEIGHT; row++) {
        for (col = 0; col < VGA_WIDTH; col++) {
            VGA_MEMORY[(row - 1) * VGA_WIDTH + col] = VGA_MEMORY[row * VGA_WIDTH + col];
        }
    }

    for (col = 0; col < VGA_WIDTH; col++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = vga_entry(' ', VGA_COLOR);
    }
}

static void vga_newline(void) {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
        cursor_row = VGA_HEIGHT - 1;
    }
}

void vga_clear(void) {
    uint32_t i;
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = vga_entry(' ', VGA_COLOR);
    }
    cursor_row = 0;
    cursor_col = 0;
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_newline();
        return;
    }

    VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(c, VGA_COLOR);
    cursor_col++;

    if (cursor_col >= VGA_WIDTH) {
        vga_newline();
    }
}

void vga_write(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}

void vga_write_hex(uint32_t value) {
    static const char* hex = "0123456789ABCDEF";
    int i;
    vga_write("0x");
    for (i = 7; i >= 0; i--) {
        uint8_t nibble = (uint8_t)((value >> (i * 4)) & 0xF);
        vga_putc(hex[nibble]);
    }
}

void vga_write_dec(uint32_t value) {
    char buf[16];
    uint32_t i = 0;

    if (value == 0) {
        vga_putc('0');
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        vga_putc(buf[--i]);
    }
}
