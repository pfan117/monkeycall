/* vi: set sw=4 ts=4: */

#ifndef __MKC_SYMBOL_TABLE_INCLUDED__
#define __MKC_SYMBOL_TABLE_INCLUDED__

typedef enum _mkc_symbol_type {
	MKC_SYMBOL_VARIABLE,
	MKC_SYMBOL_CONST,
	MKC_SYMBOL_PARAMETER,
	MKC_SYMBOL_FUNCTION,
} mkc_symbol_type;

typedef struct _mkc_symbol_function	{
	mkc_tree	* tree;
	int			param_cnt;
	int			stack_depth;
} mkc_symbol_function;

typedef struct _mkc_symbol_variable	{
	int	stack_index;
} mkc_symbol_variable;

typedef struct _mkc_symbol_label	{
	mkc_tree	* tree;
} mkc_symbol_label;

typedef struct _mkc_symbol	{
	mkc_symbol_type		type;
	struct _mkc_symbol	* overload_next;
	struct _mkc_symbol	* overload_prev;
	struct _mkc_symbol	* brace_next;
	void				* brace;
	void				* name;
	mkc_tree			* location_function;
	union	{
		mkc_symbol_function	function;
		mkc_symbol_variable	variable;
		mkc_symbol_label	label;
	};
} mkc_symbol;

extern int mkc_symbol_enter_brace(mkc_session * session);
extern int mkc_symbol_exit_brace(mkc_session * session);
extern int mkc_symbol_in_bottom_brace(mkc_session * session);
extern void mkc_symbol_set_brace_labels(mkc_session *, mkc_tree *, mkc_tree *);
extern mkc_tree * mkc_symbol_get_continue_label(mkc_session * session);
extern mkc_tree * mkc_symbol_get_break_label(mkc_session * session);
extern int mkc_symbol_add_function(mkc_session *, mkc_tree *);
extern int mkc_symbol_add_const(mkc_session * , mkc_tree *);
extern int mkc_symbol_add_variable(mkc_session *, mkc_tree *);
extern int mkc_symbol_add_parameter(mkc_session *, mkc_tree *, int);
extern int mkc_symbol_locate_lvalue(mkc_session *, mkc_tree * id);
extern int mkc_symbol_locate_rvalue(mkc_session *, mkc_tree * id);
extern int mkc_symbol_locate_variable(mkc_session * , mkc_tree * id);
extern int mkc_symbol_add_const(mkc_session *, mkc_tree *);

extern mkc_symbol * mkc_symbol_lookup_function(mkc_session *, mkc_data * id);
extern int mkc_symbol_is_global(mkc_symbol * v);
extern const char * mkc_get_id_from_symbol(mkc_symbol * symbol);

#endif

/* eof */
