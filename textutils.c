#include "textutils.h"

/* Convert an unsigned value to hex (without the trailing "0x")
 * size = size in bytes (only 1, 2 or 4 permitted)
 */
char *tohex(unsigned int value, unsigned int size)
{
	static char buffer[9];

	unsigned int offset;
	unsigned char ch;

	if(size!=1 && size!=2 && size!=4)
		return "error";

	offset=size*2;

	buffer[offset] = 0;

	while(offset--)
	{
		ch = 48 + (value & 15);
		if(ch>=58)
			ch+=7;

		buffer[offset] = ch;
		value = value >> 4;
	}

	return buffer;
}

/* Convert unsigned value to decimal
 * leading = 0 - no leading spaces/zeroes
 * leading >0 - number of leading zeroes
 * leading <0 - number (when +ve) of leading spaces
 */
char *todec(unsigned int value, int leading)
{
	/* Biggest number is 4294967295 (10 digits) */
	static char buffer[11];
	static char leadchar;

	unsigned int offset = 10;
	unsigned char ch;

	if(leading <0)
	{
		leading = -leading;
		leadchar = ' ';
	}
	else
	{
		leadchar = '0';
	}

	if(leading>10)
		return "error";

	buffer[offset] = 0;

	while(value || (offset == 10))
	{
		offset--;
		leading--;
		ch = 48 + (value % 10);
		buffer[offset] = ch;

		value = value/10;
	}

	while(leading>0)
	{
		offset--;
		leading--;
		buffer[offset] = leadchar;
	}

	return &buffer[offset];
}
