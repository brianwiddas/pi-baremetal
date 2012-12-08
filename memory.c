#include "framebuffer.h"
#include "textutils.h"

unsigned int pagetable[4096]	__attribute__ ((aligned (16384)));
unsigned int pagetable0[32]	__attribute__ ((aligned (16384)));

void mem_init(void)
{
	unsigned int x;
	unsigned int pt_addr;
	unsigned int pt0_addr;
	register unsigned int control;

	/* Translation table 1 - covers the entire memory
	 * Map each 1MB virtual page to its physical equivalent (as if the
	 * MMU wasn't enabled), except for the first 64 MB, which return
	 * page faults
	 */
	for(x=0; x<4096; x++)
	{
		pagetable[x] = (x>64)?(x<<20 | 2):0;
	}

	/* Translation table 0 - covers the first 32 MB (see below).
	 * Map each 1MB page to its physical equivalent, except for
	 * 1MB-2MB and 2MB-3MB
	 * Between it and TT1, leaves a hole at 32-64MB
	 */
	for(x=0; x<32; x++)
	{
		pagetable0[x] = x<<20 | 2;
	}

	/* Swap two pages round */
	pagetable0[1] = 2<<20 | 2;	/* 1MB -> 2MB */
	pagetable0[2] = 1<<20 | 2;	/* 2MB -> 1MB */

	pt_addr = (unsigned int) &pagetable;
	pt0_addr = (unsigned int) &pagetable0;

	/* Translation table 0 - ARM1176JZF-S manual, 3-57 */
	asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pt0_addr));
	/* Translation table 1 */
	asm volatile("mcr p15, 0, %[addr], c2, c0, 1" : : [addr] "r" (pt_addr));
	/* Use translation table 0 up to 32MB */
	asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (7));

	/* Set Domain 0 ACL to "Manager" - ARM1176JZF-S manual, 3-64
	 * Every page is in domain 0 at the moment
	 */
	asm volatile("mcr p15, 0, %[r], c3, c0, 0" : : [r] "r" (0x3));

	/* Turn on MMU */
	asm volatile("mrc p15, 0, %[control], c1, c0, 0" : [control] "=r" (control));
	control |= 1;
	asm volatile("mcr p15, 0, %[control], c1, c0, 0" : : [control] "r" (control));
}
