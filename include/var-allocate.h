/* vi: set sw=4 ts=4: */

#ifndef __MKC_VAR_ALLOCATE_INCLUDED__
#define __MKC_VAR_ALLOCATE_INCLUDED__

#include "libs/tree.h"

typedef struct _mkc_va	{
	void	* free;
	void	* busy;
} mkc_va;

extern void mkc_va_init(mkc_va * va);
extern int mkc_va_alloc(mkc_session * session);
extern void mkc_va_free(mkc_session * session, int idx);
extern void mkc_va_detach(mkc_session * session, mkc_va * va);
extern int mkc_va_depth(mkc_va * va);

#define FREE_TEMP_VAR(__tree)	\
if (__tree && (MKC_STORAGE_TEMP == __tree->result_location.storage))	{	\
	mkc_va_free(session, __tree->result_location.index);	\
}

#endif
