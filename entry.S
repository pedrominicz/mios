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

        mov %cr4, %eax
        or $0x10, %eax          // Enable page size extension.
        mov %eax, %cr4

        mov $init_page_directory - KERNEL_OFFSET, %eax
        mov %eax, %cr3

        mov %cr0, %eax
        and $0x9fffffff, %eax   // Enable caching and write-through caching.
        or $0x80000000, %eax    // Enable paging.
        mov %eax, %cr0

        mov %ebx, multiboot_info // Pointer to Multiboot information structure.

        mov $init_stack, %esp

        call init               // Should never return. If it does, hang.

hang:
        hlt
        jmp hang

.data
.global multiboot_info
multiboot_info:                 // `const MultibootInfo* const multiboot_info`
        .space 4

        .space 4096
init_stack:

.balign 4096
init_page_directory:
        .long 0x00000083
        .space (KERNEL_VIRTUAL_BASE >> 22) * 4 - (. - init_page_directory)
        .long 0x00000083
        .space 4096 - (. - init_page_directory)
