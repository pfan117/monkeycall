/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "include/internal.h"
#include "libs/misc.h"
#include "include/stack.h"
#include "include/debug.h"

mkc_cb_stack_frame *
mkc_new_cb_stack(mkc_session * session, unsigned long argc)
{
	mkc_cb_stack_frame * p;
	int size;

	size = sizeof(mkc_cb_stack_frame) + sizeof(mkc_data *) * argc;
	p = mkc_session_malloc(session, size);
	if (!p)	{
		MKC_ERROR("%s", "out of memory");
		return NULL;
	}
	p->depth = 0;
	p->argc = argc;

	return p;
}

mkc_stack_frame *
mkc_new_stack(mkc_session * session, unsigned long argc, unsigned long depth)
{
	mkc_stack_frame * p;
	int size;
	int i;

	size = sizeof(mkc_stack_frame) + sizeof(mkc_data) * (depth + argc);
	p = mkc_session_malloc(session, size);
	if (!p)	{
		MKC_ERROR("%s", "out of memory");
		return NULL;
	}
	p->depth = depth;
	p->argc = argc;
	FOR(i, argc + depth)	{
		p->stack[i].type = MKC_D_VOID;
	}

	return p;
}

void
mkc_stack_push(mkc_session * session, mkc_stack_frame * stack)	{
	stack->caller_stack = session->stack;
	session->stack = stack;
	return;
}

int
mkc_stack_pop(mkc_session * session)	{
	mkc_stack_frame * pp;
	mkc_stack_frame * p;

	p = session->stack;
	pp = p->caller_stack;
	if (!pp)	{
		MKC_ERROR("runtime error: miss place return");
		return -1;
	}
	session->stack = pp;
	mkc_session_free(session, p);
	return 0;
}

STATIC mkc_data *
mkc_find_variable_from_stack(mkc_stack_frame * sf, int idx)	{
	if (idx >= sf->depth)	{
		Panic();
		return NULL;
	}

	return sf->stack + idx;
}

mkc_data *
mkc_find_parameter_from_stack(mkc_stack_frame * sf, int idx)	{
	if (idx >= sf->argc)	{
		Panic();
		return NULL;
	}

	return sf->stack + (sf->depth + idx);
}

mkc_data *
mkc_get_data_by_location(mkc_session * session, mkc_location * location)	{
	switch(location->storage)	{
	case MKC_STORAGE_NOVALUE:
		Panic();
		return NULL;
	case MKC_STORAGE_IMMEDIATE:
		return location->data;
	case MKC_STORAGE_TEMP:
	case MKC_STORAGE_LOCAL:
		return mkc_find_variable_from_stack(session->stack, location->index);
	case MKC_STORAGE_PARAMETER:
		return mkc_find_parameter_from_stack(session->stack, location->index);
	case MKC_STORAGE_GLOBAL:
		return mkc_find_variable_from_stack(
				session->global_stack, location->index);
	case MKC_STORAGE_GLOBAL_PARAMETER:
		return mkc_find_parameter_from_stack(
				session->global_stack, location->index);
	default:
		Panic();
		return NULL;
	}
}

/* eof */
