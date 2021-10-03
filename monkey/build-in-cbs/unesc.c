/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"

/* URL parameter operations */
/* translate a character into its hex meaning value */
/* 'a' -> 10 */
/* '3' -> 3 */
/* '0' -> 0 */
/* 'Z' -> -1 */
STATIC int __t09[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1};
STATIC int __tAF[] = {-1, 10, 11, 12, 13, 14, 15, -1};
STATIC int __taf[] = {-1, 10, 11, 12, 13, 14, 15, -1};
STATIC int
__mkc_cb_get_hex_value(char c)	{
	if ((c & 0xf0) == 0x30)	{
		/* '0' - '9' */
		return __t09[c - '0'];
	}
	else if ((c & 0xf8) == 0x40)	{
		/* 'A' - 'F' */
		return __tAF[c - 0x40];	/* 'A' == 65, 64 is for error checking */
	}
	else if ((c & 0xf8) == 0x60)	{
		/* 'a' = 'f' */
		return __taf[c - 0x60];	/* 'a' == 97, 96 is for error checking */
	}
	else	{
		return -1;
	}
}

/* used to translate the string contained in POST packet into normal string */
/* return length of the result data for success */
/* the value of the length could be got by strlen() */
/* return -1 for error */
STATIC int
__mkc_cb_buf_unescape(const char * i, int ilen, char * o, int olen)	{
	int offset = 0;
	int value;
	int v;
	int k;

	for (k = 0; k < ilen;)	{
		if ('%' == i[k])	{
			if (offset >= olen)	{
				return -1;
			}
			if (k + 2 >= ilen)	{
				return -1;
			}
			v = __mkc_cb_get_hex_value(i[k + 1]);
			if (-1 == v)	{
				return -1;
			}
			value = v;
			v = __mkc_cb_get_hex_value(i[k + 2]);
			if (-1 == v)	{
				return -1;
			}
			value = (value << 4) + v;
			o[offset] = value;
			k += 3;
		}
		else if ('+' == i[k])	{
			if (offset >= olen)	{
				return -1;
			}
			o[offset] = ' ';
			k ++;
		}
		else	{
			if (offset >= olen)	{
				return -1;
			}
			o[offset] = i[k];
			k ++;
		}
		offset ++;
	}

	return offset;
}

#define UnEsc
#define CBNAME "UbEsc"

/* unesc */
int
mkc_cb_unesc(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	mkc_data * src;
	char * data;
	int r;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(1);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	src = MKC_CB_ARGV(0);

	if (!src->length)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": zero source string length");
		result->type = MKC_D_VOID;
		return 0;
	}

	data = (char *)mkc_session_malloc(session, src->length);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	r = __mkc_cb_buf_unescape(src->buffer, src->length, data, src->length);
	if (r < 0)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": invalid input string");
		result->type = MKC_D_VOID;
		return 0;
	}

	result->type = MKC_D_BUF;
	result->buffer = data;
	result->length = r;

	return 0;
}

/* eof */
