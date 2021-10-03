/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "libs/ll.h"
#include "include/debug.h"
#include "include/internal.h"
#include "include/symbol-table.h"
#include "include/label-table.h"
#include "include/var-allocate.h"
#include "include/translate.h"

/* #define SHOW_MACHINE_EXECUTION_LIST */

STATIC int
__mkc_translate_function_params(mkc_session * session, mkc_tree * param_list)
{
	int r;
	int cnt;
	mkc_tree * p;

	mkc_tree_loop_break(param_list);

	cnt = 0;
	SINGLE_LL_FOREACH(p, next, param_list)	{
		r = mkc_symbol_add_parameter(session, p, cnt);
		if (r)	{
			return -1;
		}
		cnt ++;
	}

	return cnt;
}

STATIC int
__mkc_translate_function_backpatch_goto(mkc_session * session)	{
	mkc_tree * p;
	mkc_tree * t;

	SINGLE_LL_FOREACH(p, right, session->backpatch_goto_list)	{
		t = mkc_label_find(session, p->left->data);
		if (t)	{
			p->type = MKC_T_JUMP;
			p->left = t;
		}
		else	{
			MKC_TREE_ERROR(p->left, "label undefined '%s'", p->left->data->id);
			return -1;
		}
	}

	return 0;
}

int
__mkc_translate_function(mkc_session * session, mkc_symbol_function * symbol)
{
	mkc_tree * sequence;
	mkc_tree * tree;
	mkc_tree * def;
	mkc_va va;
	int cnt;
	int r;

	tree = symbol->tree;
	session->current_function = tree;
	mkc_va_init(&va);
	session->va = &va;
	mkc_label_init(session);
	session->backpatch_goto_list = NULL;

	def = tree->right;
	if (def->left)	{
		cnt = __mkc_translate_function_params(session, def->left);
		if (cnt < 0)	{
			r = -1;
			goto __exx;
		}
	}
	else	{
		cnt = 0;
	}

	r = mkc_translate_stmts(session, def->right, &sequence);
	if (r)	{
		r = -1;
		goto __exx;
	}

	r = __mkc_translate_function_backpatch_goto(session);
	if (r)	{
		r = -1;
		goto __exx;
	}

	mkc_tree_loop_break(sequence);
	tree->sequence = sequence;
	symbol->param_cnt = cnt;
	symbol->stack_depth = mkc_va_depth(&va);

__exx:

	mkc_va_detach(session, &va);
	mkc_label_free(session);

	#ifdef SHOW_MACHINE_EXECUTION_LIST
	mkc_dump_tree_list(sequence);
	printf(" -- param_cnt = %d, stack_depth = %d\n"
			, symbol->param_cnt
			, symbol->stack_depth
			);
	#endif

	return r;
}

int
mkc_translate_function(mkc_session * session, mkc_symbol_function * symbol)	{
	int r;

	r = mkc_symbol_enter_brace(session);
	if (r)	{
		return -1;
	}

	r = __mkc_translate_function(session, symbol);

	mkc_symbol_exit_brace(session);

	return r;
}

/* eof */
