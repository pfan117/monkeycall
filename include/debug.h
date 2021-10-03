/* vi: set sw=4 ts=4: */

#ifndef __MKC_DEBUG_HEADER_INDLUDED__
#define __MKC_DEBUG_HEADER_INDLUDED__

#define IF_ERROR_RETURN(__op)	do	{	\
	if (__op < 0)	{					\
		return -1;						\
	}									\
}while(0);

#define IF_NULL_RETURN(__op)	do	{	\
	if (!__op)	{						\
		return -1;						\
	}									\
}while(0);

#define Trace()	do	{\
	printf("%s (%s): %d\n", __FUNCTION__, __FILE__, __LINE__);\
}while(0);

extern void mkc_panic(const char * file, int line);
#define Panic()	do	{ \
	mkc_panic(__FILE__, __LINE__); \
}while(0);

#define MKC_ERROR(__fmt__, ...)	do	{ \
	int __r; \
	if (session->err_buffer && session->err_buffer_length)	{ \
		__r = snprintf(session->err_buffer, session->err_buffer_length, \
				"line %d: ", session->lineno); \
		if (__r > 0 && __r < session->err_buffer_length)	{ \
			snprintf(session->err_buffer + __r \
					, session->err_buffer_length - __r \
					, __fmt__, ##__VA_ARGS__); \
		} \
	} \
}while(0)

#define MKC_TREE_ERROR(__tree__,__fmt__, ...)	do	{ \
	int __r; \
	if (session->err_buffer && session->err_buffer_length)	{ \
		__r = snprintf(session->err_buffer, session->err_buffer_length, \
				"line %d: ", __tree__->lineno); \
		if (__r > 0 && __r < session->err_buffer_length)	{ \
			snprintf(session->err_buffer + __r \
					, session->err_buffer_length - __r \
					, __fmt__, ##__VA_ARGS__); \
		} \
	} \
}while(0)

#define MKC_INTERNAL_ERROR	do	{	\
	if (session->err_buffer && session->err_buffer_length)	{ \
		snprintf(session->err_buffer, session->err_buffer_length, \
				"c-- design error at '%s' line %d: ", __FILE__, __LINE__); \
	} \
}while(0)

#endif
