/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"

#define Cut
#define CBNAME "Cut"

/* cut */
/* argument: buffer, start, end */
int
mkc_cb_cut(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	int start;
	int end;
	int len;
	char * data;

	mkc_data * src;
	mkc_data * result;

	MKC_CB_ARGC_CHECK(3);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_INT);
	MKC_CB_ARG_TYPE_CHECK(2, MKC_D_INT);
	result = MKC_CB_RESULT;
	src = MKC_CB_ARGV(0);
	start = (MKC_CB_ARGV(1))->integer;
	end = (MKC_CB_ARGV(2))->integer;

	if (start < 0 || end < 0)	{
		MKC_CB_ERROR(
				"runtime error: " CBNAME ": invalid start end value");
		result->type = MKC_D_VOID;
		return 0;
	}

	if (start >= end)	{
		MKC_CB_ERROR(
				"runtime error: " CBNAME ": end location should be ahead of start");
		result->type = MKC_D_VOID;
		return 0;
	}

	len = end - start + 1;
	data = mkc_session_malloc(session, len);
	if (!data)	{
		MKC_CB_ERROR(
				"runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	memcpy(data, src->buffer + start, len);
	result->type = MKC_D_BUF;
	result->buffer = data;
	result->length = len;

	return 0;
}

/* eof */
