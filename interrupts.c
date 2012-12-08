#include "interrupts.h"

#include "framebuffer.h"
#include "led.h"
#include "memutils.h"
#include "textutils.h"

volatile unsigned int *irqEnable1= (unsigned int *) 0x2000b210;
volatile unsigned int *irqEnable2= (unsigned int *) 0x2000b214;
volatile unsigned int *irqEnableBasic= (unsigned int *) 0x2000b218;

volatile unsigned int *armTimerLoad = (unsigned int *) 0x2000b400;
volatile unsigned int *armTimerValue = (unsigned int *) 0x2000b404;
volatile unsigned int *armTimerControl = (unsigned int *) 0x2000b408;
volatile unsigned int *armTimerIRQClear = (unsigned int *) 0x2000b40c;

__attribute__ ((interrupt ("SWI"))) void interrupt_swi(void)
{
	register unsigned int addr;
	register unsigned int swi_no;
	/* Read link register into addr - contains the address of the
	 * instruction after the SWI
	 */
	asm volatile("mov %[addr], lr" : [addr] "=r" (addr) );

	addr -= 4;
	/* Bottom 24 bits of the SWI instruction are the SWI number */
	swi_no = *((unsigned int *)addr) & 0x00ffffff;

	console_write(COLOUR_PUSH FG_GREEN "SWI call. Address: 0x");
	console_write(tohex(addr, 4));
	console_write("  SWI number ");
	console_write(todec(swi_no, 0));
	console_write(COLOUR_POP "\n");
}

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq(void)
{
	*armTimerIRQClear = 0;
	led_invert();
}

__attribute__ ((interrupt ("ABORT"))) void interrupt_data_abort(void)
{
	register unsigned int addr;
	asm volatile("mov %[addr], lr" : [addr] "=r" (addr) );

	console_write("Data abort!\n");
	console_write("Instuction address: 0x");
	/* addr = lr, but the very start of the abort routine does
	 * sub lr, lr, #4
	 * lr = address of aborted instruction, plus 8
	 */
	console_write(tohex(addr-4, 4));
	console_write("\n");

	/* Routine terminates by returning to LR-4, which is the instruction
	 * after the aborted one
	 * GCC doesn't properly deal with data aborts in its interrupt
	 * handling - no option to return to the failed instruction
	 */
}

__attribute__ ((interrupt ("ABORT"))) void interrupt_prefetch_abort(void)
{
	register unsigned int addr;
	asm volatile("mov %[addr], lr" : [addr] "=r" (addr) );

	console_write("Prefetch abort!\n");
	console_write("Instuction address: 0x");
	/* lr = address of aborted instruction, plus 4
	 * addr = lr, but the very start of the abort routine does
	 * sub lr, lr, #4
	 */
	console_write(tohex(addr, 4));
	console_write("\n");

	/* Don't attempt to return */
	while(1);

	/* Routine terminates by returning to LR-4, which is the instruction
	 * after the aborted one
	 */
}

/* These variable locations are defined by the linker
 *
 * _interrupt_start	where in memory the interrupt vectors should go
 *			(0x00000000, or 0xffff0000 with the right settings)
 * _interrupt_end	address of the word after the interrupt vector code
 * _intvec		location in memory of the compiled interrupt vector
 *			table
 */
extern void *_interrupt_start, *_intvec, *_interrupt_end;

/* Initialise the interrupts table by copying the interrupt vectors into the
 * right place
 *
 * Enable the ARM timer interrupt
 */
void interrupts_init(void)
{
	/* Copy vectors into place */
	memmove(&_interrupt_start, &_intvec,
		(unsigned int)&_interrupt_end -
		(unsigned int)&_interrupt_start);

	/* Turn on interrupts */
	asm volatile("cpsie i");

	/* Use the ARM timer - BCM 2832 peripherals doc, p.196 */
	/* Enable ARM timer IRQ */
	*irqEnableBasic = 0x00000001;

	/* Interrupt every 1024 * 256 (prescaler) timer ticks */
	*armTimerLoad = 0x00000400;

	/* Timer enabled, interrupt enabled, prescale=256, "23 bit" counter
	 * (did they mean 32 bit?)
	 */
	*armTimerControl = 0x000000aa;
}
