/* Linker script for kernel */

/* Define the kernel entry point. Needed to prevent ld's -gc-sections option
 * getting rid of everything
 */
ENTRY(_start)

/* Define the memory as starting at 0x8000, and being a megabyte long
 * (shouldn't need to be more than this)
 * "kernel" is where the majority of the kernel is run - it is linked at
 * this address, but loaded into memory after initsys at 0x9000 (probably,
 * as long as initsys is less than 4K)
 */

MEMORY
{
	initsys : org = 0x8000, len = 1M
	kernel : org = 0xf0000000, len = 1M
	data : org = 0xc0000000, len = 1M
}

/* Output kernel code (.text) and read-only data (.rodata) into kernel
 * code space (0xf0000000), read/write data (.data) and initialised-to-zero
 * data (.bss) into kernel data space (0xc0000000), and the initial
 * start-up code/data (in start.s/initsys.c) at 0x8000
 *
 * Make sure start.o comes first, so the entry point is at the start of the
 * file
 *
 * The file ends up looking something like this:
 *
 *	File offset	Load address	Remapped to
 *	0x00000000	0x00008000	0x00008000	start.o/initsys.o
 *	0x00001000	0x00009000	0xf0009000	.text, .rodata
 *	0x00010000	0x00018000	0xc0000000	.data
 *	n/a		0x00020000	0xc0008000	.bss
 *
 * Various pointers are calculated to help initsys map the kernel's virtual
 * memory, and clear .bss
 */

SECTIONS
{
	/* Initial section, loads/runs at 0x8000 */
	.init : {
		start.o(.text* .data* .bss* .rodata*)
		initsys.o (.text* .data* .bss* .rodata*)
	} >initsys

	_highkernelload = ALIGN(4k);

	/* Main kernel text/data. Loads at 0x9000, is remapped to
	 * 0xf0009000 by initsys
	 */
	.text (_highkernelload + 0xf0000000) : AT(_highkernelload) {
		_kstart = ABSOLUTE(.);	/* Address of the kernel */
		*(.text*)
	} >kernel
	.rodata : {
		/* Where the kernel code ends and read-only data begins */
		_krodata = ABSOLUTE(.);

		*(.rodata*)
	} >kernel

	_kend = .;	/* Address of the end of the kernel (RO data) */
	_data_kmem = ALIGN(4k);

	/* Kernel read/write data */
	.data : AT(_data_kmem - 0xf0000000) {
		_datastart = ABSOLUTE(.) ;
		*(.data) }
	>data

	/* Kernel read/write memory - needs to be zeroed by the kernel before
	 * being used
	 */
	.bss : {
		_bssstart = ABSOLUTE(.) ;
		*(.bss)
		*(COMMON)
		_bssend = ABSOLUTE(.) ;
	} >data

	/* Calculate physical addresses of data memory for the kernel */
	_physdatastart = _data_kmem - 0xf0000000;
	_physbssstart = _physdatastart + (_bssstart - _datastart);
	_physbssend = _physdatastart + (_bssend - _datastart);
}
