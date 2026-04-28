#include "fs.h"
#include "io.h"
#include "keyboard.h"
#include "paging.h"
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

static void shell_write_hex32(uint32_t value) {
    static const char* hex = "0123456789ABCDEF";
    int32_t i;
    shell_write("0x");
    for (i = 7; i >= 0; i--) {
        char out[2];
        out[0] = hex[(value >> (i * 4)) & 0xFu];
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

static int32_t shell_startswith(const char* text, const char* prefix) {
    while (*prefix) {
        if (*text != *prefix) {
            return 0;
        }
        text++;
        prefix++;
    }
    return 1;
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
    shell_write("  ls     - list initrd files\n");
    shell_write("  cat X  - print file contents\n");
    shell_write("  mem    - show paging/memory info\n");
    shell_write("  about  - show kernel info\n");
    shell_write("  reboot - reboot machine\n");
}

static void shell_print_file(const uint8_t* data, uint32_t size) {
    uint32_t i;
    for (i = 0; i < size; i++) {
        char out[2];
        out[0] = (char)data[i];
        out[1] = '\0';
        shell_write(out);
    }
}

static void shell_list_entry(const fs_file_t* file, void* user) {
    (void)user;
    shell_write(file->name);
    shell_write(" (");
    shell_write_u32(file->size);
    shell_write(" bytes)\n");
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

static void shell_print_mem(void) {
    shell_write("paging enabled: ");
    shell_write(paging_is_enabled() ? "yes\n" : "no\n");

    shell_write("identity mapped range: ");
    shell_write_hex32(paging_identity_start());
    shell_write(" - ");
    shell_write_hex32(paging_identity_end());
    shell_write("\n");

    shell_write("kernel start: ");
    shell_write_hex32(paging_kernel_start());
    shell_write("\n");
    shell_write("kernel end: ");
    shell_write_hex32(paging_kernel_end());
    shell_write("\n");
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
    } else if (shell_streq(line, "ls")) {
        fs_list(shell_list_entry, (void*)0);
    } else if (shell_startswith(line, "cat ")) {
        const char* name = line + 4;
        const uint8_t* data;
        uint32_t size;
        if (*name == '\0') {
            shell_write("Usage: cat <filename>\n");
        } else if (fs_read(name, &data, &size) != 0) {
            shell_write("File not found: ");
            shell_write(name);
            shell_write("\n");
        } else {
            shell_print_file(data, size);
            shell_write("\n");
        }
    } else if (shell_streq(line, "mem")) {
        shell_print_mem();
    } else if (shell_streq(line, "about")) {
        shell_write("NanoKernel OS v0.5\n");
        shell_write("Keyboard + shell + initrd FS + paging\n");
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
