#ifndef CONSOLE_H
#define CONSOLE_H

void console_clear(void);
void console_putc(char c);
void console_write(const char* str);
void console_write_dec(unsigned int value);

#endif
