CFLAGS = -m32 -fno-pic -fno-pie -ffreestanding -nostdlib -lgcc -Wall -Wextra
# -s, --strip-all: Omit all symbol information from the output file.
LDFLAGS = -m elf_i386 -s

OBJ = entry.o kernel.o

all: mios.img

mios.img: boot.bin kernel.bin
	dd if=/dev/zero of=$@ count=10000 2>/dev/null
	dd if=boot.bin of=$@ conv=notrunc 2>/dev/null
	dd if=kernel.bin of=$@ seek=1 conv=notrunc 2>/dev/null

boot.bin: boot.asm
	nasm -f bin $< -o $@

kernel.bin: kernel.ld $(OBJ)
	$(LD) $(LDFLAGS) -T $< -o $@ $(OBJ)

entry.o: entry.asm
	nasm -f elf $< -o $@

qemu: mios.img
	qemu-system-i386 -drive file=$<,index=0,media=disk,format=raw

debug: mios.img
	screen -d -m qemu-system-i386 -s -S -drive file=$<,index=0,media=disk,format=raw
	@sleep 1 # Give time for QEMU to start.
	@echo 'Starting radare2 in 16-bit mode. Remember to `e asm.bits=32` when you'
	@echo 'reach 32-bit code.'
	r2 -b 16 -d gdb://localhost:1234 -c 'db 0x7c00' -c 'dc' -c 'aaa'

clean:
	rm -f mios.img $(wildcard *.bin) $(OBJ)

.PHONY: all clean debug qemu
