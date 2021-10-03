/* vi: set sw=4 ts=4: */

#include <string.h>
#include <stdlib.h>

#include "monkeycall.h"
#include "include/internal.h"
#include "ll.h"

typedef struct m_head	{
	struct m_head * prev;
	struct m_head * next;
} m_head;

void *
mkc_session_do_malloc(
		mkc_session * session, int size
		, const char * file, int line
		)
{
	m_head * root;
	m_head * p;

	p = (m_head *)mkc_user_provide_malloc(size + sizeof(m_head), file, line);
	if (!p)	{
		return NULL;
	}

	root = session->mm_head;
	DOUBLE_LL_JOIN(p, next, prev,root);
	session->mm_head = root;
	session->mm_count ++;

	return (void *)(p + 1);
}

void
mkc_session_free(mkc_session * session, void * p)	{
	m_head * op = (((m_head *)p) - 1);
	m_head * root;

	root = session->mm_head;
	DOUBLE_LL_AWAY(op, next, prev, root);
	session->mm_head = root;
	session->mm_count --;
	mkc_user_provide_free(op);

	return;
}

void
mkc_session_free_all_mm(mkc_session * session)	{
	m_head * p;
	m_head * n;
	m_head * root;

	root = session->mm_head;
	SINGLE_LL_FREE(p, n, next, root);
	session->mm_head = root;

	return;
}

/* eof */
