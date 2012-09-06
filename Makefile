
# These values are should work for any Linux where the arm-linux-gnueabihf gcc
# suite is installed, including Raspbian
CC=arm-linux-gnueabihf-gcc
LD=arm-linux-gnueabihf-ld
AS=arm-linux-gnueabihf-as
OBJCOPY=arm-linux-gnueabihf-objcopy

# -mcpu=arm1176jzf-s will cause the assembler to error on ARM opcodes which are
# not valid for that specific processor
ASOPT=--warn -mcpu=arm1176jzf-s

# -marm prevents it building Thumb code
# -mcpu sets the exact CPU in the RPi, for optimised code
# -nostdinc so as not to use any system #include locations
# -ffreestanding to tell the compiler the usual system libraries aren't
# available
CCOPT=-Wall -O6 -nostdinc -ffreestanding -marm -mcpu=arm1176jzf-s

# Object files build from C
COBJS=led.o framebuffer.o mailbox.o main.o textutils.o

all: make.dep kernel.img

clean:
	rm -f make.dep *.o kernel.elf kernel.img

.PHONY: all clean

# Build the list of dependencies included at the bottom
make.dep: *.c *.h
	gcc -M $(COBJS:.o=.c) >make.dep || rm -f make.dep

# Build the assembler bit
start.o: start.s
	$(AS) $(ASOPT) -o start.o start.s

# Make an ELF kernel which loads at 0x8000 (RPi default) from all the object
# files
kernel.elf: linkscript start.o $(COBJS)
	$(LD) -T linkscript -nostdlib -nostartfiles -o kernel.elf start.o $(COBJS)

# Turn the ELF kernel into a binary file. This could be combined with the
# step above, but it's easier to disassemble the ELF version to see why
# something's not working
kernel.img: kernel.elf
	$(OBJCOPY) kernel.elf -O binary kernel.img

# Generic builder for C files
$(COBJS):
	$(CC) $(CCOPT) -c -o $@ $<

include make.dep
