/* vi: set sw=4 ts=4: */

%define api.pure
%lex-param		{mkc_session * session}
%parse-param	{mkc_session * session}
%name-prefix "mkc_"

%{

/* %define api.push-pull both */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/types.h"
#include "libs/misc.h"
#include "libs/ll.h"
#include "include/debug.h"
#include "include/internal.h"
#include "include/lex.h"

#define RETURN_NULL_ERROR(__op__)	do	{	\
	if (!__op__)	{						\
		YYERROR;							\
	}										\
}while(0);

#define RETURN_ERROR(__op__)	do	{	\
	if (__op__)	{						\
		YYERROR;						\
	}									\
}while(0);

#define TREE0(r__,op__)										\
	r__ = mkc_tree_new(session, op__, __FILE__, __LINE__);	\
	RETURN_NULL_ERROR(r__);

#define TREE1(r__,left__,op__)								\
	r__ = mkc_tree_new(session, op__, __FILE__, __LINE__);	\
	RETURN_NULL_ERROR(r__);									\
	r__->left = left__;

#define TREE2(r__,left__,right__,op__)						\
	r__ = mkc_tree_new(session, op__, __FILE__, __LINE__);	\
	RETURN_NULL_ERROR(r__);									\
	r__->left = left__;										\
	r__->right = right__;

#define TREE_SET_SUB(t__,sub__)	\
	t__->sub= sub__;

#define TREE_SET_LINENO(t__)	\
	t__->lineno = session->lineno;

STATIC int
mkc_error(mkc_session * session, char * s)	{
	MKC_ERROR("%s", s);
	return 0;
}

%}

%union {
	mkc_tree * tree;
}

%token ID DATA
%token LOGIC_OR LOGIC_AND
%token EQUAL DIFF GREATER_EQUAL LESS_EQUAL
%token LEFT_SHIFT RIGHT_SHIFT
%token IF ELSE
%token SWITCH CASE DEFAULT
%token DO WHILE FOR
%token BREAK CONTINUE RETURN EXIT
%token FUNCTION VAR CONST
%token INC DEC
%token GOTO
%token ERROR MEMORY
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN LEFT_SHIFT_ASSIGN RIGHT_SHIFT_ASSIGN 

%nonassoc XIF
%nonassoc ELSE

%right '=' ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN AND_ASSIGN
		XOR_ASSIGN OR_ASSIGN LEFT_SHIFT_ASSIGN RIGHT_SHIFT_ASSIGN 
%left LOGIC_OR
%left LOGIC_AND
%right '!'
%left '&' '|' '^' LEFT_SHIFT RIGHT_SHIFT
%right '~'
%left EQUAL DIFF '>' '<' GREATER_EQUAL LESS_EQUAL
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS

%type	<tree>	ID DATA
%type	<tree>	stmts stmt compond_stmt
%type	<tree>	goto_stmt label_stmt
%type	<tree>	BREAK CONTINUE RETURN EXIT
%type	<tree>	value lvalue
%type	<tree>	call call_param call_params
%type	<tree>	expr logic_expr bit_expr math_expr compare_expr
%type	<tree>	assign_expr inc_dec_expr
%type	<tree>	if_stmt if_stmt_condition
%type	<tree>	switch_stmt switch_stmt_condition switch_stmt_body
%type	<tree>	case_branches case_branch
%type	<tree>	case_branch_condition default_branch case_branch_stmt
%type	<tree>	do_while_stmt do_while_body do_while_condition
%type	<tree>	while_stmt while_condition
%type	<tree>	for_stmt for_expr
%type	<tree>	function_decl function_id function_params function_body
%type	<tree>	const_stmt const_item_list const_item
%type	<tree>	var_stmt var_item_list var_item

%start input

%%

input:
	stmts	{
		mkc_tree * function_def;
		mkc_tree * function;
		TREE2(function_def, NULL, $1, MKC_T_FUNCTION_DEF)
		TREE2(function, NULL, function_def, MKC_T_FUNCTION)
		session->tree = function;
	}
	;

stmts:
	/* nothing */	{
		$$ = NULL;
	}
	|
	stmts stmt	{
		$$ = mkc_tree_join($1, $2);
	}
	;

stmt:
	if_stmt	{
		$$ = $1;
	}
	|
	switch_stmt	{
		$$ = $1;
	}
	|
	do_while_stmt	{
		$$ = $1;
	}
	|
	while_stmt	{
		$$ = $1;
	}
	|
	for_stmt	{
		$$ = $1;
	}
	|
	function_decl	{
		$$ = $1;
	}
	|
	var_stmt ';'	{
		$$ = $1;
	}
	|
	const_stmt ';'	{
		$$ = $1;
	}
	|
	BREAK ';'	{
		TREE0($$, MKC_T_BREAK)
		TREE_SET_LINENO($$)
	}
	|
	CONTINUE ';'	{
		TREE0($$, MKC_T_CONTINUE)
		TREE_SET_LINENO($$)
	}
	|
	RETURN ';'	{
		TREE0($$, MKC_T_RETURN0)
		TREE_SET_LINENO($$)
	}
	|
	RETURN expr ';'	{
		TREE1($$, $2, MKC_T_RETURN1)
		TREE_SET_LINENO($$)
	}
	|
	EXIT ';'	{
		TREE0($$, MKC_T_EXIT0)
		TREE_SET_LINENO($$)
	}
	|
	EXIT expr ';'	{
		TREE1($$, $2, MKC_T_EXIT1)
		TREE_SET_LINENO($$)
	}
	|
	goto_stmt	{
		$$ = $1;
	}
	|
	label_stmt	{
		$$ = $1;
	}
	|
	compond_stmt	{
		$$ = $1;
	}
	|
	expr ';'	{
		$$ = $1;
	}
	|
	';'	{
		$$ = NULL;
	}
	;

compond_stmt:
	'{' stmts '}'	{
		TREE2($$, NULL, NULL, MKC_T_SCOPE)
		TREE_SET_SUB($$, $2)
	}

if_stmt:
	if_stmt_condition stmt %prec XIF	{
		mkc_tree * condition_tree;
		mkc_tree * end_label;
		TREE0(end_label, MKC_T_LABEL)
		TREE_SET_LINENO(end_label)
		TREE2(condition_tree, NULL, end_label, MKC_T_CONDITION)
		TREE_SET_SUB(condition_tree, $1)
		TREE_SET_LINENO(condition_tree)
		$$ = mkc_tree_join(condition_tree, $2);
		$$ = mkc_tree_join($$, end_label);
	}
	|
	if_stmt_condition stmt ELSE stmt	{
		mkc_tree * condition_tree;
		mkc_tree * jump_end;
		mkc_tree * false_label;
		mkc_tree * end_label;
		TREE0(false_label, MKC_T_LABEL)
		TREE_SET_LINENO(false_label)
		TREE0(end_label, MKC_T_LABEL)
		TREE_SET_LINENO(end_label)
		TREE1(jump_end, end_label, MKC_T_JUMP);
		TREE2(condition_tree, NULL, false_label, MKC_T_CONDITION)
		TREE_SET_SUB(condition_tree, $1)
		TREE_SET_LINENO(condition_tree)
		$$ = mkc_tree_join(condition_tree, $2);
		$$ = mkc_tree_join($$, jump_end);
		$$ = mkc_tree_join($$, false_label);
		$$ = mkc_tree_join($$, $4);
		$$ = mkc_tree_join($$, end_label);
	}
	;

if_stmt_condition:
	IF '(' expr ')'	{
		$$ = $3;
	}
	;

switch_stmt:
	switch_stmt_condition switch_stmt_body '}'	{
		mkc_tree * end_label;
		mkc_tree * switch_stmt;
		mkc_tree * jump_end;
		mkc_tree * p;
		TREE0(end_label, MKC_T_LABEL)
		TREE_SET_LINENO(end_label)
		TREE1(jump_end, end_label, MKC_T_JUMP);
		TREE2(switch_stmt, $1, $2, MKC_T_SWITCH)
		switch_stmt = mkc_tree_join(switch_stmt, jump_end);
		mkc_tree_loop_break($2);
		SINGLE_LL_FOREACH(p, next, $2)	{
			switch_stmt = mkc_tree_join(switch_stmt, p->right);
		}
		switch_stmt = mkc_tree_join(switch_stmt, end_label);
		TREE2($$, NULL, end_label, MKC_T_SCOPE)
		TREE_SET_SUB($$, switch_stmt)
	}
	;

switch_stmt_condition:
	SWITCH '(' expr ')' '{'	{
		$$ = $3;
	}
	;

switch_stmt_body:
	case_branches	{
		$$ = $1;
	}
	|
	default_branch	{
		$$ = $1;
	}
	|
	case_branches default_branch	{
		$$= mkc_tree_join($1, $2);
	}
	;

case_branches:
	case_branch	{
		$$ = $1;
	}
	|
	case_branches case_branch	{
		$$ = mkc_tree_join($1, $2);
	}
	;

case_branch:
	case_branch_condition case_branch_stmt	{
		TREE2($$, $1, $2, MKC_T_SWITCH_BRANCH)
	}
	;

default_branch:
	DEFAULT ':' case_branch_stmt	{
		TREE2($$, NULL, $3, MKC_T_SWITCH_BRANCH)
	}
	;

case_branch_condition:
	CASE DATA ':'	{
		$$ = $2;
	}
	;

case_branch_stmt:
	stmts	{
		mkc_tree * branch_label;
		TREE0(branch_label, MKC_T_LABEL)
		TREE_SET_LINENO(branch_label)
		$$ = mkc_tree_join(branch_label, $1);
	}
	;

do_while_stmt:
	do_while_body do_while_condition	{
		mkc_tree * end_label;
		mkc_tree * start_label;
		mkc_tree * continue_label;
		mkc_tree * condition_tree;
		mkc_tree * compond_tree;
		TREE0(start_label, MKC_T_LABEL)
		TREE_SET_LINENO(start_label)
		TREE0(end_label, MKC_T_LABEL)
		TREE_SET_LINENO(end_label)
		TREE0(continue_label, MKC_T_LABEL)
		TREE_SET_LINENO(continue_label)
		TREE2(condition_tree, start_label, end_label, MKC_T_CONDITION)
		TREE_SET_SUB(condition_tree, $2)
		TREE_SET_LINENO(condition_tree)
		compond_tree = mkc_tree_join(start_label, $1);
		compond_tree = mkc_tree_join(compond_tree, continue_label);
		compond_tree = mkc_tree_join(compond_tree, condition_tree);
		compond_tree = mkc_tree_join(compond_tree, end_label);
		TREE2($$, continue_label, end_label, MKC_T_SCOPE)
		TREE_SET_SUB($$, compond_tree)
	}
	;

do_while_body:
	DO stmt	{
		$$ = $2;
	}
	;

do_while_condition:
	WHILE '(' expr ')'	{
		$$ = $3;
	}
	;

while_stmt:
	while_condition stmt	{
		mkc_tree * condition_tree;
		mkc_tree * start_label;
		mkc_tree * end_label;
		mkc_tree * jump_start;
		mkc_tree * compond_tree;
		TREE0(start_label, MKC_T_LABEL)
		TREE_SET_LINENO(start_label)
		TREE0(end_label, MKC_T_LABEL)
		TREE_SET_LINENO(end_label)
		TREE2(condition_tree, NULL, end_label, MKC_T_CONDITION)
		TREE_SET_SUB(condition_tree, $1)
		TREE_SET_LINENO(condition_tree)
		TREE1(jump_start, start_label, MKC_T_JUMP);
		compond_tree = mkc_tree_join(start_label, condition_tree);
		compond_tree = mkc_tree_join(compond_tree, $2);
		compond_tree = mkc_tree_join(compond_tree, jump_start);
		compond_tree = mkc_tree_join(compond_tree, end_label);
		TREE2($$, start_label, end_label, MKC_T_SCOPE)
		TREE_SET_SUB($$, compond_tree)
	}
	;

while_condition:
	WHILE '(' expr ')'	{
		$$ = $3;
	}
	;

for_stmt:
	FOR '(' for_expr ';' for_expr ';' for_expr ')' stmt	{
		mkc_tree * condition_tree;
		mkc_tree * start_label;
		mkc_tree * end_label;
		mkc_tree * continue_label;
		mkc_tree * jump_start;
		mkc_tree * compond_tree;
		TREE0(start_label, MKC_T_LABEL)
		TREE_SET_LINENO(start_label)
		TREE0(end_label, MKC_T_LABEL)
		TREE_SET_LINENO(end_label)
		TREE0(continue_label, MKC_T_LABEL)
		TREE_SET_LINENO(continue_label)
		if ($5)	{
			TREE2(condition_tree, NULL, end_label, MKC_T_CONDITION)
			TREE_SET_SUB(condition_tree, $5)
		}
		else	{
			condition_tree = NULL;
		}
		TREE1(jump_start, start_label, MKC_T_JUMP);
		compond_tree = mkc_tree_join($3, start_label);
		compond_tree = mkc_tree_join(compond_tree, condition_tree);
		compond_tree = mkc_tree_join(compond_tree, $9);
		compond_tree = mkc_tree_join(compond_tree, continue_label);
		compond_tree = mkc_tree_join(compond_tree, $7);
		compond_tree = mkc_tree_join(compond_tree, jump_start);
		compond_tree = mkc_tree_join(compond_tree, end_label);
		TREE2($$, continue_label, end_label, MKC_T_SCOPE)
		TREE_SET_SUB($$, compond_tree)
	}
	;

for_expr:	{
		$$ = NULL;
	}
	|
	expr	{
		$$ = $1;
	}
	;

function_decl:
	function_id '(' function_params ')' function_body	{
		mkc_tree * function_def;
		TREE2(function_def, $3, $5, MKC_T_FUNCTION_DEF)
		TREE2($$, $1, function_def, MKC_T_FUNCTION)
	}
	;

function_id:
	FUNCTION ID	{
		$$ = $2;
	}

function_params:	{
		$$ = NULL;
	}
	|
	function_params ',' ID	{
		$$ = mkc_tree_join($1, $3);
	}
	|
	ID	{
		$$ = $1;
	}
	;

function_body:
	'{' stmts '}'	{
		mkc_tree * tree;
		if (!$2)	{
			TREE0(tree, MKC_T_RETURN0)
			TREE_SET_LINENO(tree)
		}
		else if (MKC_T_RETURN0 == $2->prev->type)	{
			tree = $2;
		}
		else if (MKC_T_RETURN1 == $2->prev->type)	{
			tree = $2;
		}
		else	{
			TREE0(tree, MKC_T_RETURN0)
			TREE_SET_LINENO(tree)
			tree = mkc_tree_join($2, tree);
		}
		TREE2($$, NULL, NULL, MKC_T_SCOPE)
		TREE_SET_SUB($$, tree)
	}
	;

goto_stmt:
	GOTO ID ';'	{
		TREE1($$, $2, MKC_T_GOTO)
	}
	;

label_stmt:
	ID ':'	{
		TREE1($$, $1, MKC_T_LABEL)
	}
	;

expr:
	call	{
		$$ = $1;
	}
	|
	'(' expr ')'	{
		$$ = $2;
	}
	|
	assign_expr	{
		$$ = $1;
	}
	|
	logic_expr	{
		$$ = $1;
	}
	|
	bit_expr	{
		$$ = $1;
	}
	|
	math_expr	{
		$$ = $1;
	}
	|
	compare_expr	{
		$$ = $1;
	}
	;

call:
	ID '(' call_params ')'	{
		TREE2($$, $1, $3, MKC_T_CALL)
		$$->lineno = $1->lineno;
	}
	;

call_params:
	/* nothing */	{
		$$ = NULL;
	
	}
	|
	call_param	{
		$$ = $1;
	}
	|
	call_params ',' call_param	{
		$$ = mkc_tree_join($$, $3);
	}
	;

call_param:
	expr	{
		TREE1($$, $1, MKC_T_CALL_PARAM)
	}
	;

const_stmt:
	CONST const_item_list	{
		$$ = $2;
	}
	;

const_item_list:
	const_item	{
		$$ = $1;
	}
	|
	const_item_list ',' const_item	{
		$$ = mkc_tree_join($1, $3);
	}
	;

const_item:
	ID '=' expr	{
		TREE2($$, $1, $3, MKC_T_CONST_ITEM)
	}
	;

var_stmt:
	VAR var_item_list	{
		$$ = $2;
	}
	;

var_item_list:
	var_item	{
		$$ = $1;
	}
	|
	var_item_list ',' var_item	{
		$$ = mkc_tree_join($1, $3);
	}
	;

var_item:
	ID '=' expr	{
		TREE2($$, $1, $3, MKC_T_VARIABLE_ITEM)
	}
	|
	ID	{
		TREE1($$, $1, MKC_T_VARIABLE_ITEM_0)
	}
	;

assign_expr:
	lvalue '=' expr	{
		TREE2($$, $1, $3, MKC_T_OP_ASSIGN)
	}
	|
	lvalue ADD_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_ADD_ASSIGN)
	}
	|
	lvalue SUB_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_SUB_ASSIGN)
	}
	|
	lvalue MUL_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_MUL_ASSIGN)
	}
	|
	lvalue DIV_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_DIV_ASSIGN)
	}
	|
	lvalue MOD_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_MOD_ASSIGN)
	}
	|
	lvalue AND_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_AND_ASSIGN)
	}
	|
	lvalue XOR_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_XOR_ASSIGN)
	}
	|
	lvalue OR_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_OR_ASSIGN)
	}
	|
	lvalue LEFT_SHIFT_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_LEFT_SHIFT_ASSIGN)
	}
	|
	lvalue RIGHT_SHIFT_ASSIGN expr	{
		TREE2($$, $1, $3, MKC_T_OP_RIGHT_SHIFT_ASSIGN)
	}
	;

lvalue:
	ID	{
		$$ = $1;
	}
	;

logic_expr:
	expr LOGIC_AND expr	{
		TREE2($$, $1, $3, MKC_T_OP_LOGIC_AND)
	}
	|
	expr LOGIC_OR expr	{
		TREE2($$, $1, $3, MKC_T_OP_LOGIC_OR)
	}
	|
	'!' expr	{
		TREE1($$, $2, MKC_T_OP_LOGIC_NOT)
	}
	;

bit_expr:
	expr '&' expr	{
		TREE2($$, $1, $3, MKC_T_OP_BIT_AND)
	}
	|
	expr '|' expr	{
		TREE2($$, $1, $3, MKC_T_OP_BIT_OR)
	}
	|
	expr '^' expr	{
		TREE2($$, $1, $3, MKC_T_OP_BIT_XOR)
	}
	|
	'~' expr	{
		TREE1($$, $2, MKC_T_OP_BIT_NOT)
	}
	|
	expr LEFT_SHIFT expr	{
		TREE2($$, $1, $3, MKC_T_OP_BIT_LEFT_SHIFT)
	}
	|
	expr RIGHT_SHIFT expr	{
		TREE2($$, $1, $3, MKC_T_OP_BIT_RIGHT_SHIFT)
	}
	;

math_expr:
	value	{
		$$ = $1;
	}
	|
	expr '/' expr	{
		TREE2($$, $1, $3, MKC_T_OP_MATH_DIV)
	}
	|
	expr '*' expr	{
		TREE2($$, $1, $3, MKC_T_OP_MATH_MUL)
	}
	|
	expr '%' expr	{
		TREE2($$, $1, $3, MKC_T_OP_MATH_MOD)
	}
	|
	expr '+' expr	{
		TREE2($$, $1, $3, MKC_T_OP_MATH_ADD)
	}
	|
	expr '-' expr	{
		TREE2($$, $1, $3, MKC_T_OP_MATH_SUB)
	}
	|
	'+' expr %prec UMINUS	{
		$$ = $2;
	}
	|
	'-' expr %prec UMINUS	{
		TREE1($$, $2, MKC_T_OP_MATH_NAGTIVE)
	}
	|
	inc_dec_expr	{
		$$ = $1;
	}
	;

value:
	ID	{
		$$ = $1;
	}
	|
	DATA	{
		$$ = $1;
	}
	;

inc_dec_expr:
	lvalue INC	{
		TREE1($$, $1, MKC_T_OP_MATH_POST_INC)
	}
	|
	lvalue DEC	{
		TREE1($$, $1, MKC_T_OP_MATH_POST_DEC)
	}
	|
	INC lvalue	{
		TREE1($$, $2, MKC_T_OP_MATH_PRE_INC)
	}
	|
	DEC lvalue	{
		TREE1($$, $2, MKC_T_OP_MATH_PRE_DEC)
	}
	;

compare_expr:
	expr EQUAL expr	{
		TREE2($$, $1, $3, MKC_T_OP_CMP_EQUAL)
	}
	|
	expr DIFF expr	{
		TREE2($$, $1, $3, MKC_T_OP_CMP_DIFF)
	}
	|
	expr '>' expr	{
		TREE2($$, $1, $3, MKC_T_OP_CMP_GREATER)
	}
	|
	expr '<' expr	{
		TREE2($$, $1, $3, MKC_T_OP_CMP_LESS)
	}
	|
	expr GREATER_EQUAL expr	{
		TREE2($$, $1, $3, MKC_T_OP_CMP_GREATER_EQUAL)
	}
	|
	expr LESS_EQUAL expr	{
		TREE2($$, $1, $3, MKC_T_OP_CMP_LESS_EQUAL)
	}
	;

%%

/* eof */
