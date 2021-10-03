/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/types.h"
#include "include/internal.h"
#include "libs/misc.h"
#include "_parser.h"
#include "include/lex.h"
#include "include/debug.h"

#define LINENO	session->lineno
#define SET_TREE_LINENO	tree->lineno = session->lineno

#define IS_NUMBER(__c__)	((__c__) >= '0' && (__c__) <= '9')
#define IS_ALPHABET(__c__)	((__c__) >= 'A' && (__c__) <= 'Z')
#define IS_alphabet(__c__)	((__c__) >= 'a' && (__c__) <= 'z')

/* smd calc may return many kinds of errors */
#define _mkc_KT_				\
	__TT(break,		BREAK), 	\
	__TT(case,		CASE), 		\
	__TT(const,		CONST), 	\
	__TT(constant,	CONST), 	\
	__TT(continue,	CONTINUE), 	\
	__TT(default,	DEFAULT), 	\
	__TT(do,		DO), 		\
	__TT(else,		ELSE), 		\
	__TT(exit,		EXIT), 		\
	__TT(for,		FOR), 		\
	__TT(function,	FUNCTION),	\
	__TT(goto,		GOTO), 		\
	__TT(if,		IF), 		\
	__TT(return,	RETURN), 	\
	__TT(switch,	SWITCH), 	\
	__TT(var,		VAR), 		\
	__TT(variable,	VAR), 		\
	__TT(while,		WHILE),

#define __TT(__K,__T)	s_##__K##__T
typedef enum	{
	_mkc_KT_
} _mkc_keyword_seq;
#undef __TT

typedef struct {
	cc * str;
	int token;
} _mkc_keyword_token;

#define __TT(__K,__T)	{ #__K, __T }
STATIC const
_mkc_keyword_token lc_keyword_token[] = {
	_mkc_KT_
};
#undef __TT

#define mkc_MAX_KEYWORD_MATCH	4
STATIC int
mkc_match_keyword_or_id(
		YYSTYPE * yylval, mkc_session * session
		, int kt_start, int cnt)
{
	int seq[mkc_MAX_KEYWORD_MATCH];
	mkc_scanner * scanner;
	mkc_tree * tree;
	mkc_data * data;
	char * name;
	cc * str;
	int len;
	int wi;		/* writing index */
	int ri;		/* reading index */
	int mc;		/* match count */
	char c;
	int i;

	#if debug
	if (kt_start > s_whileWHILE)	{
		Panic();
	}

	if (cnt > mkc_MAX_KEYWORD_MATCH)	{
		Panic();
	}
	#endif

	scanner = &(session->scanner);
	len = scanner->len - scanner->pos;
	str = scanner->str + scanner->pos;
	mc = cnt;
	FOR(i, cnt)	{
		seq[i] = kt_start + i;
	}

	for (i = 0; i < len; i ++)	{
		c = str[i];

		if (' ' == c || '\t' == c)	{
			goto __token_finish;
		}
		else if ('\r' == c || '\n' == c)	{
			LINENO ++;
			goto __token_finish;
		}
		else if (IS_alphabet(c))	{
			;
		}
		else if (IS_ALPHABET(c))	{
			;
		}
		else if (IS_NUMBER(c))	{
			;
		}
		else if ('_' == c)	{
			;
		}
		else	{
			goto __token_finish;
		}

		for (ri = 0; ri < mc;)	{
			if (lc_keyword_token[seq[ri]].str[i] == c)	{
				ri ++;
			}
			else	{
				for (wi = ri; wi < mc - 1; wi ++)	{
					seq[wi] = seq[wi + 1];
				}
				mc --;
			}
		}
	}

__token_finish:

	scanner->pos += i;
	if (!mc)	{
		goto __match_id;
	}

	FOR(ri, mc)	{
		if (lc_keyword_token[seq[ri]].str[i])	{
			continue;
		}
		return lc_keyword_token[seq[ri]].token;
	}

__match_id:

	name = mkc_session_malloc(session, i + 1);
	if (!name)	{
		return MEMORY;
	}

	data = mkc_data_new(session, MKC_D_ID, __FILE__, __LINE__);
	if (!data)	{
		return MEMORY;
	}

	tree = mkc_tree_new(session, MKC_T_ID, __FILE__, __LINE__);
	if (!tree)	{
		return MEMORY;
	}

	memcpy(name, str, i);
	name[i] = '\0';
	data->id = name;
	data->length = i;
	tree->data = data;
	SET_TREE_LINENO;
	yylval->tree = tree;

	return ID;
}

STATIC int
mkc_match_id(YYSTYPE * yylval, mkc_session * session)
{
	mkc_scanner * scanner;
	mkc_tree * tree;
	mkc_data * data;
	char * name;
	cc * str;
	int len;
	char c;
	int i;

	scanner = &(session->scanner);
	len = scanner->len - scanner->pos;
	str = scanner->str + scanner->pos;

	for (i = 1; i < len; i ++)	{
		c = str[i];

		if (' ' == c || '\t' == c)	{
			goto __token_finish;
		}

		if ('\r' == c || '\n' == c)	{
			LINENO ++;
			goto __token_finish;
		}

		if (IS_alphabet(c))	{
			continue;
		}

		if (IS_ALPHABET(c))	{
			continue;
		}

		if (IS_NUMBER(c))	{
			continue;
		}

		if ('_' == c)	{
			continue;
		}

		goto __token_finish;
	}

__token_finish:

	scanner->pos += i;
	name = mkc_session_malloc(session, i + 1);
	if (!name)	{
		return MEMORY;
	}

	data = mkc_data_new(session, MKC_D_ID, __FILE__, __LINE__);
	if (!data)	{
		return MEMORY;
	}

	tree = mkc_tree_new(session, MKC_T_ID, __FILE__, __LINE__);
	if (!tree)	{
		mkc_session_free(session, name);
		return MEMORY;
	}

	memcpy(name, str, i);
	name[i] = '\0';
	data->id = name;
	data->length = i;
	tree->data = data;
	SET_TREE_LINENO;
	yylval->tree = tree;

	return ID;
}

STATIC int
mkc_match_hex_number(YYSTYPE * yylval, mkc_session * session)	{
	mkc_scanner * scanner;
	mkc_tree * tree;
	mkc_data * data;
	cc * str;
	int len;
	char c;
	i64 s;
	int i;

	scanner = &(session->scanner);
	len = scanner->len - scanner->pos - 2;
	str = scanner->str + scanner->pos + 2;

	s = 0;

	FOR (i, len)	{
		c = str[i];
		if (c >= '0' && c <= '9')	{
			s <<= 4;
			s += (c - '0');
		}
		else if (c >= 'a' && c <= 'f')	{
			s <<= 4;
			s += (c - 'a' + 0xa);
		}
		else if (c >= 'A' && c <= 'F')	{
			s <<= 4;
			s += (c - 'A' + 0xa);
		}
		else	{
			if (i)	{
				break;
			}
			else	{
				return ERROR;
			}
		}
	}

	scanner->pos += (i + 2);

	data = mkc_data_new(session, MKC_D_INT, __FILE__, __LINE__);
	if (!data)	{
		return MEMORY;
	}

	tree = mkc_tree_new(session, MKC_T_IMMEDIATE_DATA, __FILE__, __LINE__);
	if (!tree)	{
		return MEMORY;
	}

	tree->data = data;
	data->integer = s;
	SET_TREE_LINENO;
	yylval->tree = tree;

	return DATA;
}

STATIC int
mkc_match_oct_number(YYSTYPE * yylval, mkc_session * session)	{
	mkc_scanner * scanner;
	mkc_tree * tree;
	mkc_data * data;
	cc * str;
	int len;
	char c;
	i64 s;
	int i;

	scanner = &(session->scanner);
	len = scanner->len - scanner->pos;
	str = scanner->str + scanner->pos;

	s = 0;

	FOR (i, len)	{
		c = str[i];
		if (c >= '0' && c <= '7')	{
			s <<= 3;
			s += (c - '0');
		}
		else	{
			if (i)	{
				break;
			}
			else	{
				return ERROR;
			}
		}
	}

	scanner->pos += (i);

	data = mkc_data_new(session, MKC_D_INT, __FILE__, __LINE__);
	if (!data)	{
		return MEMORY;
	}

	tree = mkc_tree_new(session, MKC_T_IMMEDIATE_DATA, __FILE__, __LINE__);
	if (!tree)	{
		return MEMORY;
	}

	tree->data = data;
	data->integer = s;
	SET_TREE_LINENO;
	yylval->tree = tree;

	return DATA;
}

STATIC int
mkc_match_dec_number(YYSTYPE * yylval, mkc_session * session)	{
	mkc_scanner * scanner;
	mkc_tree * tree;
	mkc_data * data;
	cc * str;
	int len;
	char c;
	i64 s;
	int i;

	scanner = &(session->scanner);
	len = scanner->len - scanner->pos;
	str = scanner->str + scanner->pos;

	s = 0;

	FOR(i, len)	{
		c = str[i];
		if (c >= '0' && c <= '9')	{
			s *= 10;
			s += (c - '0');
		}
		else	{
			if (i)	{
				break;
			}
			else	{
				return ERROR;
			}
		}
	}

	scanner->pos += i;

	data = mkc_data_new(session, MKC_D_INT, __FILE__, __LINE__);
	if (!data)	{
		return MEMORY;
	}

	tree = mkc_tree_new(session, MKC_T_IMMEDIATE_DATA, __FILE__, __LINE__);
	if (!tree)	{
		return MEMORY;
	}

	tree->data = data;
	data->integer = s;
	SET_TREE_LINENO;
	yylval->tree = tree;

	return DATA;
}

STATIC int
mkc_match_number_0(YYSTYPE * yylval, mkc_session * session)
{
	mkc_scanner * scanner;
	cc * str;
	int len;

	scanner = &(session->scanner);
	len = scanner->len - scanner->pos;
	str = scanner->str + scanner->pos;

	if (len > 2 && ('x' == str[1] || 'X' == str[1]))	{
		return mkc_match_hex_number(yylval, session);
	}
	else	{
		return mkc_match_oct_number(yylval, session);
	}
}

#define S_SLASH		1
#define S_NORMAL	0
/* return length */
/* return -1 for error */
STATIC int
mkc_lex_string(cc * str, int len, char * buf, mkc_tree * tree)
{
	char c;
	int l;
	int i;
	int s;

	s = S_NORMAL;
	l = 0;

	for (i = 1; i < len; i ++)	{
		c = str[i];

		if (S_NORMAL == s)	{
			if ('\"' == c)	{
				goto __token_finish;
			}
			else if ('\\' == c)	{
				s = S_SLASH;
			}
			else	{
				if (buf)	{
					buf[l] = c;
				}
				l ++;
			}
		}
		else if (S_SLASH == s)	{
			if ('\"' == c || '\\' == c)	{
				if (buf)	{
					buf[l] = c;
				}
				l ++;
				s = S_NORMAL;
			}
			else if ('n' == c)	{
				if (buf)	{
					buf[l] = '\n';
				}
				l ++;
				s = S_NORMAL;
			}
			else if ('r' == c)	{
				s = S_NORMAL;
			}
			else if ('t' == c)	{
				if (buf)	{
					buf[l] = '\t';
				}
				l ++;
				s = S_NORMAL;
			}
			else	{
				if (buf)	{
					buf[l] = '\\';
				}
				l ++;
				if (buf)	{
					buf[l] = c;
				}
				l ++;
				s = S_NORMAL;
			}
		}
	}

	return -1;

__token_finish:

	if (tree)	{
		if (buf)	{
			buf[l] = '\0';
		}
		tree->data->length = l;
	}

	return i;
}

/* return 0 or string length for success */
/* return nagetive value for error */
STATIC int
mkc_match_string(YYSTYPE * yylval, mkc_session * session)
{
	mkc_scanner * scanner;
	mkc_tree * tree;
	mkc_data * data;
	char * buf;
	cc * str;
	int len;
	int r;

	scanner = &(session->scanner);
	len = scanner->len - scanner->pos;
	str = scanner->str + scanner->pos;

	data = mkc_data_new(session, MKC_D_BUF, __FILE__, __LINE__);
	if (!data)	{
		return MEMORY;
	}

	tree = mkc_tree_new(session, MKC_T_IMMEDIATE_DATA, __FILE__, __LINE__);
	if (!tree)	{
		return MEMORY;
	}

	tree->data = data;
	SET_TREE_LINENO;
	yylval->tree = tree;

	r = mkc_lex_string(str, len, NULL, NULL);
	if (r <= 0)	{
		return ERROR;
	}

	buf = mkc_session_malloc(session, r + 1);
	if (!buf)	{
		return MEMORY;
	}

	r = mkc_lex_string(str, len, buf, tree);
	if (r < 0)	{
		Panic();
		return ERROR;
	}

	scanner->pos += (r + 1);
	data->buffer = buf;

	return DATA;
}

#define LC_MATCH_OPERATOR_1_2(__s2,__t1,__t2)	do	{	\
	if (str[i + 1] == __s2[1])	{						\
		scanner->pos += 2;								\
		return __t2;									\
	}													\
	else	{											\
		scanner->pos ++;								\
		return __t1;									\
	}													\
}while(0);												\

#define LC_MATCH_OPERATOR_1_2_2(__s2,__s3,__t1,__t2,__t3)	\
do	{														\
	char c2;												\
	c2 = str[i + 1];										\
	if (c2 == __s3[1])	{									\
		scanner->pos += 2;									\
		return __t3;										\
	}														\
	else if (c2 == __s2[1])	{								\
		scanner->pos += 2;									\
		return __t2;										\
	}														\
	else	{												\
		scanner->pos ++;									\
		return __t1;										\
	}														\
}while(0);													\

#define LC_MATCH_OPERATOR_1_2_2_3(__s2,__s3,__s4,__t1,__t2,__t3,__t4)	\
do	{																	\
	char c2;															\
	char c3;															\
	int _left = len - i;												\
	if (_left > 2)	{													\
		c2 = str[i + 1];												\
		c3 = str[i + 2];												\
		if (c2 == __s4[1] && c3 == __s4[2])	{							\
			scanner->pos += 3;											\
			return __t4;												\
		}																\
		else if (c2 == __s3[1])	{										\
			scanner->pos += 2;											\
			return __t3;												\
		}																\
		else if (c2 == __s2[1])	{										\
			scanner->pos += 2;											\
			return __t2;												\
		}																\
		else	{														\
			scanner->pos ++;											\
			return __t1;												\
		}																\
	}																	\
	else if (_left > 1)	{												\
		c2 = str[i + 1];												\
		if (c2 == __s3[1])	{											\
			scanner->pos += 2;											\
			return __t3;												\
		}																\
		else if (c2 == __s2[1])	{										\
			scanner->pos += 2;											\
			return __t2;												\
		}																\
		else	{														\
			scanner->pos ++;											\
			return __t1;												\
		}																\
	}																	\
	else if (_left > 0)	{												\
		scanner->pos ++;												\
		return __t1;													\
	}																	\
	else	{															\
		return ERROR;													\
	}																	\
}while(0);

#define CONSUME_COMMENT_LINE	do	{				\
	for (; i < len; i ++)	{						\
		if ('\n' == str[i] || '\r' == str[i])	{	\
			i ++;									\
			goto __lex;								\
		}											\
	}												\
	return EOF;										\
}while(0);

int
mkc_lex(void * yylval_ptr, mkc_session * session)	{
	YYSTYPE * yylval = yylval_ptr;
	mkc_scanner * scanner;
	cc * str;
	int len;
	char c;
	int i;

	scanner = &(session->scanner);
	str = scanner->str;
	len = scanner->len;
	i = scanner->pos;

__lex:

	/* eat the leading ' ', '\t', '\n', '\r' */
	for (; i < len; i ++)	{
		c = str[i];
		if (' ' == c || '\t' == c)	{
			continue;
		}
		if ('\n' == c || '\r' == c)	{
			LINENO ++;
			continue;
		}
		break;
	}

	if (i >= len)	{
		scanner->pos = i;
		return EOF;
	}

	scanner->pos = i;

	if (IS_ALPHABET(c) || '_' == c)	{
		return mkc_match_id(yylval, session);
	}

	switch(c)	{
	case 'a':
		return mkc_match_id(yylval, session);
	case 'b':
		return mkc_match_keyword_or_id(yylval, session, s_breakBREAK, 1);
	case 'c':
		return mkc_match_keyword_or_id(yylval, session, s_caseCASE, 4);
	case 'd':
		return mkc_match_keyword_or_id(yylval, session, s_defaultDEFAULT, 2);
	case 'e':
		return mkc_match_keyword_or_id(yylval, session, s_elseELSE, 2);
	case 'f':
		return mkc_match_keyword_or_id(yylval, session, s_forFOR, 2);
	case 'g':
		return mkc_match_keyword_or_id(yylval, session, s_gotoGOTO, 1);
	case 'h':
		return mkc_match_id(yylval, session);
	case 'i':
		return mkc_match_keyword_or_id(yylval, session, s_ifIF, 1);
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
		return mkc_match_id(yylval, session);
	case 'p':
	case 'q':
		return mkc_match_id(yylval, session);
	case 'r':
		return mkc_match_keyword_or_id(yylval, session, s_returnRETURN, 1);
	case 's':
		return mkc_match_keyword_or_id(yylval, session, s_switchSWITCH, 1);
	case 't':
	case 'u':
		return mkc_match_id(yylval, session);
	case 'v':
		return mkc_match_keyword_or_id(yylval, session, s_varVAR, 2);
	case 'w':
		return mkc_match_keyword_or_id(yylval, session, s_whileWHILE, 1);
	case 'x':
	case 'y':
	case 'z':
		return mkc_match_id(yylval, session);
	case '0':
		return mkc_match_number_0(yylval, session);
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return mkc_match_dec_number(yylval, session);
	case '\"':
		return mkc_match_string(yylval, session);
	case '{':
	case '}':
	case '(':
	case ')':
	case ';':
	case '~':
	case ':':
	case ',':
		scanner->pos ++;
		return c;
	case '=':
		LC_MATCH_OPERATOR_1_2("==", '=', EQUAL);
	case '+':
		LC_MATCH_OPERATOR_1_2_2("+=", "++", '+', ADD_ASSIGN, INC);
	case '-':
		LC_MATCH_OPERATOR_1_2_2("-=", "--", '-', SUB_ASSIGN, DEC);
	case '*':
		LC_MATCH_OPERATOR_1_2("*=", '*', MUL_ASSIGN);
	case '/':
		LC_MATCH_OPERATOR_1_2("/=", '/', DIV_ASSIGN);
	case '%':
		LC_MATCH_OPERATOR_1_2("%=", '%', MOD_ASSIGN);
	case '!':
		LC_MATCH_OPERATOR_1_2("!=", '!', DIFF);
	case '^':
		LC_MATCH_OPERATOR_1_2("^=", '^', XOR_ASSIGN);
	case '&':
		LC_MATCH_OPERATOR_1_2_2("&&", "&=", '&', LOGIC_AND, AND_ASSIGN);
	case '|':
		LC_MATCH_OPERATOR_1_2_2("||", "|=", '|', LOGIC_OR, OR_ASSIGN);
	case '>':
		LC_MATCH_OPERATOR_1_2_2_3(
				">=", ">>", ">>="
				, '>', GREATER_EQUAL, RIGHT_SHIFT, RIGHT_SHIFT_ASSIGN
				);
	case '<':
		LC_MATCH_OPERATOR_1_2_2_3(
				"<=", "<<", "<<="
				, '<', LESS_EQUAL, LEFT_SHIFT, LEFT_SHIFT_ASSIGN
				);
	case '#':
		CONSUME_COMMENT_LINE
	default:
		return ERROR;
	}
}

/* eof */
