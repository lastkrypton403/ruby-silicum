# Fedora: sudo dnf install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
CROSS      ?= aarch64-linux-gnu-
STRIDE_PX  ?= 1080
CC      = $(CROSS)gcc
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CFLAGS = -O2 -ffreestanding -fno-builtin -fno-pic -mcmodel=tiny \
         -mgeneral-regs-only -mstrict-align -fno-stack-protector \
         -fno-tree-loop-distribute-patterns -Wall -DSTRIDE_PX=$(STRIDE_PX) \
         $(if $(PIXEL_RGBA),-DPIXEL_RGBA=1)

all: Image

start.o: start.S
	$(CC) $(CFLAGS) -c $< -o $@
main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@
fbtest.elf: start.o main.o link.ld
	$(LD) -nostdlib -T link.ld start.o main.o -o $@
Image: fbtest.elf
	$(OBJCOPY) -O binary $< $@
	@ls -l $@

clean:
	rm -f *.o *.elf Image
.PHONY: all clean
