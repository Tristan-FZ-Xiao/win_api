/* Just a debug printer
 */
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "debug.h"

#ifdef T_DEBUG
static unsigned int msglevel = 7;
#else
static unsigned int msglevel = 0;
#endif

void bt_inc_msglevel(void)
{
	msglevel++;
}

void bt_set_msglevel(int level)
{
	msglevel = level;
}

unsigned int bt_get_msglevel(void)
{
	return msglevel;
}

#ifdef T_DEBUG
void print_msg(int level, const char *level_str, const char *func, int line, char *format, ...)
{
	char buffer[2000];
	int  nLen;
	va_list args;

	if (!(level & BTTRACELEVEL))
		return;

	nLen = _snprintf(buffer, 2000, "AUTO_MOB(%s): %s(%d) ", level_str, func, line);
	va_start(args, format);
	nLen = vsprintf(buffer + nLen, format, args);
	va_end(args);

	if (level & msglevel)
		fprintf(stdout, "%s", buffer);

}
#endif