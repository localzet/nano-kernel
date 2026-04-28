#ifndef NANOKERNEL_KEYBOARD_H
#define NANOKERNEL_KEYBOARD_H

#include "types.h"

void keyboard_init(void);
void keyboard_handle_irq(void);
int32_t keyboard_get_char(void);

#endif
