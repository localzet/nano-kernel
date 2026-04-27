#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

typedef struct cpu_state {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} cpu_state_t;

void idt_init(void);
void interrupts_enable(void);
void pit_init(uint32_t frequency_hz);

cpu_state_t* irq0_handler_c(cpu_state_t* state);
cpu_state_t* int80_handler_c(cpu_state_t* state);

#endif
