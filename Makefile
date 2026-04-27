BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

CROSS ?=
CC := $(if $(CROSS),$(CROSS)gcc,gcc)
LD := $(if $(CROSS),$(CROSS)ld,ld)
OBJCOPY := $(if $(CROSS),$(CROSS)objcopy,objcopy)
NASM := nasm

CFLAGS := -m32 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -Wall -Wextra -O2 -Iinclude
LDFLAGS := -m elf_i386 -T linker.ld -nostdlib

C_SOURCES := \
	kernel/kernel.c \
	kernel/interrupts.c \
	kernel/scheduler.c \
	user/user_program.c

ASM_ELF_SOURCES := \
	asm/kernel_entry.asm \
	asm/interrupts.asm \
	asm/context_switch.asm \
	asm/syscall.asm

C_OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst %.asm,$(OBJ_DIR)/%.o,$(ASM_ELF_SOURCES))
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
BOOT_BIN := $(BUILD_DIR)/boot.bin
IMAGE_BIN := $(BUILD_DIR)/os-image.bin

.PHONY: all clean run

all: $(IMAGE_BIN)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 $< -o $@

$(KERNEL_ELF): $(C_OBJECTS) $(ASM_OBJECTS) linker.ld
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(ASM_OBJECTS) $(C_OBJECTS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(BOOT_BIN): boot/boot.asm
	@mkdir -p $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(IMAGE_BIN): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	dd if=$(BOOT_BIN) of=$@ conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc status=none

run: $(IMAGE_BIN)
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE_BIN)

clean:
	rm -rf $(BUILD_DIR)
