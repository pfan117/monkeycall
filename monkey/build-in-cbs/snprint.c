/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"
#include "include/debug.h"

#define SNPRINT(__fmt__, ...)	do	{\
	r = snprintf(buffer + offset, buffer_len - offset, __fmt__, ##__VA_ARGS__);\
	if (r <= 0 || r >= (buffer_len - offset))	{\
		MKC_CB_ERROR("runtime error: snprint: buffer too small");\
		goto __exx;\
	}\
	else	{\
		offset += r;\
	}\
}while(0)

#define BUFPRINT	do	{\
	if (arg->length < buffer_len - offset)	{\
		memcpy(buffer + offset, arg->buffer, arg->length);\
		offset += arg->length;\
	}\
	else	{\
		MKC_CB_ERROR("runtime error: snprint: buffer too small");\
		goto __exx;\
	}\
}while(0)

STATIC int
__mkc_cb_data_snprint_auto(
		mkc_session * session
		, mkc_cb_stack_frame * mkc_cb_stack
		, char * buffer, int buffer_len, mkc_data * arg)
{
	int offset = 0;
	int r;

	switch(arg->type)	{
	case MKC_D_VOID:
		return 0;
	case MKC_D_INT:
		SNPRINT("%d", arg->integer);
		return offset;
	case MKC_D_ID:
		Panic();
		return -1;
	case MKC_D_BUF:
		BUFPRINT;
		return offset;
	case MKC_D_MAX:
	default:
		Panic();
		return -1;
	}

__exx:

	return -1;
}

enum	{ S_NORMAL, S_PERCENT, S_SLASH };

#define Snprint

int
mkc_cb_snprint(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	const char * fmt;
	mkc_data * arg;
	int buffer_len;
	char * buffer;
	int offset;
	int length;
	int argc;
	int argi;
	char c;
	int s;
	int i;
	int r;

	result = MKC_CB_RESULT;
	argc = MKC_CB_ARGC;
	if (argc < 2)	{
		MKC_CB_ERROR("runtime error: snprint: parameter less than 2");
		goto __exx;
	}

	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_BUF);

	argi = 2;
	s = S_NORMAL;
	buffer = MKC_CB_ARGV(0)->buffer;
	buffer_len = MKC_CB_ARGV(0)->length;
	offset = 0;
	fmt = MKC_CB_ARGV(1)->buffer;
	length = MKC_CB_ARGV(1)->length;

	for (i = 0; i < length; i ++)	{
		c = fmt[i];
		if (S_NORMAL == s)	{
			if ('%' == c)	{
				s = S_PERCENT;
			}
			else if ('\\' == c)	{
				s = S_SLASH;
			}
			else	{
				SNPRINT("%c", c);
			}
		}
		else if (S_PERCENT == s)	{
			if ('a' == c)	{
				if (argi >= argc)	{
					MKC_CB_ERROR(
							"runtime error: snprint: "
							"parameter %d not provided"
							, argi + 1
							);
					goto __exx;
				}
				arg = MKC_CB_ARGV(argi);
				argi ++;
				r = __mkc_cb_data_snprint_auto(
						session
						, mkc_cb_stack
						, buffer + offset
						, buffer_len - offset
						, arg
						);
				if (r >= (buffer_len - offset) || r <= 0)	{
					result->integer = -1;
					MKC_CB_ERROR("runtime error: snprint: buffer size");
					goto __exx;
				}
				else	{
					offset += r;
				}
				s = S_NORMAL;
			}
			else if ('d' == c)	{
				if (argi >= argc)	{
					MKC_CB_ERROR(
							"runtime error: snprint: "
							"parameter %d not provided"
							, argi + 1
							);
					goto __exx;
				}
				arg = MKC_CB_ARGV(argi);
				argi ++;
				if (MKC_D_INT == arg->type)	{
					SNPRINT("%d", arg->integer);
				}
				else	{
					MKC_CB_ERROR(
							"runtime error: snprint: "
							"parameter %d is not an integer"
							, argi + 1
							);
					goto __exx;
				}
				s = S_NORMAL;
			}
			else if ('s' == c)	{
				if (argi >= argc)	{
					MKC_CB_ERROR(
							"runtime error: snprint: "
							"parameter %d not provided"
							, argi + 1
							);
					goto __exx;
				}
				arg = MKC_CB_ARGV(argi);
				argi ++;
				if (MKC_D_BUF == arg->type)	{
					BUFPRINT;
				}
				else	{
					MKC_CB_ERROR(
							"runtime error: snprint: "
							"parameter %d is not an string"
							, argi + 1
							);
					goto __exx;
				}
				s = S_NORMAL;
			}
			else	{
				SNPRINT("%%%c", c);
				s = S_NORMAL;
			}
		}
		else	{
			if ('%' == c)	{
				SNPRINT("%%");
				s = S_NORMAL;
			}
			else if ('\\' == c)	{
				SNPRINT("\\");
				s = S_NORMAL;
			}
			else if ('r' == c)	{
				s = S_NORMAL;
			}
			else if ('n' == c)	{
				SNPRINT("\n");
				s = S_NORMAL;
			}
			else	{
				SNPRINT("\\%c", c);
				s = S_NORMAL;
			}
		}
	}

	if (offset < buffer_len)	{
		result->type = MKC_D_BUF;
		result->buffer = buffer;
		result->length = offset;
	}
	else	{
		goto __exx;
	}

	return 0;

__exx:

	result->type = MKC_D_VOID;

	return 0;
}

/* eof */
