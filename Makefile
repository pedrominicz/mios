CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
CFLAGS = -m32 -g -O3 -lgcc -Wall -Wextra -fno-pic -fno-pie -fno-stack-protector -ffreestanding -nostdlib
LDFLAGS = -m elf_i386

SRC = kernel.c terminal.c #$(wildcard *.c)
OBJ = head.o $(SRC:.c=.o)

all: mios.iso

mios.iso: mios.bin grub.cfg
	mkdir -p iso/boot/grub
	cp mios.bin iso/boot/mios.bin
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o mios.iso iso

mios.bin: kernel.ld $(OBJ)
	$(LD) $(LDFLAGS) -T kernel.ld -o mios.bin $(OBJ)

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf mios.iso iso $(wildcard *.bin) $(OBJ)

.PHONY: all clean
