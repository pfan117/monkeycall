/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"

#define IsInteger

int
mkc_cb_is_integer(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	mkc_data * arg;

	MKC_CB_ARGC_CHECK(1);
	arg = MKC_CB_ARGV(0);
	result = MKC_CB_RESULT;

	result->type = MKC_D_INT;
	if (MKC_D_INT == arg->type)	{
		result->integer = 1;
	}
	else	{
		result->integer = 0;
	}

	return 0;
}

#define IsBuffer

int
mkc_cb_is_buffer(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	mkc_data * arg;

	MKC_CB_ARGC_CHECK(1);
	arg = MKC_CB_ARGV(0);
	result = MKC_CB_RESULT;

	result->type = MKC_D_INT;
	if (MKC_D_BUF == arg->type)	{
		result->integer = 1;
	}
	else	{
		result->integer = 0;
	}

	return 0;
}

#define IsVoid

int
mkc_cb_is_void(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	mkc_data * arg;

	MKC_CB_ARGC_CHECK(1);
	arg = MKC_CB_ARGV(0);
	result = MKC_CB_RESULT;

	result->type = MKC_D_INT;
	if (MKC_D_VOID == arg->type)	{
		result->integer = 1;
	}
	else	{
		result->integer = 0;
	}

	return 0;
}

/* eof */
