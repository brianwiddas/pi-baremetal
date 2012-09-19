/* Set up stack and jump to C code */

.global	_start

_start:
	/* kernel.img is loaded at 0x8000, so area under that can be the
	 * stack
	 */
	mov sp, #0x8000
	b main
