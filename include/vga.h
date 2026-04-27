#ifndef NANOKERNEL_VGA_H
#define NANOKERNEL_VGA_H

#include "types.h"

void vga_clear(void);
void vga_putc(char c);
void vga_write(const char* str);
void vga_write_hex(uint32_t value);
void vga_write_dec(uint32_t value);

#endif
