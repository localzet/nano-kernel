#ifndef NANOKERNEL_PIT_H
#define NANOKERNEL_PIT_H

#include "types.h"

void pit_init(uint32_t frequency);
void pit_on_tick(void);
uint32_t pit_get_ticks(void);

#endif
