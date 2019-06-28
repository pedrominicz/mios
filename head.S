.code32

#define MULTIBOOT_MAGIC 0x1badb002
#define MULTIBOOT_FLAGS (0x00000001 | 0x00000002)

.section .multiboot
.global _start
_start:
        jmp start

.balign 16
multiboot_header:
        .long MULTIBOOT_MAGIC
        .long MULTIBOOT_FLAGS
        .long -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS) // Multiboot checksum.

start:
        mov $stack, %esp
        push $0
        popf

        push %ebx // Multiboot information structure.
        push %eax // Multiboot magic.
        call mios_init
hang:
        hlt
        jmp hang

.data
.balign 16
        .space 4096
stack:
