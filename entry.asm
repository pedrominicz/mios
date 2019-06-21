bits 32

section .start
global start
start:
        cli
        mov esp, 0x8000
        mov ebp, esp

        extern mios_init
        call mios_init          ; Should never return.
hang:
        cli
        hlt
        jmp hang

section .text
interrupt_common:
        push ds
        push es
        push fs
        push gs
        pusha

        mov ax, DATA_SELECTOR
        mov ds, ax
        mov es, ax

        push esp
        extern test_interrupt_handle
        call test_interrupt_handle
        add esp, 4

global interrupt_return
interrupt_return:
        popa
        pop gs
        pop fs
        pop es
        pop ds

        add esp, 8
        iret

DATA_SELECTOR equ 0x0010        ; Must to be equal to `DATA_SELECTOR` in
                                ; boot.asm.

section .data
global interrupts
interrupts:

%macro interrupt 1
section .text
interrupt_%1:
        push dword 0
        push dword %1
        jmp interrupt_common
section .data
dd interrupt_%1
%endmacro

%macro interrupt_error 1
section .text
interrupt_%1:
        push dword %1
        jmp interrupt_common
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
interrupt_error 0x08            ; Double fault.
interrupt 0x09                  ; Coprocessor segment overrun (floating-point
                                ; instructions).
interrupt_error 0x0a            ; Invalid Task Statement Segment (TTS).
interrupt_error 0x0b            ; Segment not present.
interrupt_error 0x0c            ; Stack segment fault.
interrupt_error 0x0d            ; General protection.
interrupt_error 0x0e            ; Page fault.
interrupt 0x0f                  ; Intel reserved.

section .data
times 1024-($-interrupts) db 0
