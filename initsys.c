/* Virtual memory layout
 *
 * 0x00000000 - 0x7fffffff (0-2GB) = user process memory
 * 0x80000000 - 0xa0ffffff (2GB) = physical memory
 * 	includes peripherals at 0x20000000 - 0x20ffffff
 * 0xc0000000 - 0xefffffff = kernel heap/stack
 * 0xf0000000 - 0xffffffff = kernel code
 *
 * Memory from 0x80000000 upwards won't be accessible to user processes
 */

static unsigned int *initpagetable = (unsigned int * const)0x4000; /* 16K */
static unsigned int *kerneldatatable = (unsigned int * const)0x3c00; /* 1K */

/* initsys calls main() when it's finished, so we need to tell the compiler
 * it's an external symbol
 */
extern void main(void);

/* Memory locations. Defined in linkscript, set during linking */
extern unsigned int _physdatastart, _physbssstart, _physbssend;
extern unsigned int _kstart, _kend;

__attribute__((naked)) void initsys(void)
{
	register unsigned int x;
	register unsigned int pt_addr;
	register unsigned int control;
	register unsigned int *bss;

	/* Save r0-r2 as they contain the start values used by the kernel */
	asm volatile("push {r0, r1, r2}");

	/* The MMU has two translation tables. Table 0 covers the bottom
	 * of the address space, from 0x00000000, and deals with between
	 * 32MB to 4GB of the virtual address space.
	 * Translation table 1 covers the rest of the memory. For now,
	 * both tables are set to the same thing, and, by default, table
	 * 0 manages the entire virtual address space. Later on, the
	 * table 0 register will be pointed to process-specific tables for
	 * each process's virtual memory
	 */

	/* Set up translation table
	 * ARM1176JZF-S manual, 6-39
	 *
	 * The memory is divided in to 4096 1MB sections. Most of these are
	 * unmapped (resulting in prefetch/data aborts), except
	 * 0x8000000-0xa1000000, which are mapped to 0x00000000-0x2a000000
	 * (physical memory and peripherals), and the kernel code and data
	 *
	 * Memory privilege is set by APX/AP bits (3 bits in total)
	 * APX is at bit 15 of the sector definition, while AP are at bits
	 * 10 and 11.
	 *
	 * APX AP            Privileged    Unprivileged
	 *  1  11 (0x8c00) = read-only     read-only
	 *  1  01 (0x8400) = read-only     no access
	 *  0  10 (0x0800) = read-write    read-only
	 *  0  01 (0x0400) = read-write    no-access
	 *
	 * eXecute Never (XN) is at bit 4 (0x10) - sections with this flag
	 * cannot be executes, even by privileged processor modes
	 *
	 * Bits 0 and 1 identify the table entry type
	 * 0 or 3 = translation fault (3 is reserved and shouldn't be used)
	 * 1 = course page table
	 * 2 = section or supersection
	 */
	for(x=0; x<4096; x++)
	{
		if((x >= (0x80000000>>20)) && (x < (0xa1000000>>20)))
		{
			/* Map physical memory to virtual
			 * Read/write for priviledged modes, no execute
			 */
			initpagetable[x] = (x-2048)<<20 | 0x0410 | 2;
		}
		else
		{
			/* No entry in table; translation fault */
			initpagetable[x] = 0;
		}
	}

	/* Map 0x00000000-0x000fffff into virtual memory at the same address.
	 * This is temporary: it's where the code is currently running. Once
	 * initsys has branched tothe kernel at 0xf0000000, it will be removed
	 *
	 * Read/write for privileged mode only
	 */
	initpagetable[0] = 0<<20 | 0x0400 | 2;

	/* Map 1MB at 0xf00000000-0xf00fffff to the kernel code in memory.
	 * Typically, this is loaded at 0x9000, and appears in virtual memory
	 * from 0xf0009000. This means we can use a single 1MB entry to
	 * cover it all (as long as the kernel doesn't get above 1MB...),
	 * reducing the number of TLB entries
	 *
	 * Map as read-only for privileged modes only
	 *
	 * It will also map any free memory surrounding the kernel code (eg.
	 * 0x00000000-0x8fffffff, kernel data). However, as this mapping is
	 * read-only and only available to the kernel, the potential for harm
	 * is minimal
	 */
	initpagetable[3840] = 0<<20 | 0x8400 | 2;

	/* 0xc0000000-0xc00fffff is mapped to physical memory by a course
	 * page table
	 *
	 * This maps the kernel data from where it has been loaded in memory
	 * (after the kernel code, eg. at 0x0001f000) to 0xc0000000
	 * Only memory in use is mapped (to the next 4K). The rest of the
	 * table is unmapped.
	 */
	initpagetable[3072] = 1 | (unsigned int)kerneldatatable;

	/* Populate kerneldatatable - see ARM1176JZF-S manual, 6-40
	 *
	 * APX/AP bits for a page table entry are at bits 9 and 4&5. The
	 * meaning is the same as for a section entry.
	 * 0 01 (0x0010) =  read-write privileded modes, no access otherwise
	 *
	 * bits 0 and 1 determine the page type:
	 * 0 = unmapped, translation fault
	 * 1 = large page (64K)			(XN is bit 15)
	 * 2 = small page (4K), executable	(XN is bit 0)
	 * 3 = small page (4K), not-executable  (XN is bit 0)
	 * 
	 * 256 entries, one for each 4KB in the 1MB covered by the table
	 */
	for(x=0; x<256; x++)
	{
		/* &_physbssend is the physical address of the end of the
		 * kernel data - somewhere between 0x00009000 and 1MB (any
		 * more than that and this code will need rewriting...)
		 */
		if(x <= ((unsigned int)&_physbssend >> 12))
			kerneldatatable[x] = ((unsigned int)&_physdatastart + (x<<12)) | 0x0010 | 2;
		else
			kerneldatatable[x] = 0;
	}

	/* The .bss section is allocated in physical memory, but its contents
	 * (all zeroes) are not loaded in with the kernel.
	 * It needs to be zeroed before it can be used
	 */
	bss = &_physbssstart;
	while(bss<&_physbssend)
	{
		*bss = 0;
		bss++;
	}

	pt_addr = (unsigned int) initpagetable;

	/* Translation table 0 - ARM1176JZF-S manual, 3-57 */
	asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pt_addr));
	/* Translation table 1 */
	asm volatile("mcr p15, 0, %[addr], c2, c0, 1" : : [addr] "r" (pt_addr));
	/* Use translation table 0 for everything, for now */
	asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (0));

	/* Set Domain 0 ACL to "Client", enforcing memory permissions
	 * See ARM1176JZF-S manual, 3-64
	 * Every mapped section/page is in domain 0
	 */
	asm volatile("mcr p15, 0, %[r], c3, c0, 0" : : [r] "r" (0x1));

	/* Read control register */
	asm volatile("mrc p15, 0, %[control], c1, c0, 0" : [control] "=r" (control));
	/* Turn on MMU */
	control |= 1;
	/* Enable ARMv6 MMU features (disable sub-page AP) */
	control |= (1<<23);
	/* Write value back to control register */
	asm volatile("mcr p15, 0, %[control], c1, c0, 0" : : [control] "r" (control));

	/* Set the LR (R14) to the address of main(), then pop off r0-r2
	 * before exiting this function (which doesn't store anything else
	 * on the stack). The "mov lr" comes first as it's impossible to
	 * guarantee the compiler wouldn't use one of r0-r2 for %[main]
	 */
	asm volatile("mov lr, %[main]" : : [main] "r" ((unsigned int)&main) );
	asm volatile("pop {r0, r1, r2}");
	asm volatile("bx lr");
}
