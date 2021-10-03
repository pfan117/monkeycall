/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "monkeycall.h"
#include "include/types.h"
#include "libs/misc.h"

#define DUMP_LINE_WIDTH	16

STATIC void
__mkc_dump_hex_line(cc * b, int l, int * tlvl)	{
	int i;

	for(i = 0; i < l; i ++)	{
		printf("%02hhx ", (unsigned char)b[i]);
	}

	for(;i < DUMP_LINE_WIDTH; i ++)	{
		printf("   ");
	}

	printf("| ");

	for(i = 0; i < l; i ++)	{
		if (isprint((int)b[i]))	{
			printf("%c", b[i]);
		}
		else	{
			printf(" ");
		}
	}

	for(;i < DUMP_LINE_WIDTH; i ++)	{
		printf(" ");
	}

	printf(" |\n");

	return;
}

void
mkc_dump_hex(cc * b, int l)	{
	int tlvl;
	cc * p = b;
	int left = l;

	while(left)	{
		if (left >= DUMP_LINE_WIDTH)	{
			__mkc_dump_hex_line(p, DUMP_LINE_WIDTH, &tlvl);
			left -= DUMP_LINE_WIDTH;
			p += DUMP_LINE_WIDTH;
		}
		else	{
			__mkc_dump_hex_line(p, left, &tlvl);
			break;
		}
	}
	return;
}

/* eof */
