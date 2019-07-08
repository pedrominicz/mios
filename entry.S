#include "memory.h"

.code32

#define MULTIBOOT_MAGIC         0x1badb002
#define MULTIBOOT_ALIGN_MODULES 0x00000001
#define MULTIBOOT_MEMORY_INFO   0x00000002
#define MULTIBOOT_FLAGS         (MULTIBOOT_ALIGN_MODULES | MULTIBOOT_MEMORY_INFO)

.section .multiboot
.balign 16
multiboot_header:
        .long MULTIBOOT_MAGIC
        .long MULTIBOOT_FLAGS
        .long -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS) // Multiboot checksum.
        .space 48 - (. - multiboot_header)

.text
.global entry
entry:
        cmp $0x2badb002, %eax   // Multiboot magic.
        jne hang

        mov %ebx, multiboot_info_address - KERNEL_OFFSET

        mov %cr4, %eax
        or $0x10, %eax          // Enable page size extension.
        mov %eax, %cr4

        mov $init_page_directory - KERNEL_OFFSET, %eax
        mov %eax, %cr3

        mov %cr0, %eax
        and $0x9fffffff, %eax   // Enable caching and write-through caching.
        or $0x80000000, %eax    // Enable paging.
        mov %eax, %cr0

        mov $init_stack, %esp
        push $0
        popf

        lgdt (init_gdt_descriptor)
        mov $KERNEL_DATA_SELECTOR, %ax
        mov %ax, %ds
        mov %ax, %es
        mov %ax, %fs
        mov %ax, %gs
        mov %ax, %ss
        jmp $KERNEL_CODE_SELECTOR, $1f
1:

        call init               // Should never return. If it does, hang.

hang:
        hlt
        jmp hang

.data
        .space 4096
init_stack:

.balign 4096
init_page_directory:
        .long 0x00000083
        .space (KERNEL_VIRTUAL_BASE >> 22) * 4 - (. - init_page_directory)
        .long 0x00000083
        .space 4096 - (. - init_page_directory)

.global kernel_page_directory   // `uint32_t kernel_page_directory[1024]`
.balign 4096
kernel_page_directory:
        .space 4096

init_gdt:
        .quad 0x0000000000000000 // Null segment.
        .quad 0x00cf9a000000ffff // Kernel code segment.
        .quad 0x00cf92000000ffff // Kernel data segment.
init_gdt_end:

init_gdt_descriptor:
        .word init_gdt_end - init_gdt - 1
        .long init_gdt

.global multiboot_info_address
multiboot_info_address:         // `const uintptr_t multiboot_info_address`
        .space 4
