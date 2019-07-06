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

#define INTERRUPT(n, handler) \
.text;                          \
interrupt_##n:                  \
        push $0;                \
        INTERRUPT_PUSH(n);      \
        call handler;           \
        INTERRUPT_POP_RET;      \
.data;                          \
        .long interrupt_##n;

#define INTERRUPT_ERROR(n, handler) \
.text;                          \
interrupt_##n:                  \
        INTERRUPT_PUSH(n);      \
        call handler;           \
        add $4, %esp;           \
        popa;                   \
        pop %gs;                \
        pop %fs;                \
        pop %es;                \
        pop %ds;                \
        add $4, %esp;           \
        iret;                   \
.data;                          \
        .long interrupt_##n;

#define INTERRUPT_MASTER(n, handler) \
.text;                          \
interrupt_##n:                  \
        push $0;                \
        INTERRUPT_PUSH(n);      \
        mov $0x20, %al;         \
        outb $0x20;             \
        call handler;           \
        INTERRUPT_POP_RET;      \
.data;                          \
        .long interrupt_##n;

#define INTERRUPT_SLAVE(n, handler) \
.text;                          \
interrupt_##n:                  \
        push $0;                \
        INTERRUPT_PUSH(n);      \
        mov $0x20, %al;         \
        outb $0x20;             \
        outb $0xa0;             \
        call handler;           \
        INTERRUPT_POP_RET;      \
.data;                          \
        .long interrupt_##n;

.data
.balign 4
.global interrupts
interrupts:                     // `uintptr_t interrupts[256]`.

// Divide error (`div` and `idiv`).
INTERRUPT(0x00, interrupt_handler)
// Debug exception.
INTERRUPT(0x01, interrupt_handler)
// Non maskable interrupt (NMI).
INTERRUPT(0x02, interrupt_handler)
// Breakpoint (`int $3`).
INTERRUPT(0x03, breakpoint_handler)
// Overflow (`into`).
INTERRUPT(0x04, interrupt_handler)
// Bound range exception (`bound`).
INTERRUPT(0x05, interrupt_handler)
// Invalid opcode (`ud2`).
INTERRUPT(0x06, interrupt_handler)
// Device not available (`wait` and `fwait`).
INTERRUPT(0x07, interrupt_handler)
// Double fault (error code always 0).
INTERRUPT_ERROR(0x08, interrupt_handler)
// Coprocessor segment overrun (floating-point instructions).
INTERRUPT(0x09, interrupt_handler)
// Invalid Task Statement Segment (TTS).
INTERRUPT_ERROR(0x0a, interrupt_handler)
// Segment not present.
INTERRUPT_ERROR(0x0b, interrupt_handler)
// Stack segment fault.
INTERRUPT_ERROR(0x0c, interrupt_handler)
// General protection.
INTERRUPT_ERROR(0x0d, interrupt_handler)
// Page fault.
INTERRUPT_ERROR(0x0e, interrupt_handler)
// Intel reserved.
.space 4

// x87 FPU floating-point error.
INTERRUPT(0x10, interrupt_handler)
// Alignment check (error code always 0).
INTERRUPT_ERROR(0x11, interrupt_handler)
// Machine check.
INTERRUPT(0x12, interrupt_handler)
// SIMD floating-point exception.
INTERRUPT(0x13, interrupt_handler)
// Virtualization exception.
INTERRUPT(0x14, interrupt_handler)
// Intel reserved (0x15-0x1f).
.space 44

// From here on interrupts are user-defined.

// Programmable interrupt timer (PIT).
INTERRUPT_MASTER(0x20, interrupt_handler)
// Keyboard interrupt.
INTERRUPT_MASTER(0x21, keyboard_handler)
// Cascade (used internally by the programmable interrupt controllers, never
// raised).
INTERRUPT_MASTER(0x22, interrupt_handler)
// Serial communication port 2 (if enabled).
INTERRUPT_MASTER(0x23, interrupt_handler)
// Serial communication port 1 (if enabled).
INTERRUPT_MASTER(0x24, interrupt_handler)
// Parallel port 2 (if enabled).
INTERRUPT_MASTER(0x25, interrupt_handler)
// Floppy disk.
INTERRUPT_MASTER(0x26, interrupt_handler)
// Spurious interrupt (parallel port 1).
INTERRUPT_MASTER(0x27, interrupt_handler)
// CMOS real-time clock (if enabled).
INTERRUPT_SLAVE(0x28, interrupt_handler)
// Free for peripherals.
INTERRUPT_SLAVE(0x29, interrupt_handler)
// Free for peripherals.
INTERRUPT_SLAVE(0x2a, interrupt_handler)
// Free for peripherals.
INTERRUPT_SLAVE(0x2b, interrupt_handler)
// PS/2 mouse.
INTERRUPT_SLAVE(0x2c, interrupt_handler)
// FPU / coprocessor / inter-processor.
INTERRUPT_SLAVE(0x2d, interrupt_handler)
// Primary ATA hard disk.
INTERRUPT_SLAVE(0x2e, interrupt_handler)
// Secondary ATA hard disk.
INTERRUPT_SLAVE(0x2f, interrupt_handler)

.space 512 - (. - interrupts)
INTERRUPT(0x80, syscall_handler)

.space 1024 - (. - interrupts)  // Pad `uintptr_t interrupts[256]`.
