/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"

#define Dup
#define CBNAME "Dup"

int
mkc_cb_dup(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	mkc_data * src;
	char * data;
	int len;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(1);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);

	src = MKC_CB_ARGV(0);
	len = src->length;

	if (!len)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": zero source data length");
		result->type = MKC_D_VOID;
		return 0;
	}

	data = (char *)mkc_session_malloc(session, len);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	memcpy(data, src->buffer, len);
	result->type = MKC_D_BUF;
	result->buffer = data;
	result->length = len;

	return 0;
}

/* eof */
