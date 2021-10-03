/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"
#include "include/debug.h"

enum	{ S_NORMAL, S_PERCENT, S_SLASH };

STATIC int
__mkc_cb_scan_memcpy_shift(char * buffer, const char * src, int len)	{
	char c;
	int st;
	int i;
	int w;

	st = S_NORMAL;
	w = 0;

	for (i = 0; i < len; i ++)	{
		c = src[i];
		if (S_NORMAL == st)	{
			if ('\\' == c)	{
				st = S_SLASH;
			}
			else	{
				buffer[w] = c;
				w ++;
			}
		}
		else if (S_SLASH == st)	{
			if ('\\' == c)	{
				buffer[w] = '\\';
				w ++;
			}
			else if ('\"' == c)	{
				buffer[w] = '\"';
				w ++;
				st = S_NORMAL;
			}
			else	{
				buffer[w] = '\\';
				w ++;
				buffer[w] = c;
				w ++;
				st = S_NORMAL;
			}
		}
		else	{
			Panic();
			break;
		}
	}

	return w;
}

STATIC int
__mkc_cb_scan_match_string(
		mkc_session * session, const char * s, int l, mkc_data * result)
{
	char * buffer;
	int shift;
	int begin;
	int end;
	int len;
	char c;
	int st;
	int i;

	for (i = 0; i < l; i ++)	{
		c = s[i];
		if ('\"' == c)	{
			goto __begin;
		}
	}

	return -1;

__begin:

	i ++;
	begin = i;
	st = S_NORMAL;
	shift = 0;

	for(; i < l; i ++)	{
		c = s[i];
		if (S_NORMAL == st)	{
			if ('\\' == c)	{
				shift = 1;
				st = S_SLASH;
			}
			else if ('\"' == c)	{
				end = i;
				goto __finish;
			}
		}
		else if (S_SLASH == st)	{
			if ('\\' == c)	{
				;
			}
			else	{
				st = S_NORMAL;
			}
		}
		else	{
			Panic();
			break;
		}
	}

	return -1;

__finish:

	len = end - begin;

	buffer = mkc_session_malloc(session, len);
	if (!buffer)	{
		return -1;
	}

	if (shift)	{
		len = __mkc_cb_scan_memcpy_shift(buffer, s + begin, len);
	}
	else	{
		memcpy(buffer, s + begin, len);
	}

	result->type = MKC_D_BUF;
	result->buffer = buffer;
	result->length = len;

	return i + 1;
}

#define IS_NUMBER(__c__)	(__c__ >= '0' && __c__ <= '9')

STATIC int
__mkc_cb_scan_match_integer(
		mkc_session * session, const char * s, int l, mkc_data * result)
{
	int sum;
	char c;
	int i;

	for (i = 0; i < l; i ++)	{
		c = s[i];
		if (IS_NUMBER(c))	{
			goto __begin;
		}
	}

	return -1;

__begin:

	sum = 0;

	for (; i < l; i ++)	{
		c = s[i];
		if (IS_NUMBER(c))	{
			sum *= 10;
			sum += (c - '0');
		}
		else	{
			break;
		}
	}

	result->type = MKC_D_INT;
	result->integer = sum;

	return i;
}

STATIC int
__mkc_cb_scan_match_unsigned_integer(
		mkc_session * session, const char * s, int l, mkc_data * result)
{
	unsigned int sum;
	char c;
	int i;

	for (i = 0; i < l; i ++)	{
		c = s[i];
		if (IS_NUMBER(c))	{
			goto __begin;
		}
	}

	return -1;

__begin:

	sum = 0;

	for (; i < l; i ++)	{
		c = s[i];
		if (IS_NUMBER(c))	{
			sum *= 10;
			sum += (c - '0');
		}
		else	{
			break;
		}
	}

	result->type = MKC_D_INT;
	result->integer = sum;

	return i;
}

#define IS_HEX(__c__)	(__c__ >= 'A' && __c__ <= 'F')
#define IS_hex(__c__)	(__c__ >= 'a' && __c__ <= 'f')

STATIC int
__mkc_cb_scan_match_hex(
		mkc_session * session, const char * s, int l, mkc_data * result)
{
	int sum;
	char c;
	int i;

	for (i = 0; i < l; i ++)	{
		c = s[i];
		if (IS_NUMBER(c) || IS_HEX(c) || IS_hex(c))	{
			goto __begin;
		}
	}

	return -1;

__begin:

	sum = 0;

	for (; i < l; i ++)	{
		c = s[i];
		if (IS_HEX(c))	{
			sum <<= 4;
			sum += (c - 'A' + 10);
		}
		else if (IS_hex(c))	{
			sum <<= 4;
			sum += (c - 'a' + 10);
		}
		else if (IS_NUMBER(c))	{
			sum <<= 4;
			sum += (c - '0');
		}
		else	{
			break;
		}
	}

	result->type = MKC_D_INT;
	result->integer = sum;

	return i;
}

#define Scan

int
mkc_cb_scan(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	mkc_data * result;
	const char * src;
	const char * fmt;
	mkc_data * arg;
	int src_len;
	int fmt_len;
	int offset;
	int argc;
	int argi;
	char c;
	int s;
	int i;
	int r;

	result = MKC_CB_RESULT;
	argc = MKC_CB_ARGC;
	if (argc <= 2)	{
		goto __exx;
	}
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_BUF);

	arg = MKC_CB_ARGV(0);
	src = arg->buffer;
	src_len = arg->length;

	arg = MKC_CB_ARGV(1);
	fmt = arg->buffer;
	fmt_len = arg->length;

	argi = 2;
	offset = 0;
	s = S_NORMAL;

	for (i = 0; i < fmt_len; i ++)	{
		c = fmt[i];
		if (S_NORMAL == s)	{
			if ('%' == c)	{
				if (argi >= argc)	{
					goto __exx;
				}
				else	{
					arg = MKC_CB_ARGV(argi);
					argi ++;
				}
				s = S_PERCENT;
			}
			else	{
				goto __exx;
			}
		}
		else	{
		/* else if (S_PERCENT == s) */
			if (offset >= src_len)	{
				goto __exx;
			}

			if ('s' == c)	{
				r = __mkc_cb_scan_match_string(
						session, src + offset, src_len - offset, arg);
			}
			else if ('d' == c)	{
				r = __mkc_cb_scan_match_integer(
						session, src + offset, src_len - offset, arg);
			}
			else if ('u' == c)	{
				r = __mkc_cb_scan_match_unsigned_integer(
						session, src + offset, src_len - offset, arg);
			}
			else if ('x' == c)	{
				r = __mkc_cb_scan_match_hex(
					session, src + offset, src_len - offset, arg);
			}
			else	{
				goto __exx;
			}

			if (r <= 0)	{
				goto __exx;
			}
			else	{
				s = S_NORMAL;
				offset += r;
			}
		}
	}

	result->type = MKC_D_INT;
	result->integer = 0;

	return 0;

__exx:

	result->type = MKC_D_INT;
	result->integer = -1;

	return 0;
}

/* eof */
