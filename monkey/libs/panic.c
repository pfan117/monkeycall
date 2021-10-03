/* vi: set sw=4 ts=4: */

#include <stdio.h>
//#include <../local/include/execinfo.h>
#include <execinfo.h>
#include <stdlib.h>
#include "monkeycall.h"
#include "include/types.h"

int
mkc_fbacktrace(FILE * fp)	{
	int i, nptrs;
	char ** strings;
	#define SIZE 100
	void * buffer[SIZE];

	nptrs = backtrace(buffer, SIZE);
	strings = backtrace_symbols(buffer, nptrs);
	if (!strings)	{
		return -1;
	}

	fprintf(fp, "%s\n", "-- backtrace start --");
	for (i = 0; i < nptrs; i ++)	{
        fprintf(fp, "%s\n", strings[i]);
	}
	fprintf(fp, "%s\n", "-- backtrace end --");

	free(strings);

	return 0;
}

void
mkc_panic(const char * file, int line)	{
	printf("\n** PANIC: design error **\n"
			"file: %s, line %d\n", file, line);
	mkc_fbacktrace(stdout);
	return;
}

/* eof */
