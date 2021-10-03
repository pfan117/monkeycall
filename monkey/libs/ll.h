/* vi: set sw=4 ts=4 : */

#ifndef __LL_HEADER_INCLUDED__
#define __LL_HEADER_INCLUDED__

/* double direction link list, no circle */
#define DOUBLE_LL_AWAY(__P,__NEXT,__PREV,__HEAD)	do	{\
	if ((__P) -> __PREV)	{\
		(__P) -> __PREV -> __NEXT = (__P) -> __NEXT;\
	}\
	else	{\
		(__HEAD) = (__P) -> __NEXT;\
	}\
	if ((__P) -> __NEXT)	{\
		(__P) -> __NEXT -> __PREV = (__P) -> __PREV;\
	}\
}while(0);

#define DOUBLE_LL_JOIN(__P,__NEXT,__PREV,__HEAD)	do	{\
	(__P) -> __PREV = NULL;\
	(__P) -> __NEXT = (__HEAD);\
	if ((__HEAD))	{\
		(__HEAD) -> __PREV = (__P);\
	}\
	(__HEAD) = (__P);\
}while(0);

/* single link list, no circle */
#define SINGLE_LL_FREE(__P,__N,__NEXT,__HEAD)	do	{\
	for ((__P) = __HEAD; (__P); (__P) = (__N))	{\
		(__N) = __P -> __NEXT;\
		mkc_user_provide_free((__P));\
	}\
	(__HEAD) = NULL;\
}while(0);

#define SINGLE_LL_JOIN(__P,__NEXT,__HEAD)	do	{\
	(__P) -> __NEXT = (__HEAD);\
	(__HEAD) = (__P);\
}while(0);

#define SINGLE_LL_SORT_JOIN(__P,__PP,__PPP,__NEXT,__HEAD,__CMP)	do	{\
	(__PPP) = NULL;\
	for((__PP) = (__HEAD); (__PP); (__PPP) = (__PP), (__PP) = (__PP) -> __NEXT)	{\
		if (__CMP((__PP), (__P)) >= 0)	{\
			break;\
		}\
	}\
	if (__PPP)	{\
		(__P) -> __NEXT = (__PPP) -> __NEXT;\
		(__PPP) -> __NEXT = (__P);\
	}\
	else	{\
		(__P) -> __NEXT = (__HEAD);\
		(__HEAD) = (__P);\
	}\
}while(0);

/* __P: the node need to be set away */
/* __T, __PT: two temp pointers */
/* __NEXT: next node pointer name */
/* __HEAD: root of the link list */
#define SINGLE_LL_AWAY(__P,__T,__PT,__NEXT,__HEAD)	do	{\
	__PT = NULL;\
	for(__T = __HEAD; __T; __T = __T -> __NEXT)	{\
		if (__T == __P)	{\
			if (__PT)	{\
				__PT -> __NEXT = __T -> __NEXT;\
			}\
			else	{\
				__HEAD = __T -> __NEXT;\
			}\
			break;\
		}\
		__PT = __T;\
	}\
}while(0);

#define SINGLE_LL_HEAD_AWAY(__P,__NEXT,__HEAD)	do	{\
	__P = __HEAD;\
	if (__P)	{\
		__HEAD = __P -> __NEXT;\
	}\
}while(0);

#define SINGLE_LL_FOREACH(__P,__NEXT,__ROOT)	for((__P) = (__ROOT);\
		(__P); (__P) = __P -> __NEXT)

#define SINGLE_LL_NEXT(__P,__NEXT,_HEAD)	(((__P) && __P->__NEXT)\
		? (__P->__NEXT) : (_HEAD))

#define SINGLE_LL2_JOIN_HEAD(__P,__NEXT,__HEAD,__TAIL)	do	{\
	if (__HEAD)	{\
		__P -> __NEXT = __HEAD;\
		__HEAD = __P;\
	}\
	else	{\
		__TAIL = __HEAD = __P;\
		__P -> __NEXT = NULL;\
	}\
}while(0);

#define SINGLE_LL2_JOIN_TAIL(__P,__NEXT,__HEAD,__TAIL)	do	{\
	if (__TAIL)	{\
		__TAIL -> __NEXT = __P;\
		__P -> __NEXT = NULL;\
		__TAIL = __P;\
	}\
	else	{\
		__TAIL = __HEAD = __P;\
		__P -> __NEXT = NULL;\
	}\
}while(0);

#define SINGLE_LL2_HEAD_AWAY(__P,__NEXT,__HEAD,__TAIL)	do	{\
	__P = __HEAD;\
	if (__HEAD == __TAIL)	{\
		__HEAD = __TAIL = NULL;\
	}\
	else	{\
		__HEAD = __HEAD -> __NEXT;\
		if (!__HEAD)	{\
			__TAIL = NULL;\
		}\
	}\
}while(0);

/*
 * __P: new node pointer (need to be initialized)
 * __NEXT, __PREV: field names
 * __ROOT: link list root pointer variable (need to be initialized)
 * __HEAD: node head pointer variable
 * __TAIL: node tail pointer variable
 */
#define DOUBLE_LL_ADD_2_TAIL(__P,__NEXT,__PREV,__ROOT,__HEAD,__TAIL)	\
do {\
	__HEAD = __ROOT;\
	if (__HEAD)	{\
		__TAIL = __HEAD->__PREV;\
		__TAIL->__NEXT = __P;\
		__HEAD->__PREV = __P;\
		__P->__NEXT = __HEAD;\
		__P->__PREV = __TAIL;\
	}\
	else	{\
		__P->__NEXT = __P;\
		__P->__PREV = __P;\
		__ROOT = __P;\
	}\
}while(0);

#endif
