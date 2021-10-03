/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"

#define Stick
#define CBNAME "Stick"

/* stick */
int
mkc_cb_stick(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data_type type;
	mkc_data * result;
	mkc_data * p;
	char * data;
	int offset;
	int argc;
	int len;
	int i;

	result = MKC_CB_RESULT;
	argc = MKC_CB_ARGC;
	if (!argc)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": no input data");
		result->type = MKC_D_VOID;
		return 0;
	}

	p = MKC_CB_ARGV(0);
	type = p->type;
	len = 0;

	for (i = 0; i < argc; i ++)	{
		p = MKC_CB_ARGV(i);
		if (MKC_D_BUF == p->type)	{
			len += p->length;
		}
		else	{
			MKC_CB_ERROR("runtime error: " CBNAME ": is not buffer");
			result->type = MKC_D_VOID;
			return 0;
		}
	}

	if (!len)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": zero data length");
		result->type = MKC_D_VOID;
		return 0;
	}

	data = (char *)mkc_session_malloc(session, len);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	offset = 0;
	for (i = 0; i < argc; i ++)	{
		p = MKC_CB_ARGV(i);
		memcpy(data + offset, p->buffer, p->length);
		offset += p->length;
	}
	result->length = len;
	result->buffer = data;
	result->type = type;

	return 0;
}

/* eof */
