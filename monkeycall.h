/* vi: set sw=4 ts=4: */

#ifndef __MONKEY_CALL_HEADER_INCLUDED__
#define __MONKEY_CALL_HEADER_INCLUDED__

#if defined(__cplusplus)
extern "C" {
#endif

#define _MKC_DS_ \
	__T(MKC_D_VOID), \
	__T(MKC_D_INT), \
	__T(MKC_D_ID), \
	__T(MKC_D_BUF), \
	__T(MKC_D_MAX), \

#define __T(__V)	__V
typedef enum	{
	_MKC_DS_
} mkc_data_type;
#undef __T

#define _MKC_TS_ \
	__T(MKC_T_MIN), \
	__T(MKC_T_IMMEDIATE_DATA), \
	__T(MKC_T_ID), \
	__T(MKC_T_OP_ASSIGN), \
	__T(MKC_T_OP_ADD_ASSIGN), \
	__T(MKC_T_OP_SUB_ASSIGN), \
	__T(MKC_T_OP_MUL_ASSIGN), \
	__T(MKC_T_OP_DIV_ASSIGN), \
	__T(MKC_T_OP_MOD_ASSIGN), \
	__T(MKC_T_OP_AND_ASSIGN), \
	__T(MKC_T_OP_XOR_ASSIGN), \
	__T(MKC_T_OP_OR_ASSIGN), \
	__T(MKC_T_OP_LEFT_SHIFT_ASSIGN), \
	__T(MKC_T_OP_RIGHT_SHIFT_ASSIGN), \
	__T(MKC_T_OP_LOGIC_AND), \
	__T(MKC_T_OP_LOGIC_OR), \
	__T(MKC_T_OP_BIT_AND), \
	__T(MKC_T_OP_BIT_OR), \
	__T(MKC_T_OP_BIT_XOR), \
	__T(MKC_T_OP_BIT_LEFT_SHIFT), \
	__T(MKC_T_OP_BIT_RIGHT_SHIFT), \
	__T(MKC_T_OP_MATH_DIV), \
	__T(MKC_T_OP_MATH_MUL), \
	__T(MKC_T_OP_MATH_MOD), \
	__T(MKC_T_OP_MATH_ADD), \
	__T(MKC_T_OP_MATH_SUB), \
	__T(MKC_T_OP_CMP_EQUAL), \
	__T(MKC_T_OP_CMP_DIFF), \
	__T(MKC_T_OP_CMP_GREATER), \
	__T(MKC_T_OP_CMP_LESS), \
	__T(MKC_T_OP_CMP_GREATER_EQUAL), \
	__T(MKC_T_OP_CMP_LESS_EQUAL), \
	__T(MKC_T_OP_LOGIC_NOT), \
	__T(MKC_T_OP_BIT_NOT), \
	__T(MKC_T_OP_MATH_NAGTIVE), \
	__T(MKC_T_OP_MATH_PRE_INC), \
	__T(MKC_T_OP_MATH_PRE_DEC), \
	__T(MKC_T_OP_MATH_POST_INC), \
	__T(MKC_T_OP_MATH_POST_DEC), \
	__T(MKC_T_CALL), \
	__T(MKC_T_CONDITION), \
	__T(MKC_T_GOTO), \
	__T(MKC_T_BREAK), \
	__T(MKC_T_CONTINUE), \
	__T(MKC_T_RETURN0), \
	__T(MKC_T_RETURN1), \
	__T(MKC_T_EXIT0), \
	__T(MKC_T_EXIT1), \
	__T(MKC_T_LABEL), \
	__T(MKC_T_SWITCH), \
	__T(MKC_T_SWITCH_BRANCH), \
	__T(MKC_T_SCOPE), \
	__T(MKC_T_FUNCTION), \
	__T(MKC_T_FUNCTION_DEF), \
	__T(MKC_T_CONST_ITEM), \
	__T(MKC_T_VARIABLE_ITEM), \
	__T(MKC_T_VARIABLE_ITEM_0), \
	__T(MKC_T_CALL_FUNCTION), \
	__T(MKC_T_CALL_CB), \
	__T(MKC_T_JUMP), \
	__T(MKC_T_CALL_PARAM), \
	__T(MKC_T_MAX), \

#define __T(__V)	__V
typedef enum	{
	_MKC_TS_
} mkc_tree_type;
#undef __T

typedef struct _mkc_data	{
	mkc_data_type	type;
	int length;
	union	{
		int			integer;
		const char	* id;
		void		* buffer;
	};
} mkc_data;

typedef enum	{
	MKC_STORAGE_NOVALUE,
	MKC_STORAGE_IMMEDIATE,
	MKC_STORAGE_TEMP,
	MKC_STORAGE_LOCAL,
	MKC_STORAGE_PARAMETER,
	MKC_STORAGE_GLOBAL,
	MKC_STORAGE_GLOBAL_PARAMETER,
} mkc_storage_type;

typedef struct _mkc_location	{
	mkc_storage_type	storage;
	int					index;
	mkc_data			* data;
} mkc_location;

typedef struct _mkc_tree	{
	mkc_tree_type		type;
	union	{
		int				lineno;
		int				param_list_length;
	};
	struct _mkc_tree	* next;
	struct _mkc_tree	* prev;
	struct _mkc_tree	* left;
	struct _mkc_tree	* right;
	union	{
		mkc_data			* data;
		void				* sequence;
		void				* symbol;
		void				* cb;
		struct _mkc_tree	* sub;
	};
	mkc_location		left_location;
	mkc_location		right_location;
	mkc_location		result_location;
} mkc_tree;

typedef struct	{
	const char	* str;
	int			pos;
	int			len;
} mkc_scanner;

typedef struct _mkc_session	{
	void			* instance;
	mkc_scanner		scanner;
	int				lineno;
	int				mm_count;
	int				err_buffer_length;
	char			* err_buffer;
	void			* mm_head;

	mkc_tree		* tree;

	void			* symbol_table;
	void			* brace_stack;
	mkc_tree		* current_function;
	void			* va;

	void			* stack;
	void			* global_stack;

	void			* label_table;
	mkc_tree		* backpatch_goto_list;

	mkc_tree		* last_result_expr_tree;

	mkc_data		result;

	void			* user_param0;
	void			* user_param1;
} mkc_session;

typedef struct _mkc_stack_frame	{
	unsigned long				depth;
	unsigned long				argc;
	mkc_data					* result;
	mkc_tree					* return_point;
	struct _mkc_stack_frame		* caller_stack;
	mkc_data					stack[];
} mkc_stack_frame;

typedef struct _mkc_cb_stack_frame	{
	unsigned long			depth;
	unsigned long			argc;
	mkc_data				* result;
	mkc_tree				* return_point;
	mkc_data				* stack[];
} mkc_cb_stack_frame;

/* interface that need to be provided by caller program */
extern void * mkc_user_provide_malloc(int, const char *, int);
extern void mkc_user_provide_free(void *);

/* mkc malloc interface */
extern void * mkc_session_do_malloc(mkc_session *, int, const char *, int);
#define mkc_session_malloc(__session__,__size__)							\
	 mkc_session_do_malloc(__session__, __size__, __FILE__, __LINE__)
extern void mkc_session_free(mkc_session *, void * p);

extern void * mkc_new_instance(void);
extern void mkc_free_instance(void *);
extern int mkc_buildin_cbs_init(void * instance);
extern mkc_session * mkc_new_session(void * instance, const char *, int);
extern void mkc_free_session(mkc_session *);
extern void mkc_session_set_user_param0(mkc_session * session, void * p);
extern void mkc_session_set_user_param1(mkc_session * session, void * p);
extern void mkc_session_set_error_info_buffer(mkc_session *, char *, int);
extern int mkc_parse(mkc_session *);
extern int mkc_go(mkc_session *);

#define MALLOC(__size__) mkc_user_provide_malloc(__size__, __FILE__, __LINE__)
#define FREE(__ptr__) mkc_user_provide_free(__ptr__)

#define SMALLOC(__structtype__)												\
	(__structtype__ *)mkc_session_do_malloc(session, sizeof(__structtype__), __FILE__, __LINE__);

#define SFREE(__block)	mkc_session_free(session, __block)

typedef int(* mkc_cb_t )(mkc_session *, mkc_cb_stack_frame *); 
extern mkc_cb_t mkc_cbtree_lookup(void * instance, const char * id);
extern mkc_cb_t mkc_cbtree_lookup2(void * instance, const char * id, int len);
extern int mkc_cbtree_add2(void * instance, const char * id, int len, mkc_cb_t);
extern int mkc_cbtree_add(void * instance, const char * id, mkc_cb_t);
extern int mkc_cbtree_del2(void * instance, const char * id, int len);
extern int mkc_cbtree_del(void * instance, const char * id);
extern const char * mkc_cbtree_get_first_id(
		void * instance, char * buf, int len);
extern const char * mkc_cbtree_get_next_id(
		void * instance, const char * id, char * buf, int len);
extern void mkc_dump_hex(const char * b, int l);
extern const char * mkc_data_type_name(mkc_data_type type);

#define MKC_CB_ARGC			(mkc_cb_stack->argc)
#define MKC_CB_ARGV(__idx)	(mkc_cb_stack->stack[__idx])
#define MKC_CB_RESULT		(mkc_cb_stack->result);

#define MKC_CB_ERROR(__fmt__, ...)	do	{ \
	int __r; \
	if (session->err_buffer && session->err_buffer_length)	{ \
		__r = snprintf(session->err_buffer, session->err_buffer_length, \
				"line %d: ", mkc_cb_stack->return_point->lineno); \
		if (__r > 0 && __r < session->err_buffer_length)	{ \
			snprintf(session->err_buffer + __r \
					, session->err_buffer_length - __r \
					, __fmt__, ##__VA_ARGS__); \
		} \
	} \
}while(0)

#define MKC_CB_ARGC_CHECK(__number__)	do	{ \
	int __r; \
	if (MKC_CB_ARGC == __number__)	{ \
		break; \
	} \
	if (session->err_buffer && session->err_buffer_length)	{ \
		__r = snprintf(session->err_buffer, session->err_buffer_length, \
				"line %d: ", mkc_cb_stack->return_point->lineno); \
		if (__r > 0 && __r < session->err_buffer_length)	{ \
			snprintf(session->err_buffer + __r \
					, session->err_buffer_length - __r \
					, "runtime error: parameter number should be %d" \
					, __number__); \
		} \
	} \
	return -1; \
}while(0)

#define MKC_CB_ARG_TYPE_CHECK(__index__,__type__)	do	{ \
	int __r; \
	mkc_data * ccatc; \
	ccatc = MKC_CB_ARGV(__index__); \
	if (__type__ == ccatc->type)	{ \
		break; \
	} \
	if (session->err_buffer && session->err_buffer_length)	{ \
		__r = snprintf(session->err_buffer, session->err_buffer_length, \
				"line %d: ", mkc_cb_stack->return_point->lineno); \
		if (__r > 0 && __r < session->err_buffer_length)	{ \
			snprintf(session->err_buffer + __r \
					, session->err_buffer_length - __r \
					, "runtime error: type of parameter %d is %s, " \
							"should be %s" \
					, __index__ + 1\
					, mkc_data_type_name(ccatc->type) \
					, mkc_data_type_name(__type__) \
					); \
		} \
	} \
	return -1; \
} while(0)

#define MKC_CB_USER_PARAM0	session->user_param0
#define MKC_CB_USER_PARAM1	session->user_param1

#if defined(__cplusplus)
}
#endif

#endif /* __mkc_HEADER_INCLUDED__ */
