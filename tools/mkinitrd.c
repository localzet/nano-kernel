#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITRD_MAGIC 0x4E4B5244u
#define INITRD_NAME_LEN 32u
#define INITRD_MAX_FILES 32u

typedef struct initrd_header {
    uint32_t magic;
    uint32_t file_count;
} __attribute__((packed)) initrd_header_t;

typedef struct initrd_entry {
    char name[INITRD_NAME_LEN];
    uint32_t offset;
    uint32_t size;
} __attribute__((packed)) initrd_entry_t;

static const char* basename_of(const char* path) {
    const char* base = path;
    const char* p = path;
    while (*p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
        p++;
    }
    return base;
}

static long file_size(FILE* f) {
    long current = ftell(f);
    long size;
    if (fseek(f, 0, SEEK_END) != 0) {
        return -1;
    }
    size = ftell(f);
    if (fseek(f, current, SEEK_SET) != 0) {
        return -1;
    }
    return size;
}

int main(int argc, char** argv) {
    FILE* out;
    initrd_header_t header;
    initrd_entry_t entries[INITRD_MAX_FILES];
    uint32_t file_count;
    uint32_t i;
    uint32_t data_offset;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <output.bin> <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    file_count = (uint32_t)(argc - 2);
    if (file_count > INITRD_MAX_FILES) {
        fprintf(stderr, "too many files (max %u)\n", INITRD_MAX_FILES);
        return 1;
    }

    memset(entries, 0, sizeof(entries));
    data_offset = (uint32_t)(sizeof(initrd_header_t) + file_count * sizeof(initrd_entry_t));

    for (i = 0; i < file_count; i++) {
        const char* path = argv[i + 2];
        const char* base = basename_of(path);
        FILE* in = fopen(path, "rb");
        long size;

        if (in == NULL) {
            fprintf(stderr, "cannot open %s\n", path);
            return 1;
        }

        size = file_size(in);
        fclose(in);

        if (size < 0) {
            fprintf(stderr, "cannot get size for %s\n", path);
            return 1;
        }

        if (strlen(base) >= INITRD_NAME_LEN) {
            fprintf(stderr, "filename too long: %s\n", base);
            return 1;
        }

        strncpy(entries[i].name, base, INITRD_NAME_LEN - 1);
        entries[i].name[INITRD_NAME_LEN - 1] = '\0';
        entries[i].offset = data_offset;
        entries[i].size = (uint32_t)size;
        data_offset += (uint32_t)size;
    }

    out = fopen(argv[1], "wb");
    if (out == NULL) {
        fprintf(stderr, "cannot create %s\n", argv[1]);
        return 1;
    }

    header.magic = INITRD_MAGIC;
    header.file_count = file_count;

    if (fwrite(&header, sizeof(header), 1, out) != 1) {
        fclose(out);
        return 1;
    }

    if (fwrite(entries, sizeof(initrd_entry_t), file_count, out) != file_count) {
        fclose(out);
        return 1;
    }

    for (i = 0; i < file_count; i++) {
        const char* path = argv[i + 2];
        FILE* in = fopen(path, "rb");
        uint8_t buffer[256];
        size_t n;

        if (in == NULL) {
            fclose(out);
            return 1;
        }

        while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
            if (fwrite(buffer, 1, n, out) != n) {
                fclose(in);
                fclose(out);
                return 1;
            }
        }

        fclose(in);
    }

    fclose(out);
    return 0;
}
