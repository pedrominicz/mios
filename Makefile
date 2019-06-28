CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
CFLAGS = -m32 -g -O3 -lgcc -pedantic -Wall -Wextra -fno-pic -fno-pie -fno-stack-protector -ffreestanding -nostdlib
LDFLAGS = -m elf_i386

SRC = $(wildcard *.c)
ASM = $(wildcard *.S)
OBJ = $(ASM:.S=.o) $(SRC:.c=.o)

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
	rm -rf mios.iso iso mios.bin $(OBJ)

.PHONY: all clean
