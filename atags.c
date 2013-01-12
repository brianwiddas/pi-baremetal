#include "atags.h"

#include "framebuffer.h"
#include "memory.h"
#include "textutils.h"

static void print_atag_core(struct atag_core *data)
{
	if(data->header.size == 5)
	{
		console_write("  Flags: 0x");
		console_write(tohex(data->flags, 4));
		console_write(", pagesize: 0x");
		console_write(tohex(data->pagesize, 4));
		console_write(", root device: 0x");
		console_write(tohex(data->rootdevice, 4));
		console_write("\n");
	}
	else
		console_write("  No additional data\n");
}

static void print_atag_mem(struct atag_mem *data)
{
	console_write("  Address: 0x");
	console_write(tohex(data->address, 4));
	console_write(" - 0x");
	console_write(tohex(data->address+data->size-1, 4));
	console_write(" (");
	console_write(todec(data->size, 0));
	console_write(" bytes)\n");
}

static void print_atag_ramdisk(struct atag_ramdisk *data)
{
	console_write("  Flags: 0x");
	console_write(tohex(data->flags, 4));
	console_write(", size: 0x");
	console_write(tohex(data->size, 4));
	console_write(", start block: 0x");
	console_write(tohex(data->start, 4));
	console_write("\n");
}

static void print_atag_initrd2(struct atag_initrd2 *data)
{
	console_write("  Address: 0x");
	console_write(tohex(data->address, 4));
	console_write(" - 0x");
	console_write(tohex(data->address+data->size-1, 4));
	console_write(" (");
	console_write(todec(data->size, 0));
	console_write(" bytes)\n");
}

static void print_atag_serial(struct atag_serial *data)
{
	console_write("  Serial number: 0x");
	console_write(tohex(data->high, 4));
	console_write(tohex(data->low, 4));
	console_write("\n");
}

static void print_atag_revision(struct atag_revision *data)
{
	console_write("  Board revision: ");
	console_write(todec(data->revision, 0));
	console_write("\n");
}

static void print_atag_videolfb(struct atag_videolfb *data)
{
	console_write("  Size: ");
	console_write(todec(data->width, 0));
	console_write("x");
	console_write(todec(data->height, 0));
	console_write(", depth: ");
	console_write(todec(data->depth, 0));
	console_write("bpp, linelength: ");
	console_write(todec(data->linelength, 0));

	console_write("\n  Address: 0x");
	console_write(tohex(data->address, 4));
	console_write(" - 0x");
	console_write(tohex(data->address+data->size-1, 4));
	console_write(" (");
	console_write(todec(data->size, 0));
	console_write(" bytes)\n");

	console_write("  Pos/size: R ");
	console_write(todec(data->redpos, 0));
	console_write("/");
	console_write(todec(data->redsize, 0));

	console_write(", G ");
	console_write(todec(data->greenpos, 0));
	console_write("/");
	console_write(todec(data->greensize, 0));

	console_write(", B ");
	console_write(todec(data->bluepos, 0));
	console_write("/");
	console_write(todec(data->bluesize, 0));

	console_write(", reserved ");
	console_write(todec(data->reservedpos, 0));
	console_write("/");
	console_write(todec(data->reservedsize, 0));

	console_write("\n");
}

static void print_atag_cmdline(struct atag_cmdline *data)
{
	console_write("  \"");
	console_write(&data->commandline);
	console_write("\"\n");
}

void print_atags(unsigned int address)
{
	/* Use virtual mapped physical memory to access the ATAGs */
	struct atag_header *atags = (struct atag_header *) mem_p2v(address);
	unsigned int tag;

	console_write(COLOUR_PUSH BG_GREEN BG_HALF "Reading ATAGs\n\n" COLOUR_POP);

	do
	{
		tag = atags->tag;
		console_write("ATAG at address 0x");
		console_write(tohex((unsigned int) atags, 4));
		console_write(" is 0x");
		console_write(tohex(tag, 4));

		switch(tag)
		{
			case 0:
				console_write(" (ATAG_NONE)\n\n");
				break;
			case ATAG_CORE:
				console_write(" (ATAG_CORE)\n");
				print_atag_core((struct atag_core *)atags);
				break;
			case ATAG_MEM:
				console_write(" (ATAG_MEM)\n");
				print_atag_mem((struct atag_mem *)atags);
				break;
			case ATAG_VIDEOTEXT:
				console_write(" (ATAG_VIDEOTEXT)\n");
				break;
			case ATAG_RAMDISK:
				console_write(" (ATAG_RAMDISK)\n");
				print_atag_ramdisk((struct atag_ramdisk *)atags);
				break;
			case ATAG_INITRD2:
				console_write(" (ATAG_INITRD2)\n");
				print_atag_initrd2((struct atag_initrd2 *)atags);
				break;
			case ATAG_SERIAL:
				console_write(" (ATAG_SERIAL)\n");
				print_atag_serial((struct atag_serial *)atags);
				break;
			case ATAG_REVISION:
				console_write(" (ATAG_REVISION)\n");
				print_atag_revision((struct atag_revision *)atags);
				break;
			case ATAG_VIDEOLFB:
				console_write(" (ATAG_VIDEOLFB)\n");
				print_atag_videolfb((struct atag_videolfb *)atags);
				break;
			case ATAG_CMDLINE:
				console_write(" (ATAG_CMDLINE)\n");
				print_atag_cmdline((struct atag_cmdline *)atags);
				break;
			default:
				console_write(" (UNKNOWN)\n");
				return;
		}

		atags = (struct atag_header *)((unsigned int)atags + (atags->size * 4));
	} while(tag);
}
