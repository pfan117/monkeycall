/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <strings.h>
#include "monkeycall.h"
#include "include/types.h"
#include "libs/ll.h"
#include "libs/misc.h"
#include "include/debug.h"
#include "include/internal.h"
#include "include/var-allocate.h"

typedef struct _mkc_vai	{
	RB_ENTRY(_mkc_vai)	rb;
	int					idx;
} mkc_vai;

STATIC int
__mkc_vai_cmp(mkc_vai * a, mkc_vai * b)	{
	return a->idx - b->idx;
}

RB_HEAD(MKC_VAI, _mkc_vai);
RB_PROTOTYPE_STATIC(MKC_VAI, _mkc_vai, rb, __mkc_vai_cmp);
RB_GENERATE_STATIC(MKC_VAI, _mkc_vai, rb, __mkc_vai_cmp);

void
mkc_va_init(mkc_va * va)	{
	va->free = NULL;
	va->busy = NULL;
	return;
}

int
mkc_va_alloc(mkc_session * session)	{
	mkc_va * va;
	mkc_vai * p;
	int idx;

	va = session->va;

	if (va->free)	{
		p = RB_MIN(MKC_VAI, (struct MKC_VAI *)&(va->free));
		MKC_VAI_RB_REMOVE((struct MKC_VAI *)&(va->free), p);
		MKC_VAI_RB_INSERT((struct MKC_VAI *)&(va->busy), p);
		return p->idx;
	}

	if (va->busy)	{
		p = RB_MAX(MKC_VAI, (struct MKC_VAI *)&(va->busy));
		idx = p->idx + 1;
	}
	else	{
		idx = 0;
	}

	p = SMALLOC(mkc_vai);
	if (!p)	{
		MKC_ERROR("out of memory");
		return -1;
	}
	bzero(p, sizeof(*p));

	p->idx = idx;
	MKC_VAI_RB_INSERT((struct MKC_VAI *)&(va->busy), p);

	return idx;
}

void
mkc_va_free(mkc_session * session, int idx)	{
	mkc_vai pattern;
	mkc_va * va;
	mkc_vai * p;

	va = session->va;
	pattern.idx = idx;
	p = MKC_VAI_RB_FIND((struct MKC_VAI *)&(va->busy), &pattern);
	if (!p)	{
		return;
	}

	MKC_VAI_RB_REMOVE((struct MKC_VAI *)&(va->busy), p);
	MKC_VAI_RB_INSERT((struct MKC_VAI *)&(va->free), p);

	return;
}

int
mkc_va_depth(mkc_va * va)	{
	mkc_vai * a;
	mkc_vai * b;
	int max;

	a = RB_MAX(MKC_VAI, (struct MKC_VAI *)&(va->free));
	b = RB_MAX(MKC_VAI, (struct MKC_VAI *)&(va->busy));

	if (a)	{
		if (b)	{
			if (a->idx > b->idx)	{
				max = a->idx;
			}
			else	{
				max = b->idx;
			}
		}
		else	{
			max = a->idx;
		}
	}
	else	{
		if (b)	{
			max = b->idx;
		}
		else	{
			max = -1;
		}
	}

	return max + 1;
}

STATIC void __mkc_va_rb_free(mkc_session * session, struct MKC_VAI * root)	{
	mkc_vai * p;

	while((p = RB_MIN(MKC_VAI, root)))	{
		MKC_VAI_RB_REMOVE(root, p);
		SFREE(p);
	};

	return;
}

void
mkc_va_detach(mkc_session * session, mkc_va * va)	{
	__mkc_va_rb_free(session, (struct MKC_VAI *)&(va->free));
	__mkc_va_rb_free(session, (struct MKC_VAI *)&(va->busy));
	return;
}

/* eof */
