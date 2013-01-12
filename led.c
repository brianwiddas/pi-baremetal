#include "led.h"
#include "memory.h"

/* Addresses of ARM GPIO devices (with conversion to virtual addresses)
 * See BCM2835 peripherals guide
 */
static volatile unsigned int *gpioGPFSEL1 = (unsigned int *) mem_p2v(0x20200004);
static volatile unsigned int *gpioGPSET0 = (unsigned int *) mem_p2v(0x2020001c);
static volatile unsigned int *gpioGPCLR0 = (unsigned int *) mem_p2v(0x20200028);
static volatile unsigned int *gpioGPLEV0 = (unsigned int *) mem_p2v(0x20200034);

static volatile unsigned int *gpioGPPUD = (unsigned int *) mem_p2v(0x20200094);
static volatile unsigned int *gpioPUDCLK0 = (unsigned int *) mem_p2v(0x20200098);
static volatile unsigned int *gpioPUDCLK1 = (unsigned int *) mem_p2v(0x2020009c);

/* Short delay loop */
static void delay(void)
{
	unsigned int timer=150;

	while(timer--)
		asm ("mov r0, r0");	/* No-op */
}

void led_init(void)
{
	unsigned int var;

	/* Each GPIO has 3 bits which determine its function
	 * GPIO 14 and 16 are in GPFSEL1
	 */
	/* Read current value of GPFSEL1 */
	var = *gpioGPFSEL1;

	/* GPIO 16 = 001 - output */
	var&=~(7<<18);
	var|=1<<18;
	/* GPIO 14 = 000 - input */
	var&=~(7<<12);

	/* Write back updated value */
	*gpioGPFSEL1 = var;

	/* Set up pull-up on GPIO14 */
	/* Enable pull-up control, then wait at least 150 cycles
	 * The delay loop actually waits longer than that
	 */
	*gpioGPPUD = 2;
	delay();

	/* Set the pull up/down clock for pin 14*/
	*gpioPUDCLK0 = 1<<14;
	*gpioPUDCLK1 = 0;
	delay();

	/* Disable pull-up control and reset the clock registers */
	*gpioGPPUD = 0;
	*gpioPUDCLK0 = 0;
	*gpioPUDCLK1 = 0;
}

static unsigned int led_status = 0;

void led_invert(void)
{
	led_status = !led_status;

	if(led_status)
		*gpioGPCLR0 = 1<<16;	/* on */
	else
		*gpioGPSET0 = 1<<16;	/* off */
}


/* Shortish delay loop */
static void shortdelay(void)
{
	unsigned int timer = 3000000;

	while(timer--)
		asm("mov r0, r0");	/* No-op */
}
/* Massively long delay loop */
static void longdelay(void)
{
	unsigned int timer = 10000000;

	while(timer--)
		asm("mov r0, r0");	/* No-op */
}

static void output_n(unsigned int num, unsigned int count)
{

	longdelay();
	longdelay();
	longdelay();
	longdelay();

	/* Flash quickly to indicate start */
	*gpioGPCLR0 = 1<<16;
	shortdelay();
	*gpioGPSET0 = 1<<16;
	shortdelay();
	*gpioGPCLR0 = 1<<16;
	shortdelay();
	*gpioGPSET0 = 1<<16;
	longdelay();
	longdelay();

	while(count--)
	{
		*gpioGPCLR0 = 1<<16;

		if(num & 1)
			longdelay();
		else
			shortdelay();

		*gpioGPSET0 = 1<<16;
		longdelay();
			
		num = num >> 1;
	}
}

void output32(unsigned int num)
{
	output_n(num, 32);
}

void output(unsigned int num)
{
	output_n(num, 8);
}
