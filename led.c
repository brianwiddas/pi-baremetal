
/* Addresses of ARM GPIO devices
 * See BCM2835 peripherals guide
 */
volatile unsigned int *gpioGPFSEL1 = (unsigned int *) 0x20200004;
volatile unsigned int *gpioGPSET0 = (unsigned int *) 0x2020001c;
volatile unsigned int *gpioGPCLR0 = (unsigned int *) 0x20200028;
volatile unsigned int *gpioGPLEV0 = (unsigned int *) 0x20200034;

volatile unsigned int *gpioGPPUD = (unsigned int *) 0x20200094;
volatile unsigned int *gpioPUDCLK0 = (unsigned int *) 0x20200098;
volatile unsigned int *gpioPUDCLK1 = (unsigned int *) 0x2020009c;

/* Short delay loop */
static void delay(void)
{
	unsigned int timer=150;

	while(timer--)
		asm ("mov r0, r0");	/* No-op */
}

void main(void)
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

	/* Loop round reading GPIO14 and setting GPIO16 to match
	 * As the GPIO14 is pulled high and the LED on GPIO16 is
	 * active low, the LED is normally off until GPIO14 is shorted
	 * to ground (such as the convenient ground pin next to it
	 * on the IDC header
	 */
	while(1)
	{
		var = *gpioGPLEV0;

		if(var & (1<<14))
			*gpioGPSET0 = 1<<16;
		else
			*gpioGPCLR0 = 1<<16;
	}
}
