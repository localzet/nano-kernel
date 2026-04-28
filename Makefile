PROJECT := nanokernel
BUILD_DIR := build
ISO_DIR := iso

CROSS ?= i686-elf-
CC := $(CROSS)gcc
LD := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

NASM := nasm
GRUB_MKRESCUE := grub-mkrescue

CC_FALLBACK := gcc
LD_FALLBACK := ld
OBJCOPY_FALLBACK := objcopy

ifeq ($(shell command -v $(CC) 2>/dev/null),)
CC := $(CC_FALLBACK)
LD := $(LD_FALLBACK)
OBJCOPY := $(OBJCOPY_FALLBACK)
endif

CFLAGS := -std=c99 -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-builtin -Wall -Wextra -Werror -O2 -Iinclude
LDFLAGS := -m elf_i386 -T linker.ld -nostdlib

C_SOURCES := \
	kernel/kernel.c \
	kernel/vga.c \
	kernel/gdt.c \
	kernel/idt.c \
	kernel/pic.c \
	kernel/pit.c \
	kernel/interrupts.c \
	kernel/scheduler.c \
	kernel/tasks.c \
	kernel/keyboard.c \
	kernel/shell.c

ASM_SOURCES := \
	arch/x86/boot/multiboot.asm \
	arch/x86/boot/entry.asm \
	arch/x86/gdt_flush.asm \
	arch/x86/idt_load.asm \
	arch/x86/interrupts.asm \
	arch/x86/context_switch.asm \
	arch/x86/syscall.asm

C_OBJS := $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(C_SOURCES))
ASM_OBJS := $(patsubst %.asm,$(BUILD_DIR)/obj/%.o,$(ASM_SOURCES))

KERNEL_ELF := $(BUILD_DIR)/kernel.elf
ISO_IMAGE := $(BUILD_DIR)/nanokernel.iso

.PHONY: all kernel iso run run-vbox clean

all: kernel iso

kernel: $(KERNEL_ELF)

$(BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/obj/%.o: %.asm
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 $< -o $@

$(KERNEL_ELF): $(C_OBJS) $(ASM_OBJS) linker.ld
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(ASM_OBJS) $(C_OBJS)

iso: $(ISO_IMAGE)

$(ISO_IMAGE): $(KERNEL_ELF) grub/grub.cfg
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $@ $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO_IMAGE)

run-vbox: iso
	@echo "VirtualBox quick steps:"
	@echo "1. Create VM: Type=Other, Version=Other/Unknown (32-bit)."
	@echo "2. Allocate RAM: 64MB or more."
	@echo "3. Disable hard disk (or leave empty)."
	@echo "4. Open Settings -> Storage -> attach $(abspath $(ISO_IMAGE)) as optical drive."
	@echo "5. Start VM."

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)
