/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"
#include "build-in-cbs/internal.h"

#define Base64En
#define CBNAME "Base64En"

int
mkc_cb_base64_encode(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	mkc_data * src;
	char * data;
	int len;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(1);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	src = MKC_CB_ARGV(0);

	len = src->length;
	data = mkc_session_malloc(session, len * 4 / 3 + 4);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	len = encode_base64_core(src->buffer, data, len);
	if (len < 0)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": encode error");
		result->type = MKC_D_VOID;
		return 0;
	}

	result->type = MKC_D_BUF;
	result->buffer = data;
	result->length = len;

	return 0;
}

#define Base64De
#undef CBNAME
#define CBNAME "Base64De"

int
mkc_cb_base64_decode(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	mkc_data * src;
	char * data;
	int len;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(1);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	src = MKC_CB_ARGV(0);

	len = src->length;
	data = mkc_session_malloc(session, len + 1);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	len = decode_base64_len(data, src->buffer, len);
	if (len < 0)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": non Base64 code");
		result->type = MKC_D_VOID;
		return 0;
	}

	if (len < 0)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": decode error");
		result->type = MKC_D_VOID;
		return 0;
	}

	result->type = MKC_D_BUF;
	result->buffer = data;
	result->length = len;

	return 0;
}

/* eof */
