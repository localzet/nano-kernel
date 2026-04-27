#include "console.h"
#include "interrupts.h"
#include "io.h"
#include "scheduler.h"

typedef struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

extern void irq0_stub(void);
extern void int80_stub(void);

static idt_entry_t idt[256];
static idt_ptr_t idtp;

static void idt_set_gate(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[vector].offset_low = (uint16_t)(handler & 0xFFFF);
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

static void pic_remap(void) {
    uint8_t a1 = inb(0x21);
    uint8_t a2 = inb(0xA1);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, a1 & 0xFC);
    outb(0xA1, a2 | 0xFF);
}

void idt_init(void) {
    for (uint32_t i = 0; i < 256; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0x08;
        idt[i].zero = 0;
        idt[i].type_attr = 0x8E;
        idt[i].offset_high = 0;
    }

    pic_remap();
    idt_set_gate(0x20, (uint32_t)irq0_stub, 0x08, 0x8E);
    idt_set_gate(0x80, (uint32_t)int80_stub, 0x08, 0xEE);

    idtp.limit = (uint16_t)(sizeof(idt) - 1);
    idtp.base = (uint32_t)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idtp));
}

void pit_init(uint32_t frequency_hz) {
    uint32_t divisor = 1193182u / frequency_hz;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void interrupts_enable(void) {
    __asm__ volatile ("sti");
}

cpu_state_t* irq0_handler_c(cpu_state_t* state) {
    outb(0x20, 0x20);
    return scheduler_on_tick(state);
}

enum {
    SYSCALL_WRITE = 0,
    SYSCALL_SLEEP = 1,
    SYSCALL_GETPID = 2
};

cpu_state_t* int80_handler_c(cpu_state_t* state) {
    switch (state->eax) {
        case SYSCALL_WRITE:
            if ((const char*)state->ebx != 0) {
                console_write((const char*)state->ebx);
            }
            state->eax = 0;
            return state;
        case SYSCALL_SLEEP:
            return scheduler_on_sleep(state, state->ebx);
        case SYSCALL_GETPID:
            state->eax = (uint32_t)scheduler_current_pid();
            return state;
        default:
            state->eax = (uint32_t)-1;
            return state;
    }
}
