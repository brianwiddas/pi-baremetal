/* Set up stack and jump to C code */

.org 0x0

.section intvec, "ax"

.global	_interrupt_start
_interrupt_start:
	b _start;
	b interrupt_undef;
	b interrupt_swi;
	b interrupt_prefetch_abort;
	b interrupt_data_abort;
	nop;	/* Unused vector */
	b interrupt_irq;
	b interrupt_fiq;

interrupt_undef:
interrupt_fiq:
hang:
	b hang;

.global _interrupt_end
_interrupt_end:
