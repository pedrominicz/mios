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
        mov $tss, %eax
        and $0x00ffffff, %eax
        or %eax, gdt_end - 6    // Task state segment base address (bits 0-23).
        mov $tss, %eax
        shr $24, %eax
        or %al, gdt_end - 1     // Task state segment base address
                                // (bits 24-31).
        mov $0x2b, %ax
        ltr %ax                 // Load task register
        popf
        ret

.global switch_user_mode
switch_user_mode:
        pop %edx
        mov %esp, tss + 4       // Stack pointer for ring 0.
        cli
        mov $0x23, %ax
        mov %ax, %ds
        mov %ax, %es
        mov %ax, %fs
        mov %ax, %gs
        push $0x23
        push $user_mode_stack
        pushf
        pop %eax
        or $0x200, %eax
        push %eax
        push $0x1b
        push $1f
        iret
1:
        push %edx
        ret

.data
gdt:
        .quad 0x0000000000000000 // Null segment.
        .quad 0x00cf9a000000ffff // Kernel code segment.
        .quad 0x00cf92000000ffff // Kernel cata segment.
        .quad 0x00cffa000000ffff // User mode code segment.
        .quad 0x00cff2000000ffff // User mode data segment.
        .quad 0x0000e90000000068 // Task state segment.
gdt_end:

gdt_descriptor:
        .word gdt_end - gdt - 1
        .long gdt

tss:
        .space 8
        .long KERNEL_DATA_SELECTOR // Stack segment.
        .space 60
        // Or-ing segment registers to make them accessible from privilege
        // level-3, i.e. user mode.
        .long KERNEL_DATA_SELECTOR | 3 // Extra segment.
        .long KERNEL_CODE_SELECTOR | 3 // Code segment.
        .long KERNEL_DATA_SELECTOR | 3 // Stack segment.
        .long KERNEL_DATA_SELECTOR | 3 // Data segment.
        .long KERNEL_DATA_SELECTOR | 3 // Extra segment #2 (`fs`).
        .long KERNEL_DATA_SELECTOR | 3 // Extra segment #3 (`gs`).
        .space 6
        .word 0xffff

        .space 4096
user_mode_stack:
