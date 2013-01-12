/* Set up stacks and jump to C code */

.global	_start

_start:
	/* Leave r0-r2 alone as main() is expecting to use the values in
	 * them
	 */

	/* kernel.img is loaded at 0x8000
	 * Below that, 0x4000-0x7fff is the 1MB memory page table, then
	 * 0x3c00-0x3fff is the kernel data coarse page table (see initsys.c)
	 *
	 * Stacks go below it, as follows:
	 *
	 * 0x2c00 - 0x3c00	User/system stack
	 * 0x2800 - 0x2c00	IRQ stack
	 * 0x2400 - 0x2800	Abort stack
	 * 0x2000 - 0x2400	Supervisor (SWI/SVC) stack
	 *
	 * All stacks grow down; decrement then store
	 *
	 * Stack addresses are stored in the stack pointers as
	 * 0x80000000+address, as this means the stack pointer doesn't have
	 * to change when the MMU is turned on (before the MMU is on, accesses
	 * to 0x80000000 go to 0x00000000, and so on). Eventually, the stacks
	 * will be given a proper home
	 */

	mov r4, #0x80000000

	/* SVC stack (for SWIs) at 0x2000 */
	/* The processor appears to start in this mode, but change to it
	 * anyway
	 */
	cps #0x13		/* Change to supervisor (SVC) mode */
	add sp, r4, #0x2400

	/* ABORT stack at 0x2400 */
	cps #0x17		/* Change to Abort mode */
	add sp, r4, #0x2800

	/* IRQ stack at 0x2800 */
	cps #0x12		/* Change to IRQ mode */
	add sp, r4, #0x2c00

	/* System stack at 0x2c00 */
	cps #0x1f		/* Change to system mode */
	add sp, r4, #0x3c00

	/* Stay in system mode from now on */

	/* Turn on unaligned memory access */
	mrc p15, #0, r4, c1, c0, #0
	orr r4, #0x400000	/* 1<22 */
	mcr p15, #0, r4, c1, c0, #0

	/* Jump to memory map initialisation code */
	b initsys

