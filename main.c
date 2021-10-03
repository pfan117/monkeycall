/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <stdlib.h>
#include "monkeycall.h"

void *
mkc_user_provide_malloc(int size, const char * file, int line)	{
	return malloc(size);
}

void
mkc_user_provide_free(void * p)	{
	free(p);
	return;
}

#define FILE_SIZE_LIMIT		(1024 * 1024)
#define SESSION_ERROR_BUF	(1024)

STATIC int
handle_one_file(FILE * fp)	{
	char error_info[SESSION_ERROR_BUF];
	mkc_session * session;
	void * instance;
	char * buffer;
	size_t len;
	int r;

	buffer = malloc(FILE_SIZE_LIMIT);
	if (!buffer)	{
		fprintf(stderr, "error: failed to create file buffer\n");
		return 1;
	}

	len = fread(buffer, 1, FILE_SIZE_LIMIT, fp);
	if (len >= FILE_SIZE_LIMIT)	{
		free(buffer);
		fprintf(stderr, "error: unexpected file size, only accept %d bytes\n"
				, FILE_SIZE_LIMIT);
		return 1;
	}

	instance = mkc_new_instance();
	if (!instance)	{
		free(buffer);
		fprintf(stderr, "error: failed to create instance.\n");
		return 1;
	}

	r = mkc_buildin_cbs_init(instance);
	if (r)	{
		free(buffer);
		fprintf(stderr, "error: failed to install callbacks.\n");
		return 1;
	}

	session = mkc_new_session(instance, buffer, len);
	if (!session)	{
		mkc_free_instance(instance);
		free(buffer);
		fprintf(stderr, "error: failed to create session.\n");
		return 1;
	}

	mkc_session_set_error_info_buffer(session, error_info, sizeof(error_info));

	r = mkc_parse(session);
	if (r)	{
		fprintf(stderr, "%s\n", error_info);
		goto __exx;
	}

	free(buffer);
	buffer = NULL;

	r = mkc_go(session);
	if (r)	{
		fprintf(stderr, "%s\n", error_info);
		goto __exx;
	}

	if (MKC_D_INT == session->result.type)	{
		r = session->result.integer;
	}

__exx:

	if (buffer)	{
		free(buffer);
	}
	mkc_free_session(session);
	mkc_free_instance(instance);

	return r;
}

STATIC void
show_usage(const char * argv0)	{
	printf(
"Using:\n"
"%s\n"
"%s FILENAME\n"
		, argv0
		, argv0
		);

	return;
}

int
main(int argc, char ** argv)	{

#ifdef TC_ENUMERATOR
	FILE * fp;
	int r, i;

	for (i = 1; i < argc; i ++)	{
		fp = fopen(argv[i], "r");
		if (fp)	{
			r = handle_one_file(fp);
			if (r)	{
				printf("info: handling %s return error\n", argv[i]);
			}
			else	{
				printf("info: handling %s return ok\n", argv[i]);
			}
			fclose(fp);
		}
		else	{
			fprintf(stderr, "error: failed to open file '%s'\n", argv[i]);
			exit(1);
		}
	}

#else
	FILE * fp;
	int r;

	if (1 == argc)	{
		r = handle_one_file(stdin);
	}
	else if (2 == argc)	{
		fp = fopen(argv[1], "r");
		if (fp)	{
			r = handle_one_file(fp);
			fclose(fp);
		}
		else	{
			fprintf(stderr, "error: failed to open file '%s'\n", argv[1]);
			r = 1;
		}
	}
	else	{
		show_usage(argv[0]);
		r = 1;
	}

	if (r)	{
		exit(1);
	}
	else	{
		exit(0);
	}
#endif

}

/* eof */
