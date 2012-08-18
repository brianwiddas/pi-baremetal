
# These values are for ARM cross-compiler on Linux Mint 12 (x86)
CC=arm-linux-gnueabi-gcc-4.6
LD=arm-linux-gnueabi-ld
AS=arm-linux-gnueabi-as
OBJCOPY=arm-linux-gnueabi-objcopy

ASOPT=--warn
CCOPT=-Wall -O6 -nostdinc -ffreestanding -marm -mcpu=arm1176jzf-s
# -marm prevents it building Thumb code
# -mcpu sets the exact CPU in the RPi, for optimised code

# Kernel object list
KOBJS=led.o

all: kernel.img

clean:
	rm -f *.o kernel.elf kernel.img

.PHONY: all clean

start.o: start.s
	$(AS) $(ASOPT) -o start.o start.s

led.o: led.c
	$(CC) $(CCOPT) -c -o led.o led.c

kernel.elf: linkscript start.o $(KOBJS)
	$(LD) -T linkscript -nostdlib -nostartfiles -o kernel.elf start.o $(KOBJS)

kernel.img: kernel.elf
	$(OBJCOPY) kernel.elf -O binary kernel.img
