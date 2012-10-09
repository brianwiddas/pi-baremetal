/* Set up stack and jump to C code */

.global	_start

_start:
	/* kernel.img is loaded at 0x8000, so area under that can be the
	 * stack
	 */
	mov sp, #0x8000

	/* Turn on unaligned memory access */
	mrc p15, #0, r4, c1, c0, #0
	orr r4, #0x400000	/* 1<22 */
	mcr p15, #0, r4, c1, c0, #0

	b main

