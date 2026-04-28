#include "interrupts.h"
#include "idt.h"
#include "keyboard.h"
#include "pic.h"
#include "pit.h"
#include "scheduler.h"
#include "syscall.h"
#include "vga.h"

static const char* g_exception_names[32] = {
    "Divide-by-zero",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound range",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 floating-point",
    "Alignment check",
    "Machine check",
    "SIMD floating-point",
    "Virtualization",
    "Control protection",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor injection",
    "VMM communication",
    "Security",
    "Reserved"
};

void interrupts_init(void) {
    idt_init();
}

void interrupts_enable(void) {
    __asm__ volatile ("sti");
}

static void panic_exception(interrupt_frame_t* frame) {
    vga_write("\n[EXCEPTION] ");
    if (frame->int_no < 32) {
        vga_write(g_exception_names[frame->int_no]);
    } else {
        vga_write("Unknown");
    }
    vga_write(" int=");
    vga_write_dec(frame->int_no);
    vga_write(" err=");
    vga_write_hex(frame->err_code);
    vga_write("\nSystem halted.\n");

    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static interrupt_frame_t* handle_syscall(interrupt_frame_t* frame) {
    switch (frame->eax) {
        case SYS_WRITE:
            if ((const char*)frame->ebx) {
                vga_write((const char*)frame->ebx);
                frame->eax = 0;
            } else {
                frame->eax = (uint32_t)-1;
            }
            return frame;

        case SYS_SLEEP:
            return scheduler_on_sleep(frame, frame->ebx);

        case SYS_GETPID:
            frame->eax = (uint32_t)scheduler_current_pid();
            return frame;

        default:
            frame->eax = (uint32_t)-1;
            return frame;
    }
}

interrupt_frame_t* interrupt_dispatch(interrupt_frame_t* frame) {
    if (frame->int_no < 32) {
        panic_exception(frame);
        return frame;
    }

    if (frame->int_no == 32) {
        pit_on_tick();
        pic_send_eoi(0);
        return scheduler_on_timer(frame);
    }

    if (frame->int_no == 33) {
        keyboard_handle_irq();
        pic_send_eoi(1);
        return frame;
    }

    if (frame->int_no == 128) {
        return handle_syscall(frame);
    }

    return frame;
}
