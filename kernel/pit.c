#include "io.h"
#include "pit.h"

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_BASE_FREQUENCY 1193182u

static volatile uint32_t g_ticks = 0;

void pit_init(uint32_t frequency) {
    uint32_t divisor;
    if (frequency == 0) {
        frequency = 100;
    }

    divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
}

void pit_on_tick(void) {
    g_ticks++;
}

uint32_t pit_get_ticks(void) {
    return g_ticks;
}
