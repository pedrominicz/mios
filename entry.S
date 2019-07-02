.code32

#define MULTIBOOT_MAGIC 0x1badb002
#define MULTIBOOT_FLAGS (0x00000001 | 0x00000002)

// Same as in "kernel.ld".
#define KERNEL_PHYSICAL_BASE 0x00100000 // 1MB.
#define KERNEL_VIRTUAL_BASE 0xf0100000
#define KERNEL_OFFSET (KERNEL_VIRTUAL_BASE - KERNEL_PHYSICAL_BASE)

.section .multiboot
.global _start
_start:
        jmp start

.balign 16
multiboot_header:
        .long MULTIBOOT_MAGIC
        .long MULTIBOOT_FLAGS
        .long -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS) // Multiboot checksum.
        .space 48 - (. - multiboot_header)

start:
        mov %ebx, %edi
        mov %eax, %esi

        mov %cr4, %eax
        or $0x10, %eax          // Enable page size extension.
        mov %eax, %cr4

        mov $page_directory - KERNEL_OFFSET, %eax
        mov %eax, %cr3

        mov %cr0, %eax
        and $0x9fffffff, %eax   // Enable caching and write-through caching.
        or $0x80000000, %eax    // Enable paging.
        mov %eax, %cr0

        mov $stack, %esp
        push $0
        popf

        push %edi               // Multiboot information structure.
        push %esi               // Multiboot magic.

        push $hang
        mov $mios_init, %eax
        jmp *%eax               // Far jump to `mios_init` with `hang` as the
                                // return value.

hang:
        hlt
        jmp hang

.data
        .space 4096
stack:

.balign 4096
page_directory:
        .long 0x00000083
        .space (KERNEL_VIRTUAL_BASE >> 22) * 4 - (. - page_directory)
        .long 0x00000087
        .space 4098 - (. - page_directory)
