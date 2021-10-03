/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"

#define MallocBalance

int
mkc_cb_malloc_balance(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	result = MKC_CB_RESULT;
	result->type = MKC_D_INT;
	result->integer = session->mm_count;
	return 0;
}

#define Malloc

int
mkc_cb_malloc(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	void * buffer;
	mkc_data * size;
	mkc_data * result;
	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(1);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_INT);
	size = MKC_CB_ARGV(0);

	buffer = mkc_session_malloc(session, size->integer);
	if (buffer)	{
		result->type = MKC_D_BUF;
		result->buffer = buffer;
		result->length = size->integer;
	}
	else	{
		result->type = MKC_D_VOID;
	}

	return 0;
}

#define Free

int
mkc_cb_free(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * block;
	mkc_data * result;
	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(1);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	block = MKC_CB_ARGV(0);
	mkc_session_free(session, block->buffer);
	result->type = MKC_D_VOID;

	return 0;
}

/* eof */
