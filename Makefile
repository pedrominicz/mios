TOOLPREFIX = i386-elf-
CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld

CFLAGS = -pedantic -Wall -Wextra -O3
# Generate code for 32-bit ABI.
CFLAGS += -m32
# Produce debugging information in stabs format (if that is supported).
CFLAGS += -gstabs
# No position-independent code
CFLAGS += -fno-pic -fno-pie
# No extra code to check for buffer overflows, such as stack smashing attacks.
CFLAGS += -fno-stack-protector
# Assert that compilation takes place in a freestanding environment.
CFLAGS += -ffreestanding
# Do not use the standard system startup files or libraries when linking.
CFLAGS += -nostdlib

LDFLAGS = -m elf_i386
# All code compiled with `gcc` must be linked with `libgcc`.
LIBGCC := $(shell $(CC) -print-libgcc-file-name)

ASM = $(wildcard *.S)
SRC = $(wildcard *.c)
OBJ = $(ASM:.S=.o) $(SRC:.c=.o)

all: mios.iso

mios.iso: mios.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp mios.bin isodir/boot/mios.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o mios.iso isodir

mios.bin: kernel.ld $(OBJ)
	$(LD) $(LDFLAGS) -T kernel.ld -o mios.bin $(OBJ) $(LIBGCC)

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf isodir
	rm -f mios.iso mios.bin $(OBJ)

.PHONY: all clean
