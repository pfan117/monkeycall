/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "monkeycall.h"
#include "include/types.h"
#include "include/debug.h"
#include "include/internal.h"
#include "libs/misc.h"
#include "libs/ll.h"
#include "libs/tree.h"
#include "include/symbol-table.h"
#include "include/var-allocate.h"

#define GET_RB_ROOT (struct MKC_SYMBOL *)&(session->symbol_table)

typedef struct _mkc_symbol_brace	{
	struct _mkc_symbol_brace	* next;
	mkc_symbol					* list;
	mkc_tree					* continue_label;
	mkc_tree					* break_label;
} mkc_symbol_brace;

typedef struct _mkc_symbol_name	{
	mkc_symbol					* list;
	mkc_data					* id;
	u64							cs0;
	u64							cs1;
	RB_ENTRY(_mkc_symbol_name)	rb;
} mkc_symbol_name;

STATIC int
__mkc_symbol_cmp(mkc_symbol_name * a, mkc_symbol_name * b)	{
	u64 csa;
	u64 csb;
	int la;
	int lb;

	csa = a->cs0;
	csb = b->cs0;
	if (csa != csb)	{
		if (csa > csb)	{
			return 1;
		}
		else	{
			return -1;
		}
	}

	csa = a->cs1;
	csb = b->cs1;
	if (csa != csb)	{
		if (csa > csb)	{
			return 1;
		}
		else	{
			return -1;
		}
	}

	la = a->id->length;
	lb = b->id->length;
	if (la != lb)	{
		if (la > lb)	{
			return 1;
		}
		else	{
			return -1;
		}
	}

	return memcmp(a->id->id, b->id->id, la);
}

RB_HEAD(MKC_SYMBOL, _mkc_symbol_name);
RB_PROTOTYPE_STATIC(MKC_SYMBOL, _mkc_symbol_name, rb, __mkc_symbol_cmp);
RB_GENERATE_STATIC(MKC_SYMBOL, _mkc_symbol_name, rb, __mkc_symbol_cmp);

STATIC void
__mkc_symbol_checksum(mkc_symbol_name * name)	{
	const char * uc;
	unsigned char u;
	u64 cs0;
	u64 cs1;
	int len;
	int i;

	uc = (const char *)(name->id->id);
	len = name->id->length;
	cs0 = 0U;
	cs1 = 0U;

	FOR(i, len)	{
		u = uc[i];
		cs0 = (cs0 << 1) + u;
		cs1 = (cs1 << 2) + u;
	}

	name->cs0 = cs0;
	name->cs1 = cs1;

	return;
}

STATIC int
__mkc_symbol_table_del_p(mkc_session * session, mkc_symbol * symbol)
{
	mkc_symbol_name * name;

	name = symbol->name;
	DOUBLE_LL_AWAY(symbol, overload_next, overload_prev, name->list);
	mkc_session_free(session, symbol);
	if (!name->list)	{
		MKC_SYMBOL_RB_REMOVE(GET_RB_ROOT, name);
		SFREE(name);
	}

	return 0;
}

int
mkc_symbol_enter_brace(mkc_session * session)	{
	mkc_symbol_brace * brace;

	brace = SMALLOC(mkc_symbol_brace);
	if (!brace)	{
		MKC_ERROR("no memory for parsing");
		return -1;
	}

	brace->list = NULL;
	brace->next = session->brace_stack;
	brace->continue_label = NULL;
	brace->break_label = NULL;
	session->brace_stack = brace;

	return 0;
}

void
mkc_symbol_set_brace_labels(
		mkc_session * session
		, mkc_tree * continue_label
		, mkc_tree * break_label
		)
{
	mkc_symbol_brace * brace;
	brace = session->brace_stack;
	brace->continue_label = continue_label;
	brace->break_label = break_label;
	return;
}

mkc_tree *
mkc_symbol_get_continue_label(mkc_session * session)	{
	mkc_symbol_brace * brace;
	mkc_tree * label;

	SINGLE_LL_FOREACH(brace, next, session->brace_stack)	{
		label = brace->continue_label;
		if (label)	{
			return label;
		}
	}

	return NULL;
}

mkc_tree *
mkc_symbol_get_break_label(mkc_session * session)	{
	mkc_symbol_brace * brace;
	mkc_tree * label;

	SINGLE_LL_FOREACH(brace, next, session->brace_stack)	{
		label = brace->break_label;
		if (label)	{
			return label;
		}
	}

	return NULL;
}

int
mkc_symbol_exit_brace(mkc_session * session)	{
	mkc_symbol_brace * brace;
	mkc_symbol * n;
	mkc_symbol * p;

	brace = session->brace_stack;

	for (p = brace->list; p; p = n)	{
		n = p->brace_next;
		__mkc_symbol_table_del_p(session, p);
	}

	session->brace_stack = brace->next;
	SFREE(brace);

	return 0;
}

int
mkc_symbol_in_bottom_brace(mkc_session * session)	{
	mkc_symbol_brace * brace;

	brace = session->brace_stack;
	if (brace)	{
		if (brace->next)	{
			return 0;
		}
		else	{
			return 1;
		}
	}
	else	{
		return 0;
	}
}

STATIC mkc_symbol_name *
__mkc_symbol_name_lookup(mkc_session * session, mkc_symbol_name * pattern)
{
	mkc_symbol_name * name;
	name = MKC_SYMBOL_RB_FIND(GET_RB_ROOT, pattern);
	return name;
}

STATIC mkc_symbol_name *
mkc_symbol_name_lookup(mkc_session * session, mkc_data * id)	{
	mkc_symbol_name pattern;
	mkc_symbol_name * name;

	pattern.id = id;
	__mkc_symbol_checksum(&pattern);
	name = __mkc_symbol_name_lookup(session, &pattern);
	return name;
}

mkc_symbol *
mkc_symbol_lookup_function(mkc_session * session, mkc_data * id)	{
	mkc_symbol_name * name;
	mkc_symbol * p;

	name = mkc_symbol_name_lookup(session, id);
	if (!name)	{
		return NULL;
	}

	p = name->list;
	if (MKC_SYMBOL_FUNCTION == p->type)	{
		return p;
	}

	return NULL;
}

STATIC void
__mkc_symbol_name_join(mkc_session * session, mkc_symbol_name * name)
{
	MKC_SYMBOL_RB_INSERT(GET_RB_ROOT, name);
	return;
}

STATIC void
__mkc_symbol_join_name(mkc_symbol_name * name, mkc_symbol * symbol)	{
	DOUBLE_LL_JOIN(symbol, overload_next, overload_prev, name->list);
	symbol->name = name;
	return;
}

STATIC void
__mkc_symbol_join_brace(mkc_symbol_brace * brace, mkc_symbol * symbol)	{
	SINGLE_LL_JOIN(symbol, brace_next, brace->list);
	symbol->brace = brace;
	return;
}

STATIC int
__mkc_symbol_name_can_be_overload(
		mkc_session * session
		, mkc_symbol_name * exist
		)
{
	if (session->brace_stack == exist->list->brace)	{
		return 0;
	}
	else	{
		return 1;
	}
}

STATIC mkc_symbol *
__mkc_symbol_add(mkc_session * session, mkc_tree * id)
{
	mkc_symbol_name * exist;
	mkc_symbol_name * name;
	mkc_symbol * symbol;
	mkc_data * data;

	data = id->data;

	name = SMALLOC(mkc_symbol_name);
	if (!name)	{
		MKC_TREE_ERROR(id, "no memory for symbol '%s'", data->id);
		return NULL;
	}

	symbol = SMALLOC(mkc_symbol);
	if (!symbol)	{
		SFREE(name);
		MKC_TREE_ERROR(id, "no memory for symbol '%s'", data->id);
		return NULL;
	}

	name->id = id->data;
	name->list = NULL;
	__mkc_symbol_checksum(name);
	exist = __mkc_symbol_name_lookup(session, name);
	if (exist)	{
		SFREE(name);
		if (__mkc_symbol_name_can_be_overload(session, exist))	{
			name = exist;
		}
		else	{
			SFREE(symbol);
			MKC_TREE_ERROR(id, "symbol redefined '%s'", data->id);
			return NULL;
		}
	}
	else	{
		__mkc_symbol_name_join(session, name);
	}

	__mkc_symbol_join_name(name, symbol);
	__mkc_symbol_join_brace(session->brace_stack, symbol);
	symbol->location_function = session->current_function;

	return symbol;
}

int
mkc_symbol_is_global(mkc_symbol * v)	{
	mkc_symbol_brace * brace;

	brace = v->brace;
	if (brace->next)	{
		return 0;
	}
	else	{
		return 1;
	}
}

int
mkc_symbol_add_const(mkc_session * session, mkc_tree * ns)	{
	mkc_symbol * symbol;
	int idx;

	idx = mkc_va_alloc(session);
	if (idx < 0)	{
		return -1;
	}

	symbol = __mkc_symbol_add(session, ns);
	if (!symbol)	{
		return -1;
	}

	symbol->type = MKC_SYMBOL_CONST;
	symbol->variable.stack_index = idx;
	ns->result_location.storage = MKC_STORAGE_LOCAL;
	ns->result_location.index = idx;

	return 0;
}

int
mkc_symbol_add_variable(mkc_session * session, mkc_tree * ns)	{
	mkc_symbol * symbol;
	int idx;

	idx = mkc_va_alloc(session);
	if (idx < 0)	{
		return -1;
	}

	symbol = __mkc_symbol_add(session, ns);
	if (!symbol)	{
		return -1;
	}

	symbol->type = MKC_SYMBOL_VARIABLE;
	symbol->variable.stack_index = idx;

	return 0;
}

int
mkc_symbol_add_parameter(mkc_session * session, mkc_tree * ns, int idx)	{
	mkc_symbol * symbol;

	symbol = __mkc_symbol_add(session, ns);
	if (!symbol)	{
		return -1;
	}

	symbol->type = MKC_SYMBOL_PARAMETER;
	symbol->variable.stack_index = idx;

	return 0;
}

int
mkc_symbol_add_function(mkc_session * session, mkc_tree * ns)	{
	mkc_symbol * symbol;

	if (mkc_symbol_in_bottom_brace(session))	{
		;
	}
	else	{
		MKC_TREE_ERROR(ns->left
				, "function definition only allowed at bottom scope"
				);
		return -1;
	}

	symbol = __mkc_symbol_add(session, ns->left);
	if (!symbol)	{
		return -1;
	}

	symbol->type = MKC_SYMBOL_FUNCTION;
	symbol->function.tree = ns;

	return 0;
}

int
mkc_symbol_locate_lvalue(mkc_session * session, mkc_tree * id)	{
	mkc_symbol_name * name;
	mkc_symbol * symbol;
	int idx;

	name = mkc_symbol_name_lookup(session, id->data);
	if (name)	{
		symbol = name->list;
		switch(symbol->type)	{
		case MKC_SYMBOL_VARIABLE:
			goto __use_exist;
		case MKC_SYMBOL_CONST:
			goto __try_overload;
		case MKC_SYMBOL_PARAMETER:
			goto __use_parameter;
		case MKC_SYMBOL_FUNCTION:
			goto __try_overload;
		default:
			MKC_INTERNAL_ERROR;
			return -1;
		}
	}
	else	{
		name = SMALLOC(mkc_symbol_name);
		if (!name)	{
			MKC_TREE_ERROR(id, "no memory for symbol '%s'", id->data->id);
			return -1;
		}
		name->id = id->data;
		name->list = NULL;
		__mkc_symbol_checksum(name);
		__mkc_symbol_name_join(session, name);
		goto __create_new;
	}

__try_overload:
	if (!__mkc_symbol_name_can_be_overload(session, name))	{
		MKC_TREE_ERROR(id, "'%s' is not a variable and cannot be overloaded"
				, id->data->id
				);
		return -1;
	}

__create_new:

	idx = mkc_va_alloc(session);
	if (idx < 0)	{
		return -1;
	}

	symbol = SMALLOC(mkc_symbol);
	if (!symbol)	{
		SFREE(name);
		MKC_ERROR("%s", "out of memory");
		return -1;
	}

	__mkc_symbol_join_name(name, symbol);
	__mkc_symbol_join_brace(session->brace_stack, symbol);
	symbol->location_function = session->current_function;
	symbol->type = MKC_SYMBOL_VARIABLE;
	symbol->variable.stack_index = idx;

__use_exist:

	if (symbol->location_function == session->current_function)	{
		id->result_location.storage = MKC_STORAGE_LOCAL;
	}
	else	{
		id->result_location.storage = MKC_STORAGE_GLOBAL;
	}
	id->result_location.index = symbol->variable.stack_index;

	return 0;

__use_parameter:

	if (symbol->location_function == session->current_function)	{
		id->result_location.storage = MKC_STORAGE_PARAMETER;
	}
	else	{
		id->result_location.storage = MKC_STORAGE_GLOBAL_PARAMETER;
	}
	id->result_location.index = symbol->variable.stack_index;

	return 0;
}

int
mkc_symbol_locate_rvalue(mkc_session * session, mkc_tree * id)	{
	mkc_symbol_name * name;
	mkc_symbol * symbol;

	name = mkc_symbol_name_lookup(session, id->data);
	if (name)	{
		symbol = name->list;
		switch(symbol->type)	{
		case MKC_SYMBOL_VARIABLE:
		case MKC_SYMBOL_CONST:
			goto __use_exist;
		case MKC_SYMBOL_PARAMETER:
			goto __use_parameter;
		case MKC_SYMBOL_FUNCTION:
			goto __try_overload;
		default:
			MKC_INTERNAL_ERROR;
			return -1;
		}
	}
	else	{
		MKC_TREE_ERROR(id, "'%s' doesn't exist", id->data->id);
		return -1;
	}

__try_overload:
	MKC_TREE_ERROR(id, "'%s' is not a valid right value", id->data->id);
	return -1;

__use_exist:

	if (symbol->location_function == session->current_function)	{
		id->result_location.storage = MKC_STORAGE_LOCAL;
	}
	else	{
		id->result_location.storage = MKC_STORAGE_GLOBAL;
	}
	id->result_location.index = symbol->variable.stack_index;

	return 0;

__use_parameter:

	if (symbol->location_function == session->current_function)	{
		id->result_location.storage = MKC_STORAGE_PARAMETER;
	}
	else	{
		id->result_location.storage = MKC_STORAGE_GLOBAL_PARAMETER;
	}
	id->result_location.index = symbol->variable.stack_index;

	return 0;
}

int
mkc_symbol_locate_variable(mkc_session * session, mkc_tree * id)	{
	mkc_symbol_name * name;
	mkc_symbol * symbol;

	name = mkc_symbol_name_lookup(session, id->data);
	if (name)	{
		symbol = name->list;
		switch(symbol->type)	{
		case MKC_SYMBOL_VARIABLE:
			goto __use_exist;
		case MKC_SYMBOL_CONST:
			goto __try_overload;
		case MKC_SYMBOL_PARAMETER:
			goto __use_parameter;
		case MKC_SYMBOL_FUNCTION:
			goto __try_overload;
		default:
			MKC_INTERNAL_ERROR;
			return -1;
		}
	}
	else	{
		MKC_TREE_ERROR(id, "'%s' doesn't exist", id->data->id);
		return -1;
	}

__try_overload:
	MKC_TREE_ERROR(id, "'%s' is not a variable", id->data->id);
	return -1;

__use_exist:

	if (symbol->location_function == session->current_function)	{
		id->result_location.storage = MKC_STORAGE_LOCAL;
	}
	else	{
		id->result_location.storage = MKC_STORAGE_GLOBAL;
	}
	id->result_location.index = symbol->variable.stack_index;

	return 0;

__use_parameter:

	if (symbol->location_function == session->current_function)	{
		id->result_location.storage = MKC_STORAGE_PARAMETER;
	}
	else	{
		id->result_location.storage = MKC_STORAGE_GLOBAL_PARAMETER;
	}
	id->result_location.index = symbol->variable.stack_index;

	return 0;
}

const char *
mkc_get_id_from_symbol(mkc_symbol * symbol)	{
	mkc_symbol_name * name;
	name = symbol->name;
	return name->id->id;
}

/* eof */
