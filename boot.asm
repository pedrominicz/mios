;;;; mios bootloader.

;;;; The BIOS loads this 512 byte code at address 0x7c00 and jumps to it in
;;;; real mode, as such, this code should be located in the first 512 bytes of
;;;; a boot device.

org 0x7c00                      ; The BIOS loads the bootloader at 0x7c00.

bits 16                         ; 16-bit Real Mode.
start:
        cli                     ; Clear interrupt flag and never set it again.
        cld                     ; Clear direction flag.
        xor ax, ax
        mov ds, ax              ; Data segment register.
        mov ss, ax              ; Stack segment register.
        mov es, ax              ; Extra segment register.
        mov sp, start           ; Stack pointer.
        mov bp, sp              ; Stack base pointer.

.1:     in al, 0x64             ; Wait while busy.
        test al, 2
        jnz .1
        mov al, 0xd1
        out 0x64, al            ; Write to output port.

.2:     in al, 0x64
        test al, 2
        jnz .2
        mov al, 0xdf
        out 0x60, al            ; Enable A20.

        lgdt [gdt_descriptor]   ; Load the Global Descriptor Table (GDT).
        mov eax, cr0            ; Set Protection Enable bit in `cr0`
        or al, 1
        mov cr0, eax

        jmp CODE_SEGMENT:start32 ; Long jump: resets code segment.

bits 32                         ; 32-bit Protected Mode.
start32:
        mov ax, DATA_SEGMENT
        mov ds, ax
        mov ss, ax
        mov es, ax

        xor ax, ax
        mov fs, ax              ; Extra segment register #2.
        mov gs, ax              ; Extra segment register #3.

        mov edi, 0x8000
        mov eax, 1
        call read_sector        ; Note that only the first 512 bytes of the ELF
                                ; are read, so a maximum of 14 program header
                                ; entries may be read.

        cmp dword [edi], 0x464c457f ; ELF magic.
        jne hang

        mov esi, [edi + 0x1c]   ; The program header table must immediately
        cmp esi, 0x34           ; follow the ELF header.
        jne hang
        add esi, edi            ; Make `ds:esi` point to the program header
                                ; table.

        cmp word [edi + 0x2a], 0x20 ; Size of a program header table entry.
        jne hang

        movzx ecx, word [edi + 0x2c] ; Number of segments in program.
        jcxz hang
        cmp ecx, 14
        jbe .loop
        mov ecx, 14

.loop:
        call read_segment
        add esi, 0x20           ; Adjust `ds:esi` to the next entry in the
                                ; program header table.
        loop .loop

        jmp [edi + 0x18]        ; ELF entry point.

;;;; Note that this is here because `jcxz hang` is a short jump.
hang:
        cli
        hlt
        jmp hang

;;;; Wait disk to be ready. Note that `al` and `dx` are not preserved.
wait_disk:
        mov dx, 0x01f7          ; Status and command register.
.loop:
        in al, dx
        and al, 0b11000000      ; Mask the "busy" and "drive ready" bits.
        cmp al, 0b01000000      ; Drive not busy and ready.
        jne .loop
        ret

;;;; Read sector `eax` into `es:edi`.
read_sector:
        pusha
        push eax
        call wait_disk          ; `wait_disk` modifies `eax`.
        mov dx, 0x01f2
        mov al, 1
        out dx, ax              ; Read 8 sectors.
        pop eax

        mov dx, 0x01f3
        out dx, al              ; Bits 0-7 of sector number.
        shr eax, 8
        mov dx, 0x01f4
        out dx, al              ; Bits 8-15 of sector number.
        shr eax, 8
        mov dx, 0x01f5
        out dx, al              ; Bits 16-23 of sector number.
        shr eax, 8
        or al, 0b11100000       ; Drive mask: use logical block addressing and
                                ; select the master drive. Note that bits 7 and
                                ; 5 are always 1.
        mov dx, 0x01f6          ; Bits 24-27 of sector number.
        out dx, al

        mov dx, 0x01f7
        mov al, 0x20
        out dx, al              ; Read with retry.
        call wait_disk

        mov dx, 0x01f0
        mov ecx, 128
        rep insd                ; Repeat input double word from port `dx` into
                                ; `es:edi`.
        popa
        ret

;;;; Read segment from ELF subroutine. Reads the segment described by the
;;;; program header table entry at `ds:esi` into its physical address.
read_segment:
        pusha
        mov eax, [esi + 0x4]    ; Segment's offset in file in bytes.
        shr eax, 9              ; Translate from bytes to sectors (divide by
                                ; sector size).
        inc eax                 ; Kernel starts at sector 1.

        mov edi, [esi + 0xc]    ; Segment's physical address.
        push edi
        and edi, 0xfffffe00     ; Round down to sector boundary.

        mov ecx, [esi + 0x10]   ; Segment's size in file (can be 0).
        jecxz .clear
        ; Adjust counter to the number of `read_sector` calls necessary to read
        ; the whole segment.
        add ecx, 511
        shr ecx, 9

.loop:
        call read_sector
        inc eax                 ; Adjust `eax` to the next unread sector.
        add edi, 512            ; Adjust write address.
        loop .loop

.clear:
        mov eax, [esi + 0x10]
        mov ecx, [esi + 0x14]   ; Segment's size in memory.
        sub ecx, eax
        pop edi
        add edi, eax
        inc edi
        xor al, al
        rep stosb               ; Repeat store `al` at `es:edi`.
        popa
        ret

;;;; Temporary Global Descriptor Table.
align 16
gdt:
dq 0x0000000000000000           ; Null segment.
dq 0x00cf9a000000ffff           ; Code segment.
dq 0x00cf92000000ffff           ; Data segment.
gdt_end:

CODE_SEGMENT equ 0x0008
DATA_SEGMENT equ 0x0010

align 16
gdt_descriptor:
dw gdt_end - gdt - 1
dq gdt

;;;; Make sure the master boot record and the boot signature are in the right
;;;; offset. `$` represents the current address, `$$` represents the address of
;;;; the first instruction, so `$ - $$` is the number of bytes from the start
;;;; to here.
times 446 - ($-$$) nop

mbr:
times 64 db 0
mbr_end:

dw 0xaa55                       ; Boot signature.
