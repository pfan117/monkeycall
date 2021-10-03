/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <strings.h>

#include "monkeycall.h"
#include "include/internal.h"

mkc_session *
mkc_new_session(void * instance, const char * string, int string_len)
{
	int r;

	mkc_session * session;

	r = mkc_instance_rlock(instance);
	if (r)	{
		fprintf(stderr, "execution error: %s\n", "failed to read lock instance for new session creating");
		return NULL;
	}

	session = (mkc_session *)mkc_user_provide_malloc(sizeof(mkc_session), __FILE__, __LINE__);
	if (!session)	{
		mkc_instance_unlock(instance);
		return NULL;
	}

	bzero(session, sizeof(mkc_session));
	session->instance = instance;
	session->scanner.str = string;
	session->scanner.len = string_len;
	session->lineno = 1;
	session->user_param0 = NULL;
	session->user_param1 = NULL;

	return session;
}

void
mkc_session_set_user_param0(mkc_session * session, void * p)	{
	session->user_param0 = p;
}

void
mkc_session_set_user_param1(mkc_session * session, void * p)	{
	session->user_param1 = p;
}

void
mkc_free_session(mkc_session * session)	{
	mkc_instance_unlock(session->instance);
	mkc_session_free_all_mm(session);
	FREE(session);
	return;
}

void
mkc_session_set_error_info_buffer(
		mkc_session * session, char * buffer, int length)	{
	session->err_buffer = buffer;
	session->err_buffer_length = length;
	return;
}

/* eof */
