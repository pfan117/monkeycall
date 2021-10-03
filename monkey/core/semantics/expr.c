/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "libs/ll.h"
#include "include/internal.h"
#include "include/symbol-table.h"
#include "include/var-allocate.h"
#include "include/translate.h"
#include "include/debug.h"

/* #define SHOW_TREE_TYPE */

#define UNEXPECTED	\
MKC_INTERNAL_ERROR;	\
Trace(); \
printf("line %d, unexpected node type = %s\n" \
		, tree->lineno \
		, mkc_tree_type_name(type) \
		); \
return -1;

#define CONSUME	\
mkc_tree_self_loop(tree); \
ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, tree); \
return 0;

#define VA_ALLOC \
va_idx = mkc_va_alloc(session); \
IF_ERROR_RETURN(va_idx); \
tree->result_location.storage = MKC_STORAGE_TEMP; \
tree->result_location.index = va_idx;

typedef struct _mkc_expr_translate_session	{
	mkc_tree * pre_expr_tree;
	mkc_tree * expr_tree;
	mkc_tree * post_expr_tree;
} mkc_expr_translate_session;

STATIC mkc_tree *
__mkc_unlink_no_side_effect_join(mkc_tree * list, mkc_tree * tree)	{
	mkc_tree * last;

	if (!list)	{
		return tree;
	}

	last = list->prev;
	if (MKC_T_IMMEDIATE_DATA == last->type || MKC_T_ID == last->type)	{
		if (last == list)	{
			return tree;
		}
		else	{
			last->prev->next = last->next;
			last->next->prev = last->prev;
		}
	}

	return mkc_tree_join(list, tree);
}

STATIC int
__mkc_translate_logical_and_expr(
		mkc_session * session
		, mkc_expr_translate_session * ts
		, mkc_tree ** ptree)
{
	mkc_tree * right_result;
	mkc_tree * left_result;
	mkc_tree * condition;
	mkc_tree * right;
	mkc_tree * left;
	mkc_tree * tree;
	int va_idx;

	tree = *ptree;
	left = tree->left;
	right = tree->right;
	condition = mkc_tree_new(session, MKC_T_CONDITION, __FILE__, __LINE__);
	if (!condition)	{
		return -1;
	}

	IF_ERROR_RETURN(mkc_translate_expr(session, left, &left));
	left_result = session->last_result_expr_tree;

	condition->left_location = left_result->result_location;
	condition->left = NULL;
	condition->right = tree;

	IF_ERROR_RETURN(mkc_translate_expr(session, right, &right));
	right_result = session->last_result_expr_tree;

	tree->left_location = left_result->result_location;
	tree->right_location = right_result->result_location;
	FREE_TEMP_VAR(left_result);
	FREE_TEMP_VAR(right_result);
	VA_ALLOC
	mkc_tree_self_loop(tree);

	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, left);
	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, condition);
	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, right);
	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, tree);

	return 0;
}

STATIC int
__mkc_translate_logical_or_expr(
		mkc_session * session
		, mkc_expr_translate_session * ts
		, mkc_tree ** ptree)
{
	mkc_tree * right_result;
	mkc_tree * left_result;
	mkc_tree * condition;
	mkc_tree * right;
	mkc_tree * left;
	mkc_tree * tree;
	int va_idx;

	tree = *ptree;
	left = tree->left;
	right = tree->right;
	condition = mkc_tree_new(session, MKC_T_CONDITION, __FILE__, __LINE__);
	if (!condition)	{
		return -1;
	}

	IF_ERROR_RETURN(mkc_translate_expr(session, left, &left));
	left_result = session->last_result_expr_tree;

	condition->left_location = left_result->result_location;
	condition->left = tree;
	condition->right = NULL;

	IF_ERROR_RETURN(mkc_translate_expr(session, right, &right));
	right_result = session->last_result_expr_tree;

	tree->left_location = left_result->result_location;
	tree->right_location = right_result->result_location;
	FREE_TEMP_VAR(left_result);
	FREE_TEMP_VAR(right_result);
	VA_ALLOC
	mkc_tree_self_loop(tree);

	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, left);
	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, condition);
	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, right);
	ts->expr_tree = __mkc_unlink_no_side_effect_join(ts->expr_tree, tree);

	return 0;
}

STATIC int
__mkc_translate_expr(
		mkc_session * session
		, mkc_expr_translate_session * ts
		, mkc_tree ** ptree
		)
{
	mkc_tree_type type;
	mkc_tree * right;
	mkc_tree * left;
	mkc_tree * tree;
	int va_idx;

	tree = *ptree;
	type = tree->type;
	#ifdef SHOW_TREE_TYPE
	printf("expr >> %s\n", mkc_tree_type_name(type));
	#endif
	switch (type)	{
	case MKC_T_MIN:
		UNEXPECTED;
	case MKC_T_IMMEDIATE_DATA:
		tree->result_location.storage = MKC_STORAGE_IMMEDIATE;
		tree->result_location.data = tree->data;
		CONSUME
	case MKC_T_ID:
		IF_ERROR_RETURN(mkc_symbol_locate_rvalue(session, tree));
		CONSUME
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
		left = tree->left;
		right = tree->right;
		IF_ERROR_RETURN(mkc_symbol_locate_lvalue(session, left));
		IF_ERROR_RETURN(__mkc_translate_expr(session, ts, &right));
		tree->left_location = left->result_location;
		tree->right_location = right->result_location;
		FREE_TEMP_VAR(right);
		tree->result_location = left->result_location;
		CONSUME
	case MKC_T_OP_LOGIC_AND:
		return __mkc_translate_logical_and_expr(session, ts, ptree);
	case MKC_T_OP_LOGIC_OR:
		return __mkc_translate_logical_or_expr(session, ts, ptree);
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
		left = tree->left;
		right = tree->right;
		IF_ERROR_RETURN(__mkc_translate_expr(session, ts, &left));
		IF_ERROR_RETURN(__mkc_translate_expr(session, ts, &right));
		tree->left_location = left->result_location;
		tree->right_location = right->result_location;
		FREE_TEMP_VAR(left);
		FREE_TEMP_VAR(right);
		VA_ALLOC
		CONSUME
	case MKC_T_OP_LOGIC_NOT:
	case MKC_T_OP_BIT_NOT:
	case MKC_T_OP_MATH_NAGTIVE:
		left = tree->left;
		IF_ERROR_RETURN(__mkc_translate_expr(session, ts, &left));
		tree->left_location = left->result_location;
		FREE_TEMP_VAR(left);
		VA_ALLOC
		CONSUME
	case MKC_T_OP_MATH_PRE_INC:
	case MKC_T_OP_MATH_PRE_DEC:
		left = tree->left;
		IF_ERROR_RETURN(mkc_symbol_locate_variable(session, left));
		tree->left_location = left->result_location;
		mkc_tree_self_loop(tree);
		ts->pre_expr_tree = mkc_tree_join(ts->pre_expr_tree, tree);
		*ptree = left;
		tree = left;
		CONSUME
	case MKC_T_OP_MATH_POST_INC:
	case MKC_T_OP_MATH_POST_DEC:
		left = tree->left;
		IF_ERROR_RETURN(mkc_symbol_locate_variable(session, left));
		tree->left_location = left->result_location;
		mkc_tree_self_loop(tree);
		ts->post_expr_tree = mkc_tree_join(ts->post_expr_tree, tree);
		*ptree = left;
		tree = left;
		CONSUME
	case MKC_T_CALL:
		mkc_tree_loop_break(tree->right);
		SINGLE_LL_FOREACH(right, next, tree->right)	{
			IF_ERROR_RETURN(mkc_translate_expr(session, right->left, &left));
			right->result_location
					= session->last_result_expr_tree->result_location;
			ts->expr_tree
					= __mkc_unlink_no_side_effect_join(ts->expr_tree, left);
		}
		if (tree->right)	{
			tree->right->param_list_length = 0;
			SINGLE_LL_FOREACH(right, next, tree->right)	{
				FREE_TEMP_VAR(right);
				tree->right->param_list_length ++;
			}
		}
		VA_ALLOC
		CONSUME
	case MKC_T_CONDITION:
		IF_ERROR_RETURN(mkc_translate_expr(session, tree->sub, &left));
		tree->left_location = session->last_result_expr_tree->result_location;
		FREE_TEMP_VAR(session->last_result_expr_tree);
		mkc_tree_self_loop(tree);
		ts->expr_tree = __mkc_unlink_no_side_effect_join(left, tree);
		return 0;
	case MKC_T_GOTO:
	case MKC_T_BREAK:
	case MKC_T_CONTINUE:
	case MKC_T_RETURN0:
		UNEXPECTED;
	case MKC_T_RETURN1:
		left = tree->left;
		IF_ERROR_RETURN(__mkc_translate_expr(session, ts, &left));
		tree->left_location = left->result_location;
		FREE_TEMP_VAR(left);
		VA_ALLOC
		CONSUME
	case MKC_T_EXIT0:
		UNEXPECTED;
	case MKC_T_EXIT1:
		left = tree->left;
		IF_ERROR_RETURN(__mkc_translate_expr(session, ts, &left));
		tree->left_location = left->result_location;
		FREE_TEMP_VAR(left);
		VA_ALLOC
		CONSUME
	case MKC_T_LABEL:
		UNEXPECTED;
	case MKC_T_SWITCH:
		IF_ERROR_RETURN(mkc_translate_expr(session, tree->left, &left));
		tree->left_location = session->last_result_expr_tree->result_location;
		FREE_TEMP_VAR(session->last_result_expr_tree);
		mkc_tree_self_loop(tree);
		ts->expr_tree = __mkc_unlink_no_side_effect_join(left, tree);
		return 0;
	case MKC_T_SWITCH_BRANCH:
	case MKC_T_SCOPE:
	case MKC_T_FUNCTION_DEF:
	case MKC_T_FUNCTION:
		UNEXPECTED;
	case MKC_T_CONST_ITEM:
		left = tree->left;
		right = tree->right;
		IF_ERROR_RETURN(mkc_symbol_add_const(session, left));
		IF_ERROR_RETURN(__mkc_translate_expr(session, ts, &right));
		tree->type = MKC_T_OP_ASSIGN;
		tree->left_location = left->result_location;
		tree->right_location = right->result_location;
		FREE_TEMP_VAR(right);
		tree->result_location = left->result_location;
		CONSUME
	case MKC_T_VARIABLE_ITEM:
		IF_ERROR_RETURN(mkc_symbol_locate_lvalue(session, tree->left));
		tree->type = MKC_T_OP_ASSIGN;
		IF_ERROR_RETURN(mkc_translate_expr(session, tree, &left));
		FREE_TEMP_VAR(session->last_result_expr_tree);
		ts->expr_tree = left;
		return 0;
	case MKC_T_VARIABLE_ITEM_0:
		IF_ERROR_RETURN(mkc_symbol_add_variable(session, tree->left));
		return 0;
	case MKC_T_CALL_FUNCTION:
	case MKC_T_CALL_CB:
	case MKC_T_JUMP:
	case MKC_T_CALL_PARAM:
	default:
		UNEXPECTED;
	}
}

int
mkc_translate_expr(mkc_session * session, mkc_tree * stree, mkc_tree ** rp)
{
	mkc_expr_translate_session ts;
	mkc_tree * tree;
	mkc_tree * rt;
	int r;

	ts.pre_expr_tree = NULL;
	ts.expr_tree = NULL;
	ts.post_expr_tree = NULL;
	tree = stree;

	r = __mkc_translate_expr(session, &ts, &tree);
	if (r)	{
		return -1;
	}

	session->last_result_expr_tree = tree;
	rt = __mkc_unlink_no_side_effect_join(ts.pre_expr_tree, ts.expr_tree);
	rt = __mkc_unlink_no_side_effect_join(rt, ts.post_expr_tree);
	*rp = rt;

	return 0;
}

/* eof */
