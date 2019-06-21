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
