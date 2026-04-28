#include "io.h"
#include "keyboard.h"
#include "pit.h"
#include "scheduler.h"
#include "shell.h"
#include "syscall.h"
#include "vga.h"

static void shell_write(const char* s) {
    sys_write(s);
}

static void shell_write_u32(uint32_t value) {
    char buf[16];
    int32_t i = 0;

    if (value == 0) {
        shell_write("0");
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        char out[2];
        out[0] = buf[--i];
        out[1] = '\0';
        shell_write(out);
    }
}

static int32_t shell_streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

static const char* shell_task_state_name(task_state_t state) {
    switch (state) {
        case TASK_UNUSED:
            return "unused";
        case TASK_READY:
            return "ready";
        case TASK_RUNNING:
            return "running";
        case TASK_SLEEPING:
            return "sleeping";
        default:
            return "unknown";
    }
}

static void shell_print_help(void) {
    shell_write("Commands:\n");
    shell_write("  help   - show commands\n");
    shell_write("  clear  - clear screen\n");
    shell_write("  ticks  - show PIT tick counter\n");
    shell_write("  ps     - show tasks\n");
    shell_write("  about  - show kernel info\n");
    shell_write("  reboot - reboot machine\n");
}

static void shell_print_ps(void) {
    int32_t i;
    scheduler_task_info_t info;

    shell_write("PID STATE    SLEEP_UNTIL\n");
    for (i = 0; i < scheduler_task_count(); i++) {
        if (scheduler_get_task_info(i, &info) != 0) {
            continue;
        }

        shell_write(" ");
        shell_write_u32((uint32_t)info.pid);
        shell_write("   ");
        shell_write(shell_task_state_name(info.state));
        shell_write("   ");
        shell_write_u32(info.sleep_until);
        shell_write("\n");
    }
}

static void shell_reboot(void) {
    uint32_t i;
    shell_write("Rebooting...\n");

    for (i = 0; i < 100000u; i++) {
        if ((inb(0x64) & 0x02u) == 0) {
            break;
        }
    }

    outb(0x64, 0xFE);
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void shell_execute(const char* line) {
    if (shell_streq(line, "help")) {
        shell_print_help();
    } else if (shell_streq(line, "clear")) {
        vga_clear();
    } else if (shell_streq(line, "ticks")) {
        shell_write("ticks=");
        shell_write_u32(pit_get_ticks());
        shell_write("\n");
    } else if (shell_streq(line, "ps")) {
        shell_print_ps();
    } else if (shell_streq(line, "about")) {
        shell_write("NanoKernel OS v0.3\n");
        shell_write("Keyboard IRQ + minimal shell\n");
    } else if (shell_streq(line, "reboot")) {
        shell_reboot();
    } else if (line[0] == '\0') {
        return;
    } else {
        shell_write("Unknown command. Type 'help'.\n");
    }
}

void shell_task_entry(void) {
    char line[128];
    int32_t len = 0;

    shell_write("[shell] NanoKernel shell ready (type 'help')\n");
    shell_write("nk> ");

    for (;;) {
        int32_t ch = keyboard_get_char();
        if (ch < 0) {
            sys_sleep(1);
            continue;
        }

        if (ch == '\n') {
            line[len] = '\0';
            shell_write("\n");
            shell_execute(line);
            len = 0;
            shell_write("nk> ");
            continue;
        }

        if (ch == '\b') {
            if (len > 0) {
                len--;
                shell_write("\b");
            }
            continue;
        }

        if (ch == '\t') {
            continue;
        }

        if (len < (int32_t)(sizeof(line) - 1)) {
            char out[2];
            line[len++] = (char)ch;
            out[0] = (char)ch;
            out[1] = '\0';
            shell_write(out);
        }
    }
}
