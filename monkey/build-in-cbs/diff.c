/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"

#define Diff

/* diff */
int
mkc_cb_diff(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	mkc_data * a;
	mkc_data * b;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(2);
	a = MKC_CB_ARGV(0);
	b = MKC_CB_ARGV(1);

	if (a->type != b->type)	{
		goto __return_one;
	}

	switch(a->type)	{
	case MKC_D_VOID:
		goto __return_zero;
	case MKC_D_INT:
		if (a->integer == b->integer)	{
			goto __return_zero;
		}
		else	{
			goto __return_one;
		}
	case MKC_D_ID:
		Panic();
		return -1;
	case MKC_D_BUF:
		if (a->length == b->length)	{
			if (!a->length)	{
				goto __return_zero;
			}
			else	{
				if (memcmp(a->buffer, b->buffer, a->length))	{
					goto __return_one;
				}
				else	{
					goto __return_zero;
				}
			}
		}
		else	{
			goto __return_one;
		}
	case MKC_D_MAX:
	default:
		Panic();
		return -1;
	}

__return_zero:
	result->type = MKC_D_INT;
	result->integer = 0;
	return 0;

__return_one:
	result->type = MKC_D_INT;
	result->integer = 1;
	return 0;
}

/* eof */
