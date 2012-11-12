#ifndef MEMUTILS_H
#define MEMUTILS_H

/* Clear length bytes of memory (set to 0) starting at address */
extern void memclr(void *address, unsigned int length);

/* Move length bytes from src to dest. Memory areas may overlap */
extern void *memmove(void *dest, const void *src, unsigned int length);

#endif	/* MEMUTILS_H */
