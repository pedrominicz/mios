bits 32

section .text
extern interrupt_handle

%macro interrupt_header 0
        push ds
        push es
        push fs
        push gs
        pusha

        mov ax, DATA_SELECTOR
        mov ds, ax
        mov es, ax

        push esp
%endmacro

%macro interrupt_footer 0
        add esp, 4

        popa
        pop gs
        pop fs
        pop es
        pop ds

        add esp, 8
%endmacro

interrupt_common:
        interrupt_header
        call interrupt_handle
        interrupt_footer
        iret

irq_master_interrupt_common:
        interrupt_header
        mov ax, 0x20
        out 0x20, ax            ; Send an end of interrupt to master PIC.
        call interrupt_handle
        interrupt_footer
        iret

irq_slave_interrupt_common:
        interrupt_header
        mov ax, 0x20
        out 0x20, ax            ; Send an end of interrupt to master PIC.
        out 0xa0, ax            ; Send an end of interrupt to slave PIC.
        call interrupt_handle
        interrupt_footer
        iret

DATA_SELECTOR equ 0x0010        ; Must to be equal to `DATA_SELECTOR` in
                                ; "boot.asm".

section .data
align 16
global interrupts
interrupts:                     ; `interrupts` will be accessed from C as
                                ; `uintptr_t interrupts[256]`.

%macro interrupt 1
section .text
interrupt_%1:
        push dword 0            ; Empty error code.
        push dword %1           ; Interrupt number.
        jmp interrupt_common
section .data
dd interrupt_%1
%endmacro

%macro interrupt_error 1
section .text
interrupt_%1:
        push dword %1           ; Note that the CPU has pushed the error code.
        jmp interrupt_common
section .data
dd interrupt_%1
%endmacro

%macro irq_master_interrupt 1
section .text
interrupt_%1:
        push dword 0            ; Empty error code.
        push dword %1
        jmp irq_master_interrupt_common
section .data
dd interrupt_%1
%endmacro

%macro irq_slave_interrupt 1
section .text
interrupt_%1:
        push dword 0
        push dword %1
        jmp irq_slave_interrupt_common
section .data
dd interrupt_%1
%endmacro

interrupt 0x00                  ; Divide error (`div` and `idiv`).
interrupt 0x01                  ; Debug exception.
interrupt 0x02                  ; Non Maskable interrupt (NMI).
interrupt 0x03                  ; Breakpoint (`int 3`).
interrupt 0x04                  ; Overflow (`into`).
interrupt 0x05                  ; Bound range exception (`bound`).
interrupt 0x06                  ; Invalid opcode (`ud2`).
interrupt 0x07                  ; Device not available (`wait` and `fwait`).
interrupt_error 0x08            ; Double fault (error code always 0).
interrupt 0x09                  ; Coprocessor segment overrun (floating-point
                                ; instructions).
interrupt_error 0x0a            ; Invalid Task Statement Segment (TTS).
interrupt_error 0x0b            ; Segment not present.
interrupt_error 0x0c            ; Stack segment fault.
interrupt_error 0x0d            ; General protection.
interrupt_error 0x0e            ; Page fault.
interrupt 0x0f                  ; Intel reserved.

interrupt 0x10                  ; x87 FPU floating-point error.
interrupt_error 0x11            ; Alignment check (error code always 0).
interrupt 0x12                  ; Machine check.
interrupt 0x13                  ; SIMD floating-point exception.
interrupt 0x14                  ; Virtualization exception.
interrupt 0x15                  ; Intel reserved.
interrupt 0x16                  ; Intel reserved.
interrupt 0x17                  ; Intel reserved.
interrupt 0x18                  ; Intel reserved.
interrupt 0x19                  ; Intel reserved.
interrupt 0x1a                  ; Intel reserved.
interrupt 0x1b                  ; Intel reserved.
interrupt 0x1c                  ; Intel reserved.
interrupt 0x1d                  ; Intel reserved.
interrupt 0x1e                  ; Intel reserved.
interrupt 0x1f                  ; Intel reserved.

; From here on interrupts are user-defined.
irq_master_interrupt 0x20       ; Programmable interrupt timer (PIT).
irq_master_interrupt 0x21       ; Keyboard interrupt.
irq_master_interrupt 0x22       ; Cascade (used internally by the programmable
                                ; interrupt controllers, never raised).
irq_master_interrupt 0x23       ; Serial communication port 2 (if enabled).
irq_master_interrupt 0x24       ; Serial communication port 1 (if enabled).
irq_master_interrupt 0x25       ; Parallel port 2 (if enabled).
irq_master_interrupt 0x26       ; Floppy disk.
irq_master_interrupt 0x27       ; Spurious interrupt (parallel port 1).
irq_slave_interrupt 0x28        ; CMOS real-time clock (if enabled).
irq_slave_interrupt 0x29        ; Free for peripherals.
irq_slave_interrupt 0x2a        ; Free for peripherals.
irq_slave_interrupt 0x2b        ; Free for peripherals.
irq_slave_interrupt 0x2c        ; PS/2 mouse.
irq_slave_interrupt 0x2d        ; FPU / coprocessor / inter-processor.
irq_slave_interrupt 0x2e        ; Primary ATA hard disk.
irq_slave_interrupt 0x2f        ; Secondary ATA hard disk.

section .data
times 1024-($-interrupts) db 0  ; Pad `uintptr_t interrupts[256]` with null
                                ; pointers.
