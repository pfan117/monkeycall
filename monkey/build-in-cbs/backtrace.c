/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"
#include "include/symbol-table.h"
#include "include/stack.h"

STATIC void
__mkc_cb_backtrace_output_parameter(mkc_stack_frame * mkc_cb_stack)	{
	mkc_data * p;
	int argc;
	int i;

	argc = MKC_CB_ARGC;
	if (!argc)	{
		return;
	}

	for (i = 0; i < argc; i ++)	{
		if (i)	{
			printf(", ");
		}
		p = mkc_find_parameter_from_stack(mkc_cb_stack, i);
		switch (p->type)	{
		case MKC_D_VOID:
			printf("?");
			break;
		case MKC_D_INT:
			printf("%d", p->integer);
			break;
		case MKC_D_ID:
			printf("?, ");
			break;
		case MKC_D_BUF:
			printf("%p", p->buffer);
			break;
		case MKC_D_MAX:
		default:
			printf("?");
			break;
		}
	}

	return;
}

#define Backtrace

int
mkc_cb_backtrace(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_symbol * func;
	mkc_data * result;
	mkc_tree * return_point;
	mkc_stack_frame * stack;
	mkc_stack_frame * upper_stack;

	for (stack = (mkc_stack_frame *)mkc_cb_stack; stack; stack = upper_stack)
	{
		return_point = stack->return_point;
		if (return_point)	{
			if (MKC_T_CALL_FUNCTION == return_point->type)	{
				func = return_point->symbol;
				printf("line %d: %s("
						, return_point->lineno
						, mkc_get_id_from_symbol(func)
						);
				__mkc_cb_backtrace_output_parameter(stack);
				printf(");\n");
				upper_stack = stack->caller_stack;
			}
			else if (MKC_T_CALL_CB == return_point->type)	{
				printf("line %d: backtrace();\n", return_point->lineno);
				upper_stack = session->stack;
			}
			else	{
				break;
			}
		}
		else	{
			break;
		}
	}

	result = mkc_cb_stack->result;
	result->type = MKC_D_VOID;
	return 0;
}

/* eof */
