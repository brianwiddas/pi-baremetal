#include "led.h"
#include "barrier.h"
#include "framebuffer.h"
#include "mailbox.h"
#include "textutils.h"

/* Use some free memory in the area below the kernel/stack */
#define BUFFER_ADDRESS  0x1000

/* Pull various bits of information from the VideoCore and display it on
 * screen
 */
void mailboxtest(void)
{
	volatile unsigned int *ptr = (unsigned int *) BUFFER_ADDRESS;
	unsigned int count, var;
	unsigned int mem, size;

	ptr[0] = 8 * 4;		// Total size
	ptr[1] = 0;		// Request

	ptr[2] = 0x40003;	// Display size
	ptr[3] = 8;		// Buffer size
	ptr[4] = 0;		// Request size
	ptr[5] = 0;
	ptr[6] = 0;

	ptr[7] = 0;

	writemailbox(8, BUFFER_ADDRESS);

	var = readmailbox(8);

	/*
	console_write(FG_GREEN "\n\nReading mailbox 8\n" FG_WHITE);

	console_write(COLOUR_PUSH BG_GREEN BG_HALF "Result = 0x");
	console_write(tohex(var, sizeof(var)));
	console_write(COLOUR_POP "\n");

	for(count=0; count<=7; count++)
	{
		console_write("Value ");
		console_write(todec(count, 2));
		console_write(" = 0x");
		console_write(tohex(ptr[count], 4));
		console_write("\n");
	}

	*/

	console_write(COLOUR_PUSH FG_CYAN "Display resolution: " BG_WHITE BG_HALF BG_HALF);
	console_write(todec(ptr[5], 0));
	console_write("x");
	console_write(todec(ptr[6], 0));
	console_write(COLOUR_POP "\n");


	ptr[0] = 200 * 4;	// Total size
	ptr[1] = 0;		// Request

	ptr[2] = 0x50001;	// Command line
	ptr[3] = 195*4;		// Buffer size
	ptr[4] = 0;		// Request size
	
	for(count=5; count<200; count++)
		ptr[count] = 0;

	writemailbox(8, BUFFER_ADDRESS);

	var = readmailbox(8);

	console_write("\n" COLOUR_PUSH FG_RED "Kernel command line: " COLOUR_PUSH BG_RED BG_HALF BG_HALF);
	console_write((char *)(BUFFER_ADDRESS + 20));
	console_write(COLOUR_POP COLOUR_POP "\n\n");


	ptr[0] = 13 * 4;	// Total size
	ptr[1] = 0;		// Request

	ptr[2] = 0x10005;	// ARM memory
	ptr[3] = 8;		// Buffer size
	ptr[4] = 0;		// Request size
	ptr[5] = 0;
	ptr[6] = 0;

	ptr[7] = 0x10006;	// VideoCore memory
	ptr[8] = 8;		// Buffer size
	ptr[9] = 0;		// Request size
	ptr[10] = 0;
	ptr[11] = 0;

	ptr[12] = 0;

	writemailbox(8, BUFFER_ADDRESS);

	var = readmailbox(8);

	mem = ptr[5];
	size = ptr[6];
	var = size / (1024*1024);

	console_write(COLOUR_PUSH FG_YELLOW "ARM memory: " BG_YELLOW BG_HALF BG_HALF "0x");
	console_write(tohex(mem, 4));
	console_write(" - 0x");
	console_write(tohex(mem+size-1, 4));
	console_write(" (");
	console_write(todec(size, 0));
	/* ] appears as an arrow in the SAA5050 character set */
	console_write(" bytes ] ");
	console_write(todec(var, 0));
	console_write(" megabytes)" COLOUR_POP "\n");

	mem = ptr[10];
	size = ptr[11];
	var = size / (1024*1024);
	console_write(COLOUR_PUSH FG_YELLOW "VC memory:  " BG_YELLOW BG_HALF BG_HALF "0x");
	console_write(tohex(mem, 4));
	console_write(" - 0x");
	console_write(tohex(mem+size-1, 4));
	console_write(" (");
	console_write(todec(size, 0));
	console_write(" bytes ] ");
	console_write(todec(var, 0));
	console_write(" megabytes)" COLOUR_POP "\n");
}

/* Main routine - called directly from start.s */
void main(void)
{
	led_init();
	fb_init();

	/* Say hello */
	console_write("Pi-Baremetal booted\n\n");

	/* Read in some system data */
	mailboxtest();

	console_write(FG_GREEN "\nOK LED reflects state of !GPIO14");

	/* Make the LED do something in a never-ending while loop */
	led_gpio14();
}
