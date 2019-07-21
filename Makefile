TOOLPREFIX = i386-elf-
CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
# C preprocessor.
CPP = $(TOOLPREFIX)cpp

CFLAGS = -pedantic -Wall -Wextra -Werror -O3 -DDEBUG
# Generate code for 32-bit ABI.
CFLAGS += -m32
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

mios.bin: kernel.lds.i $(OBJ)
	$(LD) $(LDFLAGS) -T kernel.lds.i -o mios.bin $(OBJ) $(LIBGCC)

kernel.lds.i: kernel.lds
	$(CPP) -D__ASSEMBLER__ -P kernel.lds -o kernel.lds.i

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf isodir
	rm -f mios.iso mios.bin kernel.lds.i $(OBJ)

.PHONY: all clean
