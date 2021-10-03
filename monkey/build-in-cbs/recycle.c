/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"

STATIC void
__mkc_data_recycle(mkc_session * session, mkc_data * data)	{
	switch (data->type)	{
	case MKC_D_VOID:
		break;
	case MKC_D_INT:
		data->type = MKC_D_VOID;
		break;
	case MKC_D_ID:
		Panic();
		break;
	case MKC_D_BUF:
		SFREE(data->buffer);
		data->type = MKC_D_VOID;
		data->buffer = NULL;
		data->length = 0;
		break;
	default:
		Panic();
		break;
	}
}

#define Recycle

int
mkc_cb_recycle(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * p;
	int argc;
	int i;

	argc = MKC_CB_ARGC;
	for (i = 0; i < argc; i ++)	{
		p = MKC_CB_ARGV(i);
		__mkc_data_recycle(session, p);
	}

	return 0;
}

/* eof */
