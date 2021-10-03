/* vi: set sw=4 ts=4: */

/*
 * Implementation of base64 encoding/decoding. 
 * @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 */

/* rewrite into ss-smd coding style by Dominic Chen */
/* modified by Dominic Chen:
 * '+' -> '-'
 * '/' -> '_' */

#include <string.h>
#include "build-in-cbs/internal.h"

/* ----------------------------------------------------------------- Private */

/* Base64 encode one byte */
STATIC char
encode(unsigned char u) {
	if (u < 26)		return 'A' + u;
	if (u < 52)		return 'a' + (u - 26);
	if (u < 62)		return '0' + (u - 52);
	/* if (u == 62)	return '+'; */
	if (u == 62)	return '-';
	/* return '/'; */
	return '_';
}


/* Decode a base64 character */
STATIC unsigned char
decode(char c)	{
	if (c >= 'A' && c <= 'Z')	return(c - 'A');
	if (c >= 'a' && c <= 'z')	return(c - 'a' + 26);
	if (c >= '0' && c <= '9')	return(c - '0' + 52);
	if (c == '-')				return 62;
	return 63;
}


/* Return TRUE if 'c' is a valid base64 character, otherwise FALSE */
STATIC int
is_base64(char c)	{
	if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
		|| (c >= '0' && c <= '9')
		|| (c == '-')
		|| (c == '_')
		|| (c == '='))	{
		return 1;
	}
	return 0;
}

/* ------------------------------------------------------------------ Public */

int
encode_base64_core(const char * str, char * dst, int size)	{
	unsigned const char * src = (unsigned const char *)str;
	char * p;
	int len;
	int i;

	p = dst;

	for (i = 0; i < size; i += 3)	{
		unsigned char b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0, b7 = 0;

		b1 = src[i];

		if (i + 1 < size)	{
			b2 = src[i + 1];
		}

		if (i + 2 < size)	{
			b3 = src[i + 2];
		}

		b4 = b1 >> 2;
		b5 = ((b1 & 0x3) << 4)| (b2 >> 4);
		b6 = ((b2 & 0xf) << 2)| (b3 >> 6);
		b7 = b3 & 0x3f;

		*p ++= encode(b4);
		*p ++= encode(b5);

		if (i + 1 < size)	{
			*p ++= encode(b6);
		}
		else	{
			*p ++ = '=';
		}

		if (i + 2 < size)	{
			*p ++= encode(b7);
		}
		else	{
			*p ++= '=';
		}
	}

	len = p - dst;

	return len;
}

/* Decode the base64 encoded 'src' into the memory pointed to by 'dest'. */
/* 'dest' pointed memory must be large enough to recieve the decoded string. */
/* on failed return -1 */
/* on success return length of the decoded data */
int
decode_base64_len(char * dest, const char * src, int len)	{
	unsigned char * p = (unsigned char *)dest;
	int i;

	if (!p || !src)	{
		return -1;
	}

	/* if there is non Base64 code, drop */
	for (i = 0; i < len; i ++)	{
		if (!is_base64(src[i]))	{
			return -1;
		}
	}

	for (i = 0; i < len; i += 4)	{

		unsigned char b1 = 0, b2 = 0, b3 = 0, b4 = 0;
		char c1 = 'A', c2 = 'A', c3 = 'A', c4 = 'A';

		c1 = src[i];
		if (i + 1 < len)	{
			c2 = src[i + 1];
		}

		if (i + 2 < len)	{
			c3 = src[i + 2];
		}

		if(i + 3 < len)	{
			c4 = src[i + 3];
		}

		b1 = decode(c1);
		b2 = decode(c2);
		b3 = decode(c3);
		b4 = decode(c4);

		*p ++ = ((b1 << 2) | (b2 >> 4));

		if (c3 != '=')	{
			*p ++ = (((b2 & 0xf) << 4) | (b3 >> 2));
		}

		if (c4 != '=')	{
			*p ++ = (((b3 & 0x3) << 6) | b4);
		}
	}

	return (p - (unsigned char *)dest);
}

/* eof */
