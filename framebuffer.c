#include "framebuffer.h"
#include "barrier.h"
#include "led.h"
#include "mailbox.h"
#include "memory.h"
#include "memutils.h"
#include "textutils.h"

/* SAA5050 (teletext) character definitions */
#include "teletext.h"

/* Framebuffer initialisation failure codes
 * If the FB can't be initialised, one of the following numbers will be
 * flashed on the OK LED
 */

/* Mailbox call to get screen resolution failed */
#define FBFAIL_GET_RESOLUTION		1
/* Mailbox call returned bad resolution */
#define FBFAIL_GOT_INVALID_RESOLUTION	2
/* Mailbox call to setup FB failed */
#define FBFAIL_SETUP_FRAMEBUFFER	3
/* Setup call FB returned an invalid list of response tags */
#define FBFAIL_INVALID_TAGS		4
/* Setup FB call returned an invalid response for the framebuffer tag */
#define FBFAIL_INVALID_TAG_RESPONSE	5
/* Setup FB call returned an invalid address/size */
#define FBFAIL_INVALID_TAG_DATA		6
/* Read FB pitch call returned an invalid response */
#define FBFAIL_INVALID_PITCH_RESPONSE	7
/* Read FB pitch call returned an invalid pitch value */
#define FBFAIL_INVALID_PITCH_DATA	8

/* Character cells are 6x10 */
#define CHARSIZE_X	6
#define CHARSIZE_Y	10

/* Screen parameters set in fb_init() */
static unsigned int screenbase, screensize;
static unsigned int fb_x, fb_y, pitch;
/* Max x/y character cell */
static unsigned int max_x, max_y;

/* Framebuffer initialisation failed. Can't display an error, so flashing
 * the OK LED will have to do
 */
static void fb_fail(unsigned int num)
{
	while(1)
		output(num);
}

/* Initialise the framebuffer */
void fb_init(void)
{
	unsigned int var;
	unsigned int count;
	unsigned int physical_screenbase;

	/* Storage space for the buffer used to pass information between the
	 * CPU and VideoCore
	 * Needs to be aligned to 16 bytes as the bottom 4 bits of the address
	 * passed to VideoCore are used for the mailbox number
	 */
	volatile unsigned int mailbuffer[256] __attribute__((aligned (16)));

	/* Physical memory address of the mailbuffer, for passing to VC */
	unsigned int physical_mb = mem_v2p((unsigned int)mailbuffer);

	/* Get the display size */
	mailbuffer[0] = 8 * 4;		// Total size
	mailbuffer[1] = 0;		// Request
	mailbuffer[2] = 0x40003;	// Display size
	mailbuffer[3] = 8;		// Buffer size
	mailbuffer[4] = 0;		// Request size
	mailbuffer[5] = 0;		// Space for horizontal resolution
	mailbuffer[6] = 0;		// Space for vertical resolution
	mailbuffer[7] = 0;		// End tag

	writemailbox(8, physical_mb);

	var = readmailbox(8);

	/* Valid response in data structure */
	if(mailbuffer[1] != 0x80000000)
		fb_fail(FBFAIL_GET_RESOLUTION);	

	fb_x = mailbuffer[5];
	fb_y = mailbuffer[6];

	/* If both fb_x and fb_y are both zero, assume we're running on the
	 * qemu Raspberry Pi emulation (which doesn't return a screen size
	 * at this point), and request a 640x480 screen
	 */
	if(fb_x==0 && fb_y==0)
	{
		fb_x = 640;
		fb_y = 480;
	}

	if(fb_x==0 || fb_y==0)
		fb_fail(FBFAIL_GOT_INVALID_RESOLUTION);


	/* Set up screen */

	unsigned int c = 1;
	mailbuffer[c++] = 0;		// Request

	mailbuffer[c++] = 0x00048003;	// Tag id (set physical size)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 8;		// Req. + value length (bytes)
	mailbuffer[c++] = fb_x;		// Horizontal resolution
	mailbuffer[c++] = fb_y;		// Vertical resolution

	mailbuffer[c++] = 0x00048004;	// Tag id (set virtual size)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 8;		// Req. + value length (bytes)
	mailbuffer[c++] = fb_x;		// Horizontal resolution
	mailbuffer[c++] = fb_y;		// Vertical resolution

	mailbuffer[c++] = 0x00048005;	// Tag id (set depth)
	mailbuffer[c++] = 4;		// Value buffer size (bytes)
	mailbuffer[c++] = 4;		// Req. + value length (bytes)
	mailbuffer[c++] = 16;		// 16 bpp

	mailbuffer[c++] = 0x00040001;	// Tag id (allocate framebuffer)
	mailbuffer[c++] = 8;		// Value buffer size (bytes)
	mailbuffer[c++] = 4;		// Req. + value length (bytes)
	mailbuffer[c++] = 16;		// Alignment = 16
	mailbuffer[c++] = 0;		// Space for response

	mailbuffer[c++] = 0;		// Terminating tag

	mailbuffer[0] = c*4;		// Buffer size

	writemailbox(8, physical_mb);

	var = readmailbox(8);

	/* Valid response in data structure */
	if(mailbuffer[1] != 0x80000000)
		fb_fail(FBFAIL_SETUP_FRAMEBUFFER);	

	count=2;	/* First tag */
	while((var = mailbuffer[count]))
	{
		if(var == 0x40001)
			break;

		/* Skip to next tag
		 * Advance count by 1 (tag) + 2 (buffer size/value size)
		 *                          + specified buffer size
		*/
		count += 3+(mailbuffer[count+1]>>2);

		if(count>c)
			fb_fail(FBFAIL_INVALID_TAGS);
	}

	/* 8 bytes, plus MSB set to indicate a response */
	if(mailbuffer[count+2] != 0x80000008)
		fb_fail(FBFAIL_INVALID_TAG_RESPONSE);

	/* Framebuffer address/size in response */
	physical_screenbase = mailbuffer[count+3];
	screensize = mailbuffer[count+4];

	if(physical_screenbase == 0 || screensize == 0)
		fb_fail(FBFAIL_INVALID_TAG_DATA);

	/* physical_screenbase is the address of the screen in RAM
	 * screenbase needs to be the screen address in virtual memory
	 */
	screenbase=mem_p2v(physical_screenbase);

	/* Get the framebuffer pitch (bytes per line) */
	mailbuffer[0] = 7 * 4;		// Total size
	mailbuffer[1] = 0;		// Request
	mailbuffer[2] = 0x40008;	// Display size
	mailbuffer[3] = 4;		// Buffer size
	mailbuffer[4] = 0;		// Request size
	mailbuffer[5] = 0;		// Space for pitch
	mailbuffer[6] = 0;		// End tag

	writemailbox(8, physical_mb);

	var = readmailbox(8);

	/* 4 bytes, plus MSB set to indicate a response */
	if(mailbuffer[4] != 0x80000004)
		fb_fail(FBFAIL_INVALID_PITCH_RESPONSE);

	pitch = mailbuffer[5];
	if(pitch == 0)
		fb_fail(FBFAIL_INVALID_PITCH_DATA);

	/* Need to set up max_x/max_y before using console_write */
	max_x = fb_x / CHARSIZE_X;
	max_y = fb_y / CHARSIZE_Y;

	console_write(COLOUR_PUSH BG_BLUE BG_HALF FG_CYAN
			"Framebuffer initialised. Address = 0x");
	console_write(tohex(physical_screenbase, sizeof(physical_screenbase)));
	console_write(" (physical), 0x");
	console_write(tohex(screenbase, sizeof(screenbase)));
	console_write(" (virtual), size = 0x");
	console_write(tohex(screensize, sizeof(screensize)));
	console_write(", resolution = ");
	console_write(todec(fb_x, 0));
	console_write("x");
	console_write(todec(fb_y, 0));
	console_write(COLOUR_POP "\n");
}

/* Current console text cursor position (ie. where the next character will
 * be written
*/
static int consx = 0;
static int consy = 0;

/* Current fg/bg colour */
static unsigned short int fgcolour = 0xffff;
static unsigned short int bgcolour = 0;

/* A small stack to allow temporary colour changes in text */
static unsigned int colour_stack[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static unsigned int colour_sp = 8;

/* Move to a new line, and, if at the bottom of the screen, scroll the
 * framebuffer 1 character row upwards, discarding the top row
 */
static void newline()
{
	unsigned int source;
	/* Number of bytes in a character row */
	register unsigned int rowbytes = CHARSIZE_Y * pitch;

	consx = 0;
	if(consy<(max_y-1))
	{
		consy++;
		return;
	}

	/* Copy a screen's worth of data (minus 1 character row) from the
	 * second row to the first
	 */

	/* Calculate the address to copy the screen data from */
	source = screenbase + rowbytes;
	memmove((void *)screenbase, (void *)source, (max_y-1)*rowbytes);

	/* Clear last line on screen */
	memclr((void *)(screenbase + (max_y-1)*rowbytes), rowbytes);
}

/* Write null-terminated text to the console
 * Supports control characters (see framebuffer.h) for colour and newline
 */
void console_write(char *text)
{
	volatile unsigned short int *ptr;

	unsigned int row, addr;
	int col;
	unsigned char ch;

	/* Double parentheses to silence compiler warnings about
	 * assignments as boolean values
	 */
	while((ch = (unsigned char)*text))
	{
		text++;

		/* Deal with control codes */
		switch(ch)
		{
			case 1: fgcolour = 0b1111100000000000; continue;
			case 2: fgcolour = 0b0000011111100000; continue;
			case 3: fgcolour = 0b0000000000011111; continue;
			case 4: fgcolour = 0b1111111111100000; continue;
			case 5: fgcolour = 0b1111100000011111; continue;
			case 6: fgcolour = 0b0000011111111111; continue;
			case 7: fgcolour = 0b1111111111111111; continue;
			case 8: fgcolour = 0b0000000000000000; continue;
				/* Half brightness */
			case 9: fgcolour = (fgcolour >> 1) & 0b0111101111101111; continue;
			case 10: newline(); continue;
			case 11: /* Colour stack push */
				if(colour_sp)
					colour_sp--;
				colour_stack[colour_sp] =
					fgcolour | (bgcolour<<16);
				continue;
			case 12: /* Colour stack pop */
				fgcolour = colour_stack[colour_sp] & 0xffff;
				bgcolour = colour_stack[colour_sp] >> 16;
				if(colour_sp<8)
					colour_sp++;
				continue;
			case 17: bgcolour = 0b1111100000000000; continue;
			case 18: bgcolour = 0b0000011111100000; continue;
			case 19: bgcolour = 0b0000000000011111; continue;
			case 20: bgcolour = 0b1111111111100000; continue;
			case 21: bgcolour = 0b1111100000011111; continue;
			case 22: bgcolour = 0b0000011111111111; continue;
			case 23: bgcolour = 0b1111111111111111; continue;
			case 24: bgcolour = 0b0000000000000000; continue;
				/* Half brightness */
			case 25: bgcolour = (bgcolour >> 1) & 0b0111101111101111; continue;
		}

		/* Unknown control codes, and anything >127, get turned into
		 * spaces. Anything >=32 <=127 gets 32 subtracted from it to
		 * turn it into a value between 0 and 95, to index into the
		 * character definitions table
		 */
		if(ch<32)
		{
			ch=0;
		}
		else
		{
			if(ch>127)
				ch=0;
			else
				ch-=32;
		}

		/* Plot character onto screen
		 *
		 * CHARSIZE_Y and CHARSIZE_X are the size of the block the
		 * character occupies. The character itself is one pixel
		 * smaller in each direction, and is located in the upper left
		 * of the block
		 */
		for(row=0; row<CHARSIZE_Y; row++)
		{
			addr = (row+consy*CHARSIZE_Y)*pitch + consx*CHARSIZE_X*2;

			for(col=(CHARSIZE_X-2); col>=0; col--)
			{
				ptr = (unsigned short int *)(screenbase+addr);

				addr+=2;

				if(row<(CHARSIZE_Y-1) && (teletext[ch][row] & (1<<col)))
					*ptr = fgcolour;
				else
					*ptr = bgcolour;
			}

			ptr = (unsigned short int *)(screenbase+addr);
			*ptr = bgcolour;
		}

		if(++consx >=max_x)
		{
			newline();
		}
	}
}
