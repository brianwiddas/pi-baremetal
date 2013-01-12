#ifndef MEMORY_H
#define MEMORY_H

/* Convert a virtual address to a physical one by following the page tables
 * Returns physical address, or 0xffffffff if the virtual address does not map
 */
extern unsigned int mem_v2p(unsigned int);

/* Convert a physical address to a virtual one - essentially, just add
 * 0x80000000 to it
 */
#define mem_p2v(X) (X+0x80000000)

extern void mem_init(void);

#endif /* MEMORY_H */
