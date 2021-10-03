/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "string.h"
#include "monkeycall.h"
#include "include/internal.h"
#include "include/symbol-table.h"
#include "include/translate.h"
#include "include/stack.h"
#include "include/machine.h"
#include "include/debug.h"
#include "libs/misc.h"
#include "libs/ll.h"

/* #define SHOW_MACHINE_EXECUTION */

#define UNEXPECTED	\
	MKC_INTERNAL_ERROR;	\
	return -1;

#define LOCATE_LEFT \
left = mkc_get_data_by_location(session, &(p->left_location)); \
IF_NULL_RETURN(left); \

#define LOCATE_RIGHT \
right = mkc_get_data_by_location(session, &(p->right_location)); \
IF_NULL_RETURN(right); \

#define LOCATE_RESULT \
result = mkc_get_data_by_location(session, &(p->result_location)); \
IF_NULL_RETURN(result); \

#define HANDLE_CALL	do { \
	int argc; \
	mkc_symbol * symbol; \
	if (p->right)	{ \
		argc = p->right->param_list_length; \
	} \
	else	{ \
		argc = 0; \
	} \
	symbol = mkc_symbol_lookup_function(session, p->left->data);\
	if (symbol)	{ \
		if (symbol->function.tree->sequence)	{ \
			if (argc < symbol->function.param_cnt)	{ \
				MKC_TREE_ERROR(p, "runtime error: more parameter expected"); \
				return -1; \
			} \
		} \
		else	{ \
			r = mkc_translate_function(session, &(symbol->function)); \
			if (r)	{ \
				return -1; \
			} \
		} \
		p->type = MKC_T_CALL_FUNCTION; \
		p->symbol = symbol; \
		np = p; \
	} \
	else	{ \
		mkc_cb_t cb; \
		cb = mkc_cbtree_lookup(session->instance, p->left->data->id); \
		if (cb)	{ \
			p->type = MKC_T_CALL_CB; \
			p->cb = cb; \
			np = p; \
		} \
		else	{ \
			MKC_TREE_ERROR( \
					p->left \
					, "runtime error: '%s' is not a function or a callback" \
					, p->left->data->id); \
			return -1; \
		} \
	} \
} while(0);

#define HANDLE_CALL_FUNCTION do	{ \
	int i; \
	int argc; \
	int depth; \
	mkc_tree * param; \
	mkc_symbol * symbol; \
	symbol = p->symbol; \
	if (p->right)	{ \
		argc = p->right->param_list_length; \
	} \
	else	{ \
		argc = 0; \
	} \
	depth = symbol->function.stack_depth; \
	stack = mkc_new_stack(session, argc, depth); \
	IF_NULL_RETURN(stack); \
	stack->return_point = p; \
	stack->result = mkc_get_data_by_location(session, &(p->result_location)); \
	IF_NULL_RETURN(stack->result); \
	i = 0; \
	SINGLE_LL_FOREACH(param, next, p->right)	{ \
		left = mkc_get_data_by_location(session, &(param->result_location)); \
		if (!left)	{ \
			UNEXPECTED; \
			return -1; \
		} \
		if (i >= argc)	{ \
			Panic(); \
			return -1; \
		} \
		result = stack->stack + (depth + i); \
		i ++; \
		IF_ERROR_RETURN(mkc_assign(session, result, left)); \
	} \
	mkc_stack_push(session, stack); \
	np = symbol->function.tree->sequence; \
} while(0);

#define HANDLE_CALL_CB do	{ \
	int i; \
	int argc; \
	mkc_cb_t cb; \
	mkc_tree * param; \
	mkc_cb_stack_frame * stack; \
	cb = p->cb; \
	if (p->right)	{ \
		argc = p->right->param_list_length; \
	} \
	else	{ \
		argc = 0; \
	} \
	stack = mkc_new_cb_stack(session, argc); \
	IF_NULL_RETURN(stack); \
	stack->return_point = p; \
	stack->result = mkc_get_data_by_location(session, &(p->result_location)); \
	IF_NULL_RETURN(stack->result); \
	i = 0; \
	SINGLE_LL_FOREACH(param, next, p->right)	{ \
		left = mkc_get_data_by_location(session, &(param->result_location)); \
		if (!left)	{ \
			UNEXPECTED; \
			return -1; \
		} \
		if (i >= argc)	{ \
			Panic(); \
			return -1; \
		} \
		stack->stack[i] = left; \
		i ++; \
	} \
	r = cb(session, stack); \
	if (r)	{ \
		return -1; \
	} \
	mkc_session_free(session, stack); \
} while(0);

#define HANDLE_RETURN0 do	{ \
	stack = session->stack; \
	np = stack->return_point->next; \
	mkc_stack_pop(session); \
} while(0);

#define HANDLE_RETURN1 do	{ \
	stack = session->stack; \
	IF_ERROR_RETURN(mkc_assign(session, stack->result, left)); \
	np = stack->return_point->next; \
	mkc_stack_pop(session); \
} while(0);

STATIC int
mkc_assign(
		mkc_session * session, mkc_data * result, mkc_data * right)
{
	switch (right->type)	{
	case MKC_D_VOID:
		result->type = MKC_D_VOID;
		break;
	case MKC_D_INT:
		result->type = MKC_D_INT;
		result->integer = right->integer;
		break;
	case MKC_D_ID:
		Panic();
		break;
	case MKC_D_BUF:
		result->type = MKC_D_BUF;
		result->buffer = right->buffer;
		result->length = right->length;
		break;
	default:
		Panic();
		return -1;
	}

	return 0;
}

#define ASSIGN_OPERATE(__result)	do	{ \
	switch (right->type)	{ \
	case MKC_D_VOID: \
		result->type = MKC_D_VOID; \
		break; \
	case MKC_D_INT: \
		if (MKC_D_VOID == result->type)	{ \
			result->type = MKC_D_INT; \
			result->integer = right->integer; \
		} \
		else if (MKC_D_INT == result->type)	{ \
			__result; \
		} \
		else	{ \
			MKC_ERROR("runtime error: assign to a incompatible variable"); \
			return -1; \
		} \
		break; \
	case MKC_D_ID: \
	case MKC_D_BUF: \
		MKC_ERROR("runtime error: operation not supported"); \
		return -1; \
	default: \
		Panic(); \
		return -1; \
	} \
} while(0);

#define ASSIGN_NZ_OPERATE(__result)	do	{ \
	switch (right->type)	{ \
	case MKC_D_INT: \
		if (MKC_D_VOID == result->type)	{ \
			MKC_ERROR("runtime error: variable don't have a value"); \
			return -1; \
		} \
		else if (MKC_D_INT == result->type)	{ \
			if (!right->integer)	{ \
				MKC_ERROR("runtime error: divide by zero"); \
				return -1; \
			} \
			__result; \
		} \
		else	{ \
			MKC_ERROR("runtime error: assign to a incompatible variable"); \
			return -1; \
		} \
		break; \
	case MKC_D_BUF: \
		MKC_ERROR("runtime error: operation not supported"); \
		return -1; \
	default: \
		Panic(); \
		return -1; \
	} \
} while(0);

#define BIT_OPERATE(__result)	do	{ \
	int rv; \
	int lv; \
\
	if (MKC_D_INT != left->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at bit operation"); \
		return -1; \
	} \
\
	if (MKC_D_INT != right->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at bit operation"); \
		return -1; \
	} \
\
	rv = right->integer; \
	lv = left->integer; \
\
	result->type = MKC_D_INT; \
	result->integer = (__result); \
\
} while(0);

#define MATH_NZ_OPERATE(__result)	do	{ \
	int rv; \
	int lv; \
\
	if (MKC_D_INT != left->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at math operation"); \
		return -1; \
	} \
\
	if (MKC_D_INT != right->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at math operation"); \
		return -1; \
	} \
\
	rv = right->integer; \
	lv = left->integer; \
\
	if (!rv)	{ \
		MKC_ERROR("runtime error: divide by zero"); \
		return -1; \
	} \
\
	result->type = MKC_D_INT; \
	result->integer = (__result); \
\
} while(0);

#define MATH_OPERATE(__result)	do	{ \
	int rv; \
	int lv; \
\
	if (MKC_D_INT != left->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at math operation"); \
		return -1; \
	} \
\
	if (MKC_D_INT != right->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at math operation"); \
		return -1; \
	} \
\
	rv = right->integer; \
	lv = left->integer; \
\
	result->type = MKC_D_INT; \
	result->integer = (__result); \
\
} while(0);

#define UNARY_OPERATE(__result)	do	{ \
	int lv; \
\
	if (MKC_D_INT != left->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at unary operation"); \
		return -1; \
	} \
\
	lv = left->integer; \
\
	result->type = MKC_D_INT; \
	result->integer = (__result); \
\
} while(0);

#define SELF_UNARY_OPERATE(__result)	do	{ \
	int lv; \
\
	if (MKC_D_INT != left->type)	{ \
		MKC_ERROR("runtime error: unsupported data type at unary operation"); \
		return -1; \
	} \
\
	lv = left->integer; \
	left->integer = (__result); \
\
} while(0);

#define HANDLE_SWITCH	do	{ \
	int lv; \
	mkc_tree * condition; \
	LOCATE_LEFT \
	if (MKC_D_INT != left->type)	{ \
		MKC_ERROR("runtime error: unsupported data type in switch statement"); \
		return -1; \
	} \
	lv = left->integer; \
	SINGLE_LL_FOREACH(condition, next, p->right)	{ \
		if (!condition->left)	{ \
			np = condition->right; \
			break; \
		} \
		right = condition->left->data; \
		if (MKC_D_INT == right->type && right->integer == lv)	{ \
			np = condition->right; \
			break; \
		} \
	} \
} while(0);

int
mkc_machine(mkc_session * session, mkc_tree * sequence, int stack_depth)	{
	mkc_stack_frame * stack;
	mkc_tree_type type;
	mkc_tree end_label;
	mkc_tree * np;
	mkc_tree * p;
	int r = 0;

	mkc_data * left;
	mkc_data * right;
	mkc_data * result;

	end_label.type = MKC_T_LABEL;
	end_label.next = NULL;

	stack = mkc_new_stack(session, 0, stack_depth);
	if (!stack)	{
		return -1;
	}
	stack->return_point = &end_label;
	stack->result = &(session->result);
	mkc_stack_push(session, stack);
	session->global_stack = stack;

	for (p = sequence; p; p = np)	{
		np = p->next;
		type = p->type;
		#ifdef SHOW_MACHINE_EXECUTION
		printf("machine >> %s\n", mkc_tree_type_name(type));
		#endif
		switch (type)	{
		case MKC_T_MIN:
			UNEXPECTED;
		case MKC_T_IMMEDIATE_DATA:
		case MKC_T_ID:
			break;
		case MKC_T_OP_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			IF_ERROR_RETURN(mkc_assign(session, result, right));
			break;
		case MKC_T_OP_ADD_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer += right->integer);
			break;
		case MKC_T_OP_SUB_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer -= right->integer);
			break;
		case MKC_T_OP_MUL_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer *= right->integer);
			break;
		case MKC_T_OP_DIV_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_NZ_OPERATE(result->integer /= right->integer);
			break;
		case MKC_T_OP_MOD_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_NZ_OPERATE(result->integer /= right->integer);
			break;
		case MKC_T_OP_AND_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer &= right->integer);
			break;
		case MKC_T_OP_XOR_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer ^= right->integer);
			break;
		case MKC_T_OP_OR_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer |= right->integer);
			break;
		case MKC_T_OP_LEFT_SHIFT_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer <<= right->integer);
			break;
		case MKC_T_OP_RIGHT_SHIFT_ASSIGN:
			LOCATE_RIGHT
			LOCATE_RESULT
			ASSIGN_OPERATE(result->integer >>= right->integer);
			break;
		case MKC_T_OP_LOGIC_AND:
			LOCATE_RESULT
			LOCATE_LEFT
			result->type = MKC_D_INT;
			if (MKC_D_INT == left->type && left->integer)	{
				LOCATE_RIGHT
				if (MKC_D_INT == right->type && right->integer)	{
					result->integer = 1;
				}
				else	{
					result->integer = 0;
				}
			}
			else	{
				result->integer = 0;
			}
			break;
		case MKC_T_OP_LOGIC_OR:
			LOCATE_RESULT
			LOCATE_LEFT
			result->type = MKC_D_INT;
			if (MKC_D_INT == left->type && left->integer)	{
				result->integer = 1;
			}
			else	{
				LOCATE_RIGHT
				if (MKC_D_INT == right->type && right->integer)	{
					result->integer = 1;
				}
				else	{
					result->integer = 0;
				}
			}
			break;
		case MKC_T_OP_BIT_AND:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			BIT_OPERATE(lv & rv);
			break;
		case MKC_T_OP_BIT_OR:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			BIT_OPERATE(lv | rv);
			break;
		case MKC_T_OP_BIT_XOR:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			BIT_OPERATE(lv ^ rv);
			break;
		case MKC_T_OP_BIT_LEFT_SHIFT:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			BIT_OPERATE(lv << rv);
			break;
		case MKC_T_OP_BIT_RIGHT_SHIFT:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			BIT_OPERATE(lv >> rv);
			break;
		case MKC_T_OP_MATH_DIV:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_NZ_OPERATE(lv / rv)
			break;
		case MKC_T_OP_MATH_MUL:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE(lv * rv)
			break;
		case MKC_T_OP_MATH_MOD:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_NZ_OPERATE(lv % rv)
			break;
		case MKC_T_OP_MATH_ADD:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE(lv + rv)
			break;
		case MKC_T_OP_MATH_SUB:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE(lv - rv)
			break;
		case MKC_T_OP_CMP_EQUAL:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE((lv == rv))
			break;
		case MKC_T_OP_CMP_DIFF:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE((lv != rv))
			break;
		case MKC_T_OP_CMP_GREATER:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE((lv > rv))
			break;
		case MKC_T_OP_CMP_LESS:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE((lv < rv))
			break;
		case MKC_T_OP_CMP_GREATER_EQUAL:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE((lv >= rv))
			break;
		case MKC_T_OP_CMP_LESS_EQUAL:
			LOCATE_RIGHT
			LOCATE_LEFT
			LOCATE_RESULT
			MATH_OPERATE((lv <= rv))
			break;
		case MKC_T_OP_LOGIC_NOT:
			LOCATE_LEFT
			LOCATE_RESULT
			UNARY_OPERATE(!lv)
			break;
		case MKC_T_OP_BIT_NOT:
			LOCATE_LEFT
			LOCATE_RESULT
			UNARY_OPERATE(~lv)
			break;
		case MKC_T_OP_MATH_NAGTIVE:
			LOCATE_LEFT
			LOCATE_RESULT
			UNARY_OPERATE(-lv)
			break;
		case MKC_T_OP_MATH_PRE_INC:
			LOCATE_LEFT
			SELF_UNARY_OPERATE(++lv)
			break;
		case MKC_T_OP_MATH_PRE_DEC:
			LOCATE_LEFT
			SELF_UNARY_OPERATE(--lv)
			break;
		case MKC_T_OP_MATH_POST_INC:
			LOCATE_LEFT
			SELF_UNARY_OPERATE(++lv)
			break;
		case MKC_T_OP_MATH_POST_DEC:
			LOCATE_LEFT
			SELF_UNARY_OPERATE(--lv)
			break;
		case MKC_T_CALL:
			HANDLE_CALL
			break;
		case MKC_T_CONDITION:
			LOCATE_LEFT
			if (MKC_D_INT != left->type)	{
				MKC_TREE_ERROR(
						p
						, "%s"
						, "runtime error: condition data type is not integer"
						);
				return -1;
			}
			if (left->integer)	{
				if (p->left)	{
					np = p->left;
				}
			}
			else	{
				if (p->right)	{
					np = p->right;
				}
			}
			break;
		case MKC_T_GOTO:
		case MKC_T_BREAK:
		case MKC_T_CONTINUE:
			UNEXPECTED;
		case MKC_T_RETURN0:
			HANDLE_RETURN0;
			break;
		case MKC_T_RETURN1:
			LOCATE_LEFT
			HANDLE_RETURN1;
			break;
		case MKC_T_EXIT0:
			session->result.type = MKC_D_VOID;
			goto __exit_point;
		case MKC_T_EXIT1:
			LOCATE_LEFT
			mkc_assign(session, &(session->result), left);
			goto __exit_point;
		case MKC_T_LABEL:
			break;
		case MKC_T_SWITCH:
			HANDLE_SWITCH
			break;
		case MKC_T_SWITCH_BRANCH:
		case MKC_T_SCOPE:
		case MKC_T_FUNCTION_DEF:
		case MKC_T_FUNCTION:
		case MKC_T_CONST_ITEM:
		case MKC_T_VARIABLE_ITEM:
		case MKC_T_VARIABLE_ITEM_0:
			UNEXPECTED;
		case MKC_T_CALL_FUNCTION:
			HANDLE_CALL_FUNCTION
			break;
		case MKC_T_CALL_CB:
			HANDLE_CALL_CB
			break;
		case MKC_T_JUMP:
			np = p->left;
			break;
		case MKC_T_CALL_PARAM:
		default:
			UNEXPECTED;
		}
	}

__exit_point:

	return 0;
}

/* eof */
