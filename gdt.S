#include "gdt.h"

.code32

.text
.global init_gdt
init_gdt:
        pushf
        cli
        lgdt (gdt_descriptor)
        mov $KERNEL_DATA_SELECTOR, %ax
        mov %ax, %ds
        mov %ax, %es
        mov %ax, %fs
        mov %ax, %gs
        mov %ax, %ss
        jmp $KERNEL_CODE_SELECTOR, $1f
1:
        popf
        ret

.data
.balign 16
gdt:
.quad 0x0000000000000000        // Null segment.
.quad 0x00cf9a000000ffff        // Kernel code segment.
.quad 0x00cf92000000ffff        // Kernel cata segment.
.quad 0x00cffa000000ffff        // User mode code segment.
.quad 0x00cff2000000ffff        // User mode data segment.
gdt_end:

.balign 16
gdt_descriptor:
.word gdt_end - gdt - 1
.long gdt
