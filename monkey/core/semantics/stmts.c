/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "libs/ll.h"
#include "include/debug.h"
#include "include/internal.h"
#include "include/symbol-table.h"
#include "include/label-table.h"
#include "include/translate.h"
#include "include/var-allocate.h"

/* #define SHOW_TREE_TYPE */

#define UNEXPECTED											\
	MKC_INTERNAL_ERROR;										\
	printf("node type = %s\n", mkc_tree_type_name(type));	\
	r = -1;													\
	break;

#define CONSUME				\
	mkc_tree_self_loop(p);	\
	new_sequence = p;		\
	break;

int
mkc_translate_stmts(mkc_session * session, mkc_tree * tree, mkc_tree ** rp)
{
	mkc_tree * new_sequence;
	mkc_tree * sequence;
	mkc_tree_type type;
	mkc_tree * np;
	mkc_tree * p;
	mkc_tree * t;
	int r = 0;

	mkc_tree_loop_break(tree);
	sequence = NULL;

	for (p = tree; p; p = np)	{
		np = p->next;
		type = p->type;
		#ifdef SHOW_TREE_TYPE
		printf("stmt >> %s\n", mkc_tree_type_name(type));
		#endif
		switch (type)	{
		case MKC_T_MIN:
			UNEXPECTED;
		case MKC_T_IMMEDIATE_DATA:
		case MKC_T_ID:
		case MKC_T_OP_ASSIGN:
		case MKC_T_OP_ADD_ASSIGN:
		case MKC_T_OP_SUB_ASSIGN:
		case MKC_T_OP_MUL_ASSIGN:
		case MKC_T_OP_DIV_ASSIGN:
		case MKC_T_OP_MOD_ASSIGN:
		case MKC_T_OP_AND_ASSIGN:
		case MKC_T_OP_XOR_ASSIGN:
		case MKC_T_OP_OR_ASSIGN:
		case MKC_T_OP_LEFT_SHIFT_ASSIGN:
		case MKC_T_OP_RIGHT_SHIFT_ASSIGN:
		case MKC_T_OP_LOGIC_AND:
		case MKC_T_OP_LOGIC_OR:
		case MKC_T_OP_BIT_AND:
		case MKC_T_OP_BIT_OR:
		case MKC_T_OP_BIT_XOR:
		case MKC_T_OP_BIT_LEFT_SHIFT:
		case MKC_T_OP_BIT_RIGHT_SHIFT:
		case MKC_T_OP_MATH_DIV:
		case MKC_T_OP_MATH_MUL:
		case MKC_T_OP_MATH_MOD:
		case MKC_T_OP_MATH_ADD:
		case MKC_T_OP_MATH_SUB:
		case MKC_T_OP_CMP_EQUAL:
		case MKC_T_OP_CMP_DIFF:
		case MKC_T_OP_CMP_GREATER:
		case MKC_T_OP_CMP_LESS:
		case MKC_T_OP_CMP_GREATER_EQUAL:
		case MKC_T_OP_CMP_LESS_EQUAL:
		case MKC_T_OP_LOGIC_NOT:
		case MKC_T_OP_BIT_NOT:
		case MKC_T_OP_MATH_NAGTIVE:
		case MKC_T_OP_MATH_PRE_INC:
		case MKC_T_OP_MATH_PRE_DEC:
		case MKC_T_OP_MATH_POST_INC:
		case MKC_T_OP_MATH_POST_DEC:
		case MKC_T_CALL:
		case MKC_T_CONDITION:
			r = mkc_translate_expr(session, p, &new_sequence);
			FREE_TEMP_VAR(session->last_result_expr_tree);
			break;
		case MKC_T_GOTO:
			t = mkc_label_find(session, p->left->data);
			if (t)	{
				p->type = MKC_T_JUMP;
				p->left = t;
			}
			else	{
				p->right = session->backpatch_goto_list;
				session->backpatch_goto_list = p;
			}
			CONSUME
		case MKC_T_BREAK:
			t = mkc_symbol_get_break_label(session);
			if (!t)	{
				MKC_TREE_ERROR(p, "'break' must inside a loop or a switch");
				r = -1;
			}
			p->type = MKC_T_JUMP;
			p->left = t;
			CONSUME
		case MKC_T_CONTINUE:
			t = mkc_symbol_get_continue_label(session);
			if (!t)	{
				MKC_TREE_ERROR(p, "'continue' must inside a loop");
				r = -1;
			}
			p->type = MKC_T_JUMP;
			p->left = t;
			CONSUME
		case MKC_T_RETURN0:
			CONSUME
		case MKC_T_RETURN1:
			r = mkc_translate_expr(session, p, &new_sequence);
			FREE_TEMP_VAR(session->last_result_expr_tree);
			break;
		case MKC_T_EXIT0:
			CONSUME
		case MKC_T_EXIT1:
			r = mkc_translate_expr(session, p, &new_sequence);
			FREE_TEMP_VAR(session->last_result_expr_tree);
			break;
		case MKC_T_LABEL:
			if (p->left)	{
				r = mkc_label_add(session, p);
				if (r)	{
					break;
				}
			}
			CONSUME
		case MKC_T_SWITCH:
			r = mkc_translate_expr(session, p, &new_sequence);
			FREE_TEMP_VAR(session->last_result_expr_tree);
			break;
		case MKC_T_SWITCH_BRANCH:
			UNEXPECTED;
			break;
		case MKC_T_SCOPE:
			r = mkc_symbol_enter_brace(session);
			if (r)	{
				goto __exx;
			}
			mkc_symbol_set_brace_labels(session, p->left, p->right);
			r = mkc_translate_stmts(session, p->sub, &new_sequence);
			if (r)	{
				goto __exx;
			}
			mkc_symbol_exit_brace(session);
			break;
		case MKC_T_FUNCTION_DEF:
			UNEXPECTED;
			break;
		case MKC_T_FUNCTION:
			r = mkc_symbol_add_function(session, p);
			new_sequence = NULL;
			break;
		case MKC_T_CONST_ITEM:
		case MKC_T_VARIABLE_ITEM:
		case MKC_T_VARIABLE_ITEM_0:
			r = mkc_translate_expr(session, p, &new_sequence);
			FREE_TEMP_VAR(session->last_result_expr_tree);
			break;
		case MKC_T_CALL_FUNCTION:
		case MKC_T_CALL_CB:
			UNEXPECTED;
			break;
		case MKC_T_JUMP:
			CONSUME
		case MKC_T_CALL_PARAM:
		default:
			UNEXPECTED;
			break;
		}

		if (r)	{
__exx:
			*rp = NULL;
			return -1;
		}
		else	{
			sequence = mkc_tree_join(sequence, new_sequence);
		}
	}

	*rp = sequence;
	return 0;
}

/* eof */
