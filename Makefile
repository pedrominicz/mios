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
	@# `e dbg.bpinmaps=false` allows placing breakpoints outside mapped memory.
	r2 -b 32 -d gdb://localhost:1234 -c 'e dbg.bpinmaps=false; db 0x7c00; dc; aaa'

clean:
	rm -f mios.img $(wildcard *.bin) $(OBJ)

.PHONY: all clean debug qemu
