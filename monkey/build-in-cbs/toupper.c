/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"

#define ToUpper

/* parameter: string: input string */
/* return value: undefined */
int
mkc_cb_toupper(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	mkc_data * s;
	char * str;
	char c;
	int l;
	int i;

	MKC_CB_ARGC_CHECK(1);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	s = MKC_CB_ARGV(0);
	result = MKC_CB_RESULT;

	l = s->length;
	str = s->buffer;

	for (i = 0; i < l; i ++)	{
		c = str[i];
		if (c >= 'a' && c <= 'z')	{
			str[i] = c - 'a' + 'A';
		}
	}

	result->type = MKC_D_BUF;
	result->buffer = str;
	result->length = l;

	return 0;
}

/* eof */
