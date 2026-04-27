# NanoKernel OS

NanoKernel OS is a minimal 32-bit x86 protected-mode hobby operating system kernel loaded by GRUB via Multiboot v1.

## Features

- GRUB Multiboot v1 compatible kernel image
- VGA text mode output
- Flat 32-bit GDT setup
- IDT with exception, IRQ0, and `int 0x80` entries
- PIC remapping and EOI support
- PIT timer and global tick counter
- Interrupt dispatcher in C
- Syscalls via `int 0x80`:
  - `SYS_WRITE`
  - `SYS_SLEEP`
  - `SYS_GETPID`
- Round-robin scheduler
- Two demo tasks that run repeatedly
- Bootable ISO image for QEMU and VirtualBox

## Architecture Overview

- `arch/x86/boot/`
  - Multiboot header and kernel entry point
- `arch/x86/`
  - GDT/IDT load helpers, interrupt stubs, context restore, syscall wrappers
- `kernel/`
  - Core kernel logic: VGA, GDT, IDT, PIC, PIT, interrupts, scheduler, tasks
- `include/`
  - Public kernel headers
- `grub/grub.cfg`
  - GRUB menu configuration

## Build Requirements

Recommended:
- `i686-elf-gcc`
- `i686-elf-ld`
- `nasm`
- `grub-mkrescue`
- `xorriso`
- `qemu-system-i386`

Fallback supported in Makefile:
- host `gcc/ld/objcopy` with `-m32`

## Build Commands

```bash
make all
make kernel
make iso
```

Artifacts:
- `build/kernel.elf`
- `build/nanokernel.iso`

## Run in QEMU

```bash
make run
```

Equivalent:

```bash
qemu-system-i386 -cdrom build/nanokernel.iso
```

## Run in VirtualBox

```bash
make run-vbox
```

Then follow printed steps to create a VM and attach `build/nanokernel.iso` as optical media.

## Current Limitations

- 32-bit x86 only
- Paging is not enabled
- No true ring3 user mode yet
- No filesystem
- No disk driver
- All tasks share one address space

## Roadmap

- Paging and higher-half kernel mapping
- Ring3 user mode and privilege separation
- ELF user program loader
- Initrd support
- Filesystem support
- Keyboard driver
- Interactive shell
- IPC primitives
