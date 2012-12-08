/* Set up stack and jump to C code */

.global	_start

_start:
	/* Leave r0-r2 alone as main() is expecting to use the values in
	 * them
	 */

	/* kernel.img is loaded at 0x8000, so area under that can be the
	 * stacks
	 *
	 * Set up stacks as follows:
	 *
	 * 0x7000 - 0x8000	User/system stack
	 * 0x6000 - 0x7000	IRQ stack
	 * 0x5000 - 0x6000	Abort stack
	 * 0x4000 - 0x5000	Supervisor (SWI/SVC) stack
	 *
	 * All stacks grow down; decrement then store
	 */

	/* SVC stack (for SWIs) at 0x5000 */
	/* The processor appears to start in this mode, but change to it
	 * anyway
	 */
	cps #0x13		/* Change to supervisor (SVC) mode */
	mov sp, #0x5000

	/* ABORT stack at 0x6000 */
	cps #0x17		/* Change to Abort mode */
	mov sp, #0x6000

	/* IRQ stack at 0x7000 */
	cps #0x12		/* Change to IRQ mode */
	mov sp, #0x7000

	/* System stack at 0x8000 */
	cps #0x1f		/* Change to system mode */
	mov sp, #0x8000

	/* Stay in system mode from now on */

	/* Turn on unaligned memory access */
	mrc p15, #0, r4, c1, c0, #0
	orr r4, #0x400000	/* 1<22 */
	mcr p15, #0, r4, c1, c0, #0

	b main

