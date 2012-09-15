#include "framebuffer.h"
#include "textutils.h"

/* This function is called by libgcc.a's division functions when an attempt
 * is made to divide by zero. If this has happened, something has probably
 * gone wrong, so print an error and stop
 */
void raise(void)
{
	console_write(FG_RED "Error: division by zero attempted\n");
	console_write("STOPPED\n");

	while(1);
}
