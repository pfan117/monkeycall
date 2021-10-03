/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <strings.h>
#include "monkeycall.h"
#include "include/internal.h"
#include "libs/misc.h"
#include "libs/ll.h"

#define __T(__V)	#__V
STATIC const char * __mkc_data_type_name[] = {
	_MKC_DS_
};
#undef __T

const char *
mkc_data_type_name(mkc_data_type type)	{
	if (type < MKC_D_VOID || type >= MKC_D_MAX)	{
		return "invalid";
	}
	return __mkc_data_type_name[type];
}

#define __T(__V)	#__V
STATIC const char * __mkc_tree_type_name[] = {
	_MKC_TS_
};
#undef __T

const char *
mkc_tree_type_name(mkc_tree_type type)	{
	if (type < MKC_T_MIN || type >= MKC_T_MAX)	{
		return "invalid tree type";
	}
	return __mkc_tree_type_name[type];
}

mkc_data *
mkc_data_new(mkc_session * session, mkc_data_type type, const char * fn, int ln)
{
	mkc_data * data;

	data = mkc_session_do_malloc(session, sizeof(mkc_data), fn, ln);
	if (!data)	{
		return NULL;
	}

	data->type = type;

	return data;
}

mkc_tree *
mkc_tree_new(mkc_session * session, mkc_tree_type type, const char * fn, int ln)
{
	mkc_tree * tree;

	tree = mkc_session_do_malloc(session, sizeof(mkc_tree), fn, ln);
	if (!tree)	{
		return NULL;
	}

	bzero(tree, sizeof(mkc_tree));
	tree->prev = tree;
	tree->next = tree;
	tree->type = type;

	return tree;
}

mkc_tree *
mkc_tree_join(mkc_tree * a, mkc_tree * b)	{
	if (a)	{
		if (b)	{
			mkc_tree * a_tail;
			mkc_tree * b_tail;

			a_tail = a->prev;
			b_tail = b->prev;

			a_tail->next = b;
			b_tail->next = a;
			a->prev = b_tail;
			b->prev = a_tail;

			return a;
		}
		else	{
			return a;
		}
	}
	else	{
		if (b)	{
			return b;
		}
		else	{
			return NULL;
		}
	}
}

void
mkc_tree_loop_break(mkc_tree * root)	{
	mkc_tree * start;
	mkc_tree * end;

	if (!root)	{
		return;
	}

	start = root;
	end = root->prev;

	start->prev = NULL;
	end->next = NULL;

	return;
}

void
mkc_tree_self_loop(mkc_tree * root)	{
	root->prev = root;
	root->next = root;
}

STATIC void
__mkc_machine_show_storage_location(const char * title, mkc_location * l)	{
	switch(l->storage)	{
	case MKC_STORAGE_NOVALUE:
		break;
	case MKC_STORAGE_IMMEDIATE:
		printf("%simmediate"
				, title
				);
		break;
	case MKC_STORAGE_TEMP:
		printf("%stemp[%d]"
				, title
				, l->index
				);
		break;
	case MKC_STORAGE_LOCAL:
		printf("%slocal[%d]"
				, title
				, l->index
				);
		break;
	case MKC_STORAGE_PARAMETER:
		printf("%sparam[%d]"
				, title
				, l->index
				);
		break;
	case MKC_STORAGE_GLOBAL:
		printf("%sglobal[%d]"
				, title
				, l->index
				);
		break;
	case MKC_STORAGE_GLOBAL_PARAMETER:
		printf("%sglobal-param[%d]"
				, title
				, l->index
				);
		break;
	default:
		printf ("error value in storage location!");
	}

	return;
}

void
mkc_dump_tree_list(mkc_tree * sequence)	{
	mkc_tree * start;
	mkc_tree * p;
	int i;

	i = 0;
	start = NULL;
	SINGLE_LL_FOREACH(p, next, sequence)	{
		if (start == p)	{
			printf("loop detected\n");
			break;
		}
		printf("%04d %p %s"
				, i
				, p
				, mkc_tree_type_name(p->type)
				);

		if (p->left)	{
			printf(" l:%p"
				, p->left
				);
		}

		if (p->right)	{
			printf(" r:%p"
				, p->right
				);
		}

		__mkc_machine_show_storage_location(" ll:", &(p->left_location));
		__mkc_machine_show_storage_location(" rl:", &(p->right_location));
		__mkc_machine_show_storage_location(" rsl:", &(p->result_location));

		if (MKC_T_CALL == p->type || MKC_T_CALL_CB == p->type
				|| MKC_T_CALL_FUNCTION == p->type)
		{
			mkc_tree * right;
			printf(" args:");
			SINGLE_LL_FOREACH(right, next, p->right)	{
				__mkc_machine_show_storage_location(" "
						, &(right->result_location));
			}
		}

		printf("\n");

		i ++;
		if (!start)	{
			start = p;
		}
	}

	return;
}

/* eof */
