/* Various memory utilities */

#include "memutils.h"

/* Clear (set to 0) length bytes of memory starting at address
 */
void memclr(void *address, unsigned int length)
{
	register unsigned int addr = (unsigned int)address;

	/* If the start address is unaligned, fill in the first 1-3 bytes
	 * until it is
	 */
	while((addr & 3) && length)
	{
		*((unsigned char *)addr) = 0;
		addr++;
		length--;
	}

	/* Fill in the remaining 32-bit word-aligned memory locations */
	while(length & 0xfffffffc)
	{
		*((unsigned int *)addr) = 0;
		addr+=4;
		length-=4;
	}

	/* Deal with the remaining 1-3 bytes, if any */
	while(length)
	{
		addr++;
		length--;
		*((unsigned char *)addr) = 0;
	}
}
	
/* Move length bytes from src to dest. Memory areas may overlap
 * Four possibilities:
 * ..[...src...]..[...dest...]..	-- non-overlapping
 * ..[...dest...]..[...src...]..	-- non-overlapping
 * ..[...src..[.]..dest...]..		-- overlapping
 * ..[...dest..[.]..src...]..		-- overlapping
 * The first two can be dealt with by copying from either end of the source
 * block
 * The third (overlapping, source first) by copying from the end of the block
 * back to the start
 * The fourth (overlapping, destination first) by copying from the start of the
 * source block to the end
 *
 * Returns the address of the destination memory area
 */
void *memmove(void *dest, const void *src, unsigned int length)
{
	/* Turn destination and source pointers into integers for easier
	 * calculations
	 */
	register unsigned int d = (unsigned int)dest;
	register unsigned int s = (unsigned int)src;

	if(!length)
		return dest;

	/* Assume the memory blocks are word aligned. Most will be, and the
	 * CPU can deal with unaligned accesses if necessary (as long as it
	 * is configured to do so)
	 */
	if(d>s && d<(s+length))
	{
		/* Destination starts inside source area - work backwards */
		/* If length isn't a multiple of 4 bytes, copy the last 1-3
		 * bytes first
		 */
		while(length & 3)
		{
			length--;
			((unsigned char *)d)[length] = ((unsigned char *)s)[length];
		}

		/* Copy everything else as 32-bit words. If one or both of the
		 * memory areas aren't aligned, this will cause unaligned
		 * reads. Inefficient, but less so than doing everything as
		 * a series of bytes
		 */
		while(length)
		{
			length-=4;
			*((unsigned int *)d+length) = *((unsigned int *)s+length);
		}
	}
	else
	{
		/* Source starts inside destination area - working forwards
		 * is fine - or two areas don't overlap (or they overlap
		 * exactly, but that's an unlikely edge case)
		 *
		 * Copy as much as possible as 32-bit words. See above for
		 * alignment issues
		 */
		while(length & 0xfffffffc)
		{
			*((unsigned int *)d) = *((unsigned int *)s);
			d+=4;
			s+=4;
			length-=4;
		}

		/* Deal with 1-3 remaining bytes, if applicable */
		while(length)
		{
			*((unsigned char *)d) = *((unsigned char *)s);
			d++;
			s++;
			length--;
		}
	}

	return dest;
}
