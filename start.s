/* Set up stack and jump to C code */

.global	_start

_start:
	/* kernel.img is loaded at 0x8000, so area under that can be the
	 * stack
	 */
	mov sp, #0x8000
	b main

/* Data synchronisation barrier
 * No instruction after the DSB can run until all instructions before it have
 * completed
 */
.global dsb
dsb:
	mov r0, #0
	mcr p15, #0, r0, c7, c10, #4
	mov pc, lr

/* Data memory barrier
 * No memory access after the DMB can run until all memory accesses before it
 * have completed
 */
.global dmb
dmb:
	mov r0, #0
	mcr p15, #0, r0, c7, c10, #5
	mov pc, lr

/* Clean and invalidate entire cache
 * Flush pending writes to main memory
 * Remove all data in data cache
 */
.global flushcache
flushcache:
	mov r0, #0
	mcr p15, #0, r0, c7, c14, #0
	mov pc, lr
