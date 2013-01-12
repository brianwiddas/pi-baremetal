
# These values should work for any Linux where the arm-linux-gnueabihf gcc
# suite is installed, including Raspbian
#
# If they don't, you'll need to manually add the paths for gcc, ld, as and
# objcopy
CC:=arm-linux-gnueabihf-gcc
LD:=$(shell $(CC) -print-prog-name=ld)
AS:=$(shell $(CC) -print-prog-name=as)
OBJCOPY:=$(shell $(CC) -print-prog-name=objcopy)

# Location of libgcc.a (contains ARM AEABI functions such as numeric
# division)
#
# The standard libgcc.a in some (cross-compiling) versions of gcc appears to
# contain Thumb2 instructions, which don't exist on the ARM1176. This causes
# undefined instruction exceptions. If the code is being compiled on a
# Raspberry Pi, it ought to be fine. Otherwise, you may need to use a
# different libgcc.a to the one which comes with your compiler
#
# If compiling on a Broadcom 2708 (a Raspberry Pi), use the libgcc.a
# provided by the compiler. Otherwise, use rpi-libgcc/libgcc.a (this won't
# definitely work on anything other than arm-linux-gnueabihf-gcc 4.6, but
# is likely to)

# If LOCALLIBGCC is set, use the libgcc.a supplied with the compiler
ifdef LOCALLIBGCC
	LIBGCC:=$(shell $(CC) -print-libgcc-file-name)
endif

# If this is a Raspberry Pi (specifically, a Broadcom 2708 SoC), use the
# libgcc.a supplied with the compiler, unless an alternative is set in
# $(LIBGCC)
ifneq ($(shell grep BCM2708 /proc/cpuinfo 2>/dev/null),)
	LIBGCC ?= $(shell $(CC) -print-libgcc-file-name)
endif

# If no alternative is set in $(LIBGCC) by this point, use rpi-libgcc/libgcc.a
LIBGCC ?= rpi-libgcc/libgcc.a


# Assembler options:
# -mcpu=arm1176jzf-s will cause the assembler to error on ARM opcodes which are
# not valid for that specific processor
ASOPT=--warn -mcpu=arm1176jzf-s

# Compiler options:
# -marm prevents gcc building Thumb code
# -mcpu sets the exact CPU in the RPi, for optimised code
# -nostdinc so as not to use any system #include locations
# -ffreestanding to tell the compiler the usual system libraries aren't
# available
CCOPT=-Wall -O6 -nostdinc -ffreestanding -marm -mcpu=arm1176jzf-s

# Object files built from C
COBJS=atags.o divby0.o framebuffer.o initsys.o interrupts.o led.o mailbox.o \
	main.o memory.o memutils.o textutils.o

# Object files build from assembler
ASOBJS=start.o

all: make.dep kernel.img

clean:
	rm -f make.dep *.o kernel.elf kernel.img

.PHONY: all clean

# Build the list of dependencies included at the bottom
make.dep: *.c *.h
	gcc -M $(COBJS:.o=.c) >make.dep

# If gcc -M fails, delete make.dep rather than allowing a half-finished file
# to sit around for the next build
.DELETE_ON_ERROR: make.dep

# Build the assembler bits
start.o: start.s

$(ASOBJS):
	$(AS) $(ASOPT) -o $@ $<

# Make an ELF kernel which loads at 0x8000 (RPi default) from all the object
# files
kernel.elf: linkscript $(ASOBJS) $(COBJS)
	$(LD) -T linkscript -nostdlib -nostartfiles -gc-sections \
		-o kernel.elf \
		$(ASOBJS) $(COBJS) $(LIBGCC)

# Turn the ELF kernel into a binary file. This could be combined with the
# step above, but it's easier to disassemble the ELF version to see why
# something's not working
kernel.img: kernel.elf
	$(OBJCOPY) kernel.elf -O binary kernel.img

# Generic builder for C files
$(COBJS):
	$(CC) $(CCOPT) -c -o $@ $<

# Include the auto-generated make.dep file. The hyphen before "include"
# stops it complaining that the file isn't there.
-include make.dep
