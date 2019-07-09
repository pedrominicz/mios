#include "interrupt.h"

.code32

.text
interrupt_common:
        push %ds
        push %es
        push %fs
        push %gs
        pusha
        mov $0x10, %ax
        mov %ax, %ds
        mov %ax, %ss
        push %esp
        call interrupt_handler
        add $4, %esp
        popa
        pop %gs
        pop %fs
        pop %es
        pop %ds
        add $8, %esp
        iret

#define INTERRUPT_COMMON(n, error) \
.text;                          \
interrupt_##n:                  \
        error;                  \
        push $##n;              \
        jmp interrupt_common;   \
.data;                          \
        .long interrupt_##n;

#define INTERRUPT(n)            INTERRUPT_COMMON(n, push $0)
#define INTERRUPT_ERROR(n)      INTERRUPT_COMMON(n, ;)

.data
.balign 4
.global interrupts
interrupts:                     // `const uintptr_t interrupts[]`.

INTERRUPT(0x00)                 // Divide error (`div` and `idiv`).
INTERRUPT(0x01)                 // Debug exception.
INTERRUPT(0x02)                 // Non maskable interrupt (NMI).
INTERRUPT(0x03)                 // Breakpoint (`int $3`).
INTERRUPT(0x04)                 // Overflow (`into`).
INTERRUPT(0x05)                 // Bound range exception (`bound`).
INTERRUPT(0x06)                 // Invalid opcode (`ud2`).
INTERRUPT(0x07)                 // Device not available (`wait` and `fwait`).
INTERRUPT_ERROR(0x08)           // Double fault (error code always 0).
INTERRUPT(0x09)                 // Coprocessor segment overrun (floating-point
                                // instructions).
INTERRUPT_ERROR(0x0a)           // Invalid Task Statement Segment (TTS).
INTERRUPT_ERROR(0x0b)           // Segment not present.
INTERRUPT_ERROR(0x0c)           // Stack segment fault.
INTERRUPT_ERROR(0x0d)           // General protection.
INTERRUPT_ERROR(0x0e)           // Page fault.
INTERRUPT(0x0f)                 // Intel reserved.

INTERRUPT(0x10)                 // x87 FPU floating-point error.
INTERRUPT_ERROR(0x11)           // Alignment check (error code always 0).
INTERRUPT(0x12)                 // Machine check.
INTERRUPT(0x13)                 // SIMD floating-point exception.
INTERRUPT(0x14)                 // Virtualization exception.
INTERRUPT(0x15)                 // Intel reserved.
INTERRUPT(0x16)                 // Intel reserved.
INTERRUPT(0x17)                 // Intel reserved.
INTERRUPT(0x18)                 // Intel reserved.
INTERRUPT(0x19)                 // Intel reserved.
INTERRUPT(0x1a)                 // Intel reserved.
INTERRUPT(0x1b)                 // Intel reserved.
INTERRUPT(0x1c)                 // Intel reserved.
INTERRUPT(0x1d)                 // Intel reserved.
INTERRUPT(0x1e)                 // Intel reserved.
INTERRUPT(0x1f)                 // Intel reserved.

// From here on interrupts are user-defined.
INTERRUPT(0x20)                 // Programmable interrupt timer (PIT).
INTERRUPT(0x21)                 // Keyboard interrupt.
INTERRUPT(0x22)                 // Cascade (used internally by the programmable
                                // interrupt controllers, never raised).
INTERRUPT(0x23)                 // Serial communication port 2 (if enabled).
INTERRUPT(0x24)                 // Serial communication port 1 (if enabled).
INTERRUPT(0x25)                 // Parallel port 2 (if enabled).
INTERRUPT(0x26)                 // Floppy disk.
INTERRUPT(0x27)                 // Spurious interrupt (parallel port 1).
INTERRUPT(0x28)                 // CMOS real-time clock (if enabled).
INTERRUPT(0x29)                 // Free for peripherals.
INTERRUPT(0x2a)                 // Free for peripherals.
INTERRUPT(0x2b)                 // Free for peripherals.
INTERRUPT(0x2c)                 // PS/2 mouse.
INTERRUPT(0x2d)                 // FPU / coprocessor / inter-processor.
INTERRUPT(0x2e)                 // Primary ATA hard disk.
INTERRUPT(0x2f)                 // Secondary ATA hard disk.

INTERRUPT(SYSCALL_INTERRUPT)

.data
        .long 0                 // Null-terminate `interrupts`.
