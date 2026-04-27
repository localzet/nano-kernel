#include "syscall.h"

static void u32_to_dec(unsigned int value, char* out) {
    char buf[16];
    int i = 0;

    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        out[j++] = buf[--i];
    }
    out[j] = '\0';
}

void user_program_a(void) {
    int pid = sys_getpid();
    (void)pid;

    for (;;) {
        sys_write("[Proc1] Hello from task A\n");
        sys_sleep(20);
    }
}

void user_program_b(void) {
    unsigned int counter = 0;
    char num_buf[16];
    char msg[48];

    for (;;) {
        const char* prefix = "[Proc2] Counting: ";
        int p = 0;

        while (prefix[p] != '\0') {
            msg[p] = prefix[p];
            p++;
        }

        u32_to_dec(counter++, num_buf);
        int i = 0;
        while (num_buf[i] != '\0') {
            msg[p++] = num_buf[i++];
        }
        msg[p++] = '\n';
        msg[p] = '\0';

        sys_write(msg);
        sys_sleep(30);
    }
}
