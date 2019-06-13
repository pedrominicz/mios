;;;; Bootloader

;;;; The BIOS loads this 512 byte code at address 0x7c00 and jumps to it in
;;;; real mode, as such, this code should be located in the first 512 bytes of
;;;; a boot device.

org     0x7c00                  ; The BIOS loads the bootloader at 0x7c00.
bits    16                      ; 16-bit real mode.

start:
        xor     ax, ax
        mov     ds, ax          ; Data segment register.
        mov     ss, ax          ; Stack segment register.
        mov     esp, 0xf000

;;;; Initialize serial port 0 with BIOS interrupt 0x14,0, so we can report
;;;; status even without VGA, note that AH was zeroed.
        push    dx
        xor     dx, dx
        mov     al, 0b11100011
        int     0x14            ; http://stanislavs.org/helppc/int_14-0.html
        pop     dx

        mov     si, welcome_msg
        call    puts

;;;; Verify if interrupt 0x13 extensions are available.
        mov     ah, 0x41         ; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=41h:_Check_Extensions_Present
        mov     bx, 0x55aa
        int     0x13
        jc      missing_extension

        xor     dh, dh          ; The BIOS passes in the drive this code was
        mov     ax, dx          ; read from in DL. 0x80 is hard drive 0, 0x81
        call    puti            ; hard drive 1, etc.
        mov     al, 10          ; New line.
        call    putc
        mov     ax, cx          ; Return from interrupt 0x13,41.
        call    puti
        mov     al, 10
        call    putc

        mov     ax, 0x2000
        mov     es, ax
        mov     ebx, 1
        mov     al, 1
        call    read_sector

        cld                     ; Clear direction flag.
;;;; Print the first ten character of sector read.
        mov     cx, 10
.1:     mov     si, cx
        mov     al, [es:si]
        call    putc
        loop    .1
        mov     al, 10
        call    putc

        mov     si, hello
        call    puts
        jmp     hang


;;;; Print null-terminated string at SI.
puts:
        push    ax
.1:     mov     al, [si]
        inc     si
        test    al, al
        jz      .2
        call    putc
        jmp     .1
.2:     pop     ax
        ret

;;;; Print integer in AX followed by a new line.
puti:
        pusha
        mov     cx, 0
        mov     bx, 10
.1:     inc     cx
        mov     dx, 0
        div     bx
        add     dx, '0'
        push    dx
        cmp     ax, 0
        jne     .1
.2:     dec     cx
        pop     ax
        call    putc
        cmp     cx, 0
        jne     .2
        popa
        ret

;;;; Print character in AL and add a carriage return if AL is a new line.
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
        mov     word [.2], 0x9090 ; Turn `int 0x14` into NOPs
.3:     cmp     al, 0x0a
        jne     .4
        mov     al, 0x0d
        jmp     .1
.4:     popa
        ret

;;;; Read sector subroutine. Reads AL sectors from drive number DL into memory
;;;; at ES:0000. Starts reading from sector EBX. Returns with carry set on
;;;; error.
read_sector:
        pusha
        push    dword 0
        push    ebx             ; First sector to read.
        push    es              ; Segment to which sectors will be read.
        push    word 0          ; Offset.
        mov     ah, 0
        push    ax              ; Number of sectors to read.
        push    word 16
        mov     si, sp
        mov     ah, 0x42
        int     0x13            ; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
        jc      read_sector_error
        popa                    ; Pop 16 bytes used as arguments for interrupt.
        popa
        ret

missing_extension:
        mov     si, missing_extension_msg
        call    puts
        jmp     hang

read_sector_error:
        mov     si, read_sector_error_msg
        call    puts
        movzx   ax, ah
        call    puti
        mov     al, '!'
        call    putc
hang:
        cli                     ; Clear interrupt flag.
        hlt
        jmp     hang

;;;; `$` represents the current address, `$$` represents the address of the
;;;; first instruction, so `$ - $$` is the number of bytes from the start to
;;;; here.
times   512 - (enddata-data) - ($-$$) nop

data:

welcome_msg:
db      'mios bootloader.', 10, 0
missing_extension_msg:
db      'BIOS interrupt 0x13 extensions missing!', 0
read_sector_error_msg:
db      'BIOS interrupt 0x13,42 (extended read sectors from drive) error number ', 0
hello:
db      'Hello OS world!', 10, 0

;;;; Boot signature.
dw      0xaa55

enddata:
