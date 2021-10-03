/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"

#define Strcmp

int
mkc_cb_strcmp(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	mkc_data * a;
	mkc_data * b;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(2);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_BUF);

	a = MKC_CB_ARGV(0);
	b = MKC_CB_ARGV(1);

	result->type = MKC_D_INT;

	if (a->length == b->length)	{
		if (a->length)	{
			result->integer = memcmp(a->buffer, b->buffer, a->length);
		}
		else	{
			result->integer = 0;
			return 0;
		}
	}
	else	{
		if (a->length > b->length)	{
			result->integer = 1;
			return 0;
		}
		else	{
			result->integer = -1;
			return 0;
		}
	}

	return 0;
}

/* eof */
