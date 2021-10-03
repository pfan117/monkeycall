/* vi: set sw=4 ts=4: */

#include <stdlib.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"
#include "libs/misc.h"

STATIC int
__mkc_rand(void)	{
	int t;
	int r;

	t = __mkc_get_usec();
	r = rand();

	return r + t;
}

#define Rand

/* rand */
/* param: integer: length, integer: flag */
/* return value: string */
int
mkc_cb_rand(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	int r;

	r = __mkc_rand();
	result = MKC_CB_RESULT;
	result->type = MKC_D_INT;
	result->integer = r;

	return 0;
}

/* eof */
