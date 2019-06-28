.code32

#define INTERRUPT_PUSH(n) \
        push $##n;              \
        push %ds;               \
        push %es;               \
        push %fs;               \
        push %gs;               \
        pusha;                  \
        mov $0x10, %ax;         \
        mov %ax, %ds;           \
        mov %ax, %ss;           \
        push %esp;

#define INTERRUPT_POP_RET \
        add $4, %esp;           \
        popa;                   \
        pop %gs;                \
        pop %fs;                \
        pop %es;                \
        pop %ds;                \
        add $8, %esp;           \
        iret;

#define INTERRUPT(n) \
.text;                          \
interrupt_##n:                  \
        push $0;                \
        INTERRUPT_PUSH(n);      \
        call trap;              \
        INTERRUPT_POP_RET;      \
.data;                          \
        .long interrupt_##n;

#define INTERRUPT_ERROR(n) \
.text;                          \
interrupt_##n:                  \
        INTERRUPT_PUSH(n);      \
        call trap;              \
        INTERRUPT_POP_RET;      \
.data;                          \
        .long interrupt_##n;

#define INTERRUPT_MASTER(n) \
.text;                          \
interrupt_##n:                  \
        push $0;                \
        INTERRUPT_PUSH(n);      \
        mov $0x20, %al;         \
        outb $0x20;             \
        call trap;              \
        INTERRUPT_POP_RET;      \
.data;                          \
        .long interrupt_##n;

#define INTERRUPT_SLAVE(n) \
.text;                          \
interrupt_##n:                  \
        push $0;                \
        INTERRUPT_PUSH(n);      \
        mov $0x20, %al;         \
        outb $0x20;             \
        outb $0xa0;             \
        call trap;              \
        INTERRUPT_POP_RET;      \
.data;                          \
        .long interrupt_##n;

.data
.balign 16
.global traps
traps:                          // `traps`will be accessed from C as
                                // `uintptr_t traps[256]`.

INTERRUPT(0x00)                 // Divide error (`div` and `idiv`).
INTERRUPT(0x01)                 // Debug exception.
INTERRUPT(0x02)                 // Non Maskable interrupt (NMI).
INTERRUPT(0x03)                 // Breakpoint (`int 3`).
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
INTERRUPT_MASTER(0x20)          // Programmable interrupt timer (PIT).
INTERRUPT_MASTER(0x21)          // Keyboard interrupt.
INTERRUPT_MASTER(0x22)          // Cascade (used internally by the programmable
                                // interrupt controllers, never raised).
INTERRUPT_MASTER(0x23)          // Serial communication port 2 (if enabled).
INTERRUPT_MASTER(0x24)          // Serial communication port 1 (if enabled).
INTERRUPT_MASTER(0x25)          // Parallel port 2 (if enabled).
INTERRUPT_MASTER(0x26)          // Floppy disk.
INTERRUPT_MASTER(0x27)          // Spurious interrupt (parallel port 1).
INTERRUPT_SLAVE(0x28)           // CMOS real-time clock (if enabled).
INTERRUPT_SLAVE(0x29)           // Free for peripherals.
INTERRUPT_SLAVE(0x2a)           // Free for peripherals.
INTERRUPT_SLAVE(0x2b)           // Free for peripherals.
INTERRUPT_SLAVE(0x2c)           // PS/2 mouse.
INTERRUPT_SLAVE(0x2d)           // FPU / coprocessor / inter-processor.
INTERRUPT_SLAVE(0x2e)           // Primary ATA hard disk.
INTERRUPT_SLAVE(0x2f)           // Secondary ATA hard disk.

.data
        .space 1024 - (. - traps)
