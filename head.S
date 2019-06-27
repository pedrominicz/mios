#define MULTIBOOT_MAGIC 0x1badb002
#define MULTIBOOT_FLAGS (0x00000001 | 0x00000002)

.text
.balign 8
multiboot_header:
        .long MULTIBOOT_MAGIC
        .long MULTIBOOT_FLAGS
        .long -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS) // Multiboot checksum

.global _start
_start:
        mov $stack_start, %esp
        push $0
        popf

        push %ebx
        push %eax
        call mios_init
hang:
        cli
        hlt
        jmp hang

.data
.balign 16
stack:
        .space 4096
stack_start:
