org     0x7c00                  ; the BIOS loads the bootloader at 0x7c00
bits    16

start:
        xor     ax, ax
        mov     ds, ax
        cld                     ; clear direction flag, when the DF is 0,
                                ; string operations increment the index
                                ; registers
        mov     si, msg
printloop:
        lodsb                   ; load string byte: load byte at dress DS:SI
                                ; into AL and increments/decrements SI
        or      al, al
        jz      hang            ; jump if AL is zero
        mov     ah, 0x0e
        int     0x10            ; BIOS video interrupt
        jmp     printloop
hang:
        cli                     ; clear interrupt flag
        hlt

msg     db      'hello os world', 0

; `$` represents the current address, `$$` represents the address of the first
; instruction, so `$ - $$` is the number of bytes from the start to here
times   510-($-$$) db 0
dw      0xaa55                  ; boot signature
