/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"
#include "include/debug.h"

STATIC int
__mkc_cb_data_print_auto(mkc_data * data)	{
	switch(data->type)	{
	case MKC_D_VOID:
		return 0;
	case MKC_D_INT:
		printf("%d", data->integer);
		return 0;
	case MKC_D_ID:
		Panic();
		return -1;
	case MKC_D_BUF:
		mkc_dump_hex(data->buffer, data->length);
		return 0;
	case MKC_D_MAX:
	default:
		Panic();
		return -1;
	}
}

enum	{ S_NORMAL, S_PERCENT, S_SLASH };

STATIC int
__mkc_cb_print_by_fmt(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	const char * fmt;
	mkc_data * arg;
	int length;
	int argc;
	int argi;
	char c;
	int s;
	int i;

	argi = 1;
	s = S_NORMAL;
	argc = MKC_CB_ARGC;
	fmt = MKC_CB_ARGV(0)->buffer;
	length = MKC_CB_ARGV(0)->length;

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
				printf("%c", c);
			}
		}
		else if (S_PERCENT == s)	{
			if ('a' == c)	{
				if (argi >= argc)	{
					MKC_CB_ERROR(
							"runtime error: print: "
							"parameter %d not provided"
							, argi + 1
							);
					return 0;
				}
				arg = MKC_CB_ARGV(argi);
				argi ++;
				__mkc_cb_data_print_auto(arg);
				s = S_NORMAL;
			}
			if ('d' == c)	{
				if (argi >= argc)	{
					MKC_CB_ERROR(
							"runtime error: print: "
							"parameter %d not provided"
							, argi + 1
							);
					return 0;
				}
				arg = MKC_CB_ARGV(argi);
				argi ++;
				if (MKC_D_INT == arg->type)	{
					printf("%d", arg->integer);
				}
				else	{
					MKC_CB_ERROR(
							"runtime error: print: "
							"parameter %d is not an integer"
							, argi + 1
							);
					return 0;
				}
				s = S_NORMAL;
			}
			else if ('s' == c || 'b' == c)	{
				if (argi >= argc)	{
					MKC_CB_ERROR(
							"runtime error: print: "
							"parameter %d not provided"
							, argi + 1
							);
					return 0;
				}
				arg = MKC_CB_ARGV(argi);
				argi ++;
				if (MKC_D_BUF == arg->type)	{
					mkc_dump_hex(arg->buffer, arg->length);
				}
				else	{
					MKC_CB_ERROR(
							"runtime error: print: "
							"parameter %d is not an buffer"
							, argi + 1
							);
					return 0;
				}
				s = S_NORMAL;
			}
			else	{
				printf("%%%c", c);
				s = S_NORMAL;
			}
		}
		else	{
			if ('%' == c)	{
				printf("%%");
				s = S_NORMAL;
			}
			else if ('\\' == c)	{
				printf("\\");
				s = S_NORMAL;
			}
			else if ('r' == c)	{
				s = S_NORMAL;
			}
			else if ('n' == c)	{
				printf("\n");
				s = S_NORMAL;
			}
			else	{
				printf("\\%c", c);
				s = S_NORMAL;
			}
		}
	}
	
	return 0;
}

#define Print

int
mkc_cb_print(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * arg;
	int argc;
	int i;
	int r;

	argc = MKC_CB_ARGC;
	if (!argc)	{
		return 0;
	}

	arg = MKC_CB_ARGV(0);
	if (MKC_D_BUF == arg->type)	{
		return __mkc_cb_print_by_fmt(session, mkc_cb_stack);
	}

	for (i = 0; i < argc ; i ++)	{
		r = __mkc_cb_data_print_auto(MKC_CB_ARGV(i));
		if (r)	{
			return -1;
		}
	}

	return 0;
}

/* eof */
