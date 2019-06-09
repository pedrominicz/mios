;;;; bootloader

;;;; the BIOS loads this 512 byte code at address 0x7c00 and jumps to it in
;;;; real mode, as such, this code should be located in the first 512 bytes of
;;;; a boot device

org     0x7c00                  ; the BIOS loads the bootloader at 0x7c00
bits    16                      ; 16-bit real mode

start:
        xor     ax, ax
        mov     ds, ax          ; data segment register
        mov     ss, ax          ; stack segment register
        mov     esp, 0xf000

;;;; initialize serial port 0 with BIOS interrupt 0x14,0, so we can report
;;;; status even without VGA, note that AH was zeroed
        xor     dx, dx
        mov     al, 0b11100011
        int     0x14            ; http://stanislavs.org/helppc/int_14-0.html

        call    puts
        db      'mios', 10, 0
        call    puts
        db      'hello os world', 10, 0

hang:
        cli                     ; clear interrupt flag
        hlt
        jmp     hang

;;;; print null-terminated string located right after the call instruction
puts:
        xchg    [esp], si
        push    ax
.1:     mov     al, [si]
        inc     si
        test    al, al
        jz      .2
        call    putc
        jmp     .1
.2:     pop     ax
        xchg    [esp], si
        ret

;;;; print character in AL and add a carriage return if AL is a new line
putc:
        pusha
.1:     xor     bh, bh
        mov     ah, 0x0e
        int     0x10            ; http://stanislavs.org/helppc/int_10-e.html
        xor     dx, dx
        mov     ah, 0x01
.2:     int     0x14            ; http://stanislavs.org/helppc/int_14-1.html
        test    ah, 0x80
        jz      .3
        mov     word [.2], 0x9090 ; turn `int 0x14` into NOPs
.3:     cmp     al, 0x0a
        jne     .4
        mov     al, 0x0d
        jmp     .1
.4:     popa
        ret

; `$` represents the current address, `$$` represents the address of the first
; instruction, so `$ - $$` is the number of bytes from the start to here
times   510-($-$$) db 0
dw      0xaa55                  ; boot signature
