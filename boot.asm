;;;; mios bootloader.

;;;; The BIOS loads this 512 byte code at address 0x7c00 and jumps to it in
;;;; real mode, as such, this code should be located in the first 512 bytes of
;;;; a boot device.

org 0x7c00                      ; The BIOS loads the bootloader at 0x7c00.
bits 16                         ; 16-bit real mode.

start:
        cld                     ; Clear direction flag.
        xor ax, ax
        mov ds, ax              ; Data segment register.
        mov ss, ax              ; Stack segment register.
        mov es, ax              ; Extra segment register.
        mov sp, start           ; Stack pointer.
        mov bp, sp              ; Stack base pointer.

        mov [drive_number], dl  ; The BIOS passes the drive the loader was read
                                ; from in `dl`.

;;;; Initialize serial port 0 with BIOS interrupt 0x14,0, so we can report
;;;; status even without VGA. Note that `ah` was zeroed.
        xor dx, dx              ; Zero based serial port number.
        mov al, 0b11100011      ; 9600 baud, no parity, 1 stop bit, 8 data
                                ; bits.
        int 0x14

        mov si, boot_msg
        call puts

        mov di, 0x8000
        mov bx, 1
        call read_sector

        cmp dword [di], 0x464c457f ; ELF magic.
        jne hang

        mov si, [di + 0x1c]     ; The program header table must immediately
        cmp si, 0x34            ; follow the ELF header.
        jne hang
        add si, di

        mov bx, [di + 0x2a]     ; Size of a program header table entry.
        mov cx, [di + 0x2c]     ; Number of segments in program.
        jcxz hang

.loop:
        call read_segment
        add si, bx
        loop .loop

        mov si, success_msg
        call puts
hang:
        cli
        hlt
        jmp hang

;;;; Print null-terminated string at `si`. Note that a carriage return will be
;;;; automatically added upon encountering a new line.
puts:
        pusha
.loop:
        lodsb                   ; Load byte at `ds:si` into `al` and increment
                                ; or decrement `si` according to the direction
                                ; flag.
        cmp al, 0
        je .return
        call putchar
        jmp .loop
.return:
        popa
        ret

;;;; Print character in `al`. Note that a carriage return will be automatically
;;;; added if `al` contains a new line.
putchar:
        pusha
.putchar:
        xor bx, bx              ; Page number and foreground pixel color.
        mov ah, 0x0e
        int 0x10                ; Write character in teletype mode.
        xor dx, dx              ; Zero based serial port number.
        mov ah, 0x01
        int 0x14                ; Send character to communications port.
        cmp al, 10              ; New line.
        jne .return
        mov al, 13              ; Carriage return.
        jmp .putchar
.return:
        popa
        ret

;;;; Read sector subroutine. Reads 8 sectors from drive into `es:di`. Starts
;;;; reading from sector `bx`.
read_sector:
        pusha
        mov dl, [drive_number]
        ; Fill disk address packet (DAP).
        mov [dap_address], di
        mov [dap_segment], es
        mov [dap_start_sector], bx
        mov si, disk_address_packet
        mov ah, 0x42
        int 0x13
        jc hang
        popa
        ret

read_segment:
        pusha

        mov ebx, [si + 0x4]     ; Segment's offset in file in bytes.
        shr ebx, 9              ; Divide by sector size (512 bytes).
        inc ebx                 ; Kernel starts at sector 1.

        mov edx, [si + 0xc]     ; Segment's physical address.
        and edx, 0xfffffe00
        mov di, dx
        shr edx, 4
        and dx, 0xf000
        mov es, dx

        mov ecx, [si + 0x10]    ; Segment's size in file (can be 0).
        jecxz .return
        add ecx, 4095
        shr ecx, 12             ; Divide by the number of bytes `read_sector`
                                ; reads (4096).

        mov al, '.'
.loop:
        call read_sector
        call putchar

        add ebx, 8              ; Adjust `ebx` to the next unread sector.
        add di, 4096            ; Adjust write address.
        jnc .continue
        add dx, 0x1000
        mov es, dx
.continue:
        loop .loop

.return:
        popa
        ret

times 384 - (data_end-data) - ($-$$) nop

data:

drive_number db 0

boot_msg db 'mios bootloader', 10, 0
success_msg db 10, 'success', 0

data_end:

align 4
disk_address_packet:
        db 0x10
        db 0x00
        dw 0x0008               ; Read 8 sectors.
        dap_address dw 0        ; Address to which sectors will be read.
        dap_segment dw 0        ; Segment to which sectors will be read.
        dap_start_sector dw 0   ; First sector to read.
        dw 0x0000
        dd 0x00000000

align 16
gdt:
dq 0x0000000000000000           ; Null segment.
dq 0x00cf9a000000ffff           ; Code segment.
dq 0x00cf92000000ffff           ; Data segment.
gdt_end:

NULL_SELECTOR equ 0x00
CODE_SELECTOR equ 0x08
DATA_SELECTOR equ 0x10

align 16
gdt_descriptor:
dw gdt_end - gdt - 1
dq gdt

times 446 - ($-$$) nop

mbr:
times 64 db 0
mbr_end:

dw 0xaa55                       ; Boot signature.
