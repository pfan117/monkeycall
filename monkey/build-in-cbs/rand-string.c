/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <stdlib.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"
#include "libs/misc.h"

STATIC int
__mkc_rand_str_rand(void)	{
	int t;
	int r;

	t = __mkc_get_usec();
	r = rand();

	return r + t;
}

STATIC char
__mkc_rand_number_alpha(void)	{
	int r;

	/* range = 26 + 10 = 36 */
	/* 35 = 15 + 15 + 3 + 1 + 1 */
	r = __mkc_rand_str_rand();
	r = (r & 0xf) + ((r & 0xf0) >> 4) + ((r & 0x300) >> 8)
			+ ((r & 0x1000) >> 12) + ((r & 0x10000) >> 16);
	if (r < 10)	{
		return r + '0';
	}
	else	{
		return r - 10 + 'a';
	}
}

STATIC char
__mkc_rand_alpha(void)	{
	int r;

	/* 25 = 15 + 7 + 3 */
	r = __mkc_rand_str_rand();
	r = (r & 0xf) + ((r & 0x70) >> 4) + ((r & 0x300) >> 8);

	return r + 'a';
}

STATIC char
__mkc_rand_number(void)	{
	int r;

	/* 9 = 7 + 1 + 1 */
	r = __mkc_rand_str_rand();
	r = (r & 0x7) + ((r & 0x10) >> 4) + ((r & 0x100) >> 8);

	return r + '0';
}

#define RAND_ALPHA	1
#define RAND_NUMBER	2

STATIC int
__mkc_randstr(char * buffer, int l, int flag)	{
	int i;

	if ((RAND_ALPHA & flag))	{
		if ((RAND_NUMBER & flag))	{
			for (i = 0; i < l; i ++)	{
				buffer[i] = __mkc_rand_number_alpha();
			}
		}
		else	{
			for (i = 0; i < l; i ++)	{
				buffer[i] = __mkc_rand_alpha();
			}
		}
	}
	else	{
		if ((RAND_NUMBER & flag))	{
			for (i = 0; i < l; i ++)	{
				buffer[i] = __mkc_rand_number();
			}
		}
		else	{
			return -1;
		}
	}

	return 0;
}

#define RandStr

/* randstr */
/* param: integer: length, integer: flag */
/* return value: string */
int
mkc_cb_randstr(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	mkc_data * lv;
	mkc_data * fv;
	char * buffer;
	int l;
	int f;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(2);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_INT);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_INT);
	lv = MKC_CB_ARGV(0);
	fv = MKC_CB_ARGV(1);

	l = lv->integer;
	f = fv->integer;

	buffer = (char *)mkc_session_malloc(session, l);
	if (!buffer)	{
		result->type = MKC_D_VOID;
		mkc_session_free(session, buffer);
		return 0;
	}

	if (__mkc_randstr(buffer, l, f))	{
		result->type = MKC_D_VOID;
		mkc_session_free(session, buffer);
		return 0;
	}

	result->type = MKC_D_BUF;
	result->buffer = buffer;
	result->length = l;

	return 0;
}

/* eof */
