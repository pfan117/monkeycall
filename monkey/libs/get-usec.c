/* vi: set sw=4 ts=4: */

#include <time.h>
#include <sys/time.h>
#include "libs/misc.h"

int
__mkc_get_usec(void)	{
	struct timezone tz;
	struct timeval tv;
	gettimeofday(&tv, &tz);
	return tv.tv_usec;
}

/* eof */
