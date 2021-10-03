/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/types.h"
#include "libs/tree.h"
#include "libs/misc.h"
#include "include/debug.h"
#include "include/internal.h"

#define GET_ROOT root = (struct MKC_LABEL *)&(session->label_table)

typedef struct
_mkc_label_node	{
	u64							checksum0;
	u64							checksum1;
	mkc_data					* id;
	mkc_tree					* label;
	RB_ENTRY(_mkc_label_node)	rb;
} mkc_label_node;

STATIC int
__mkc_label_cmp(mkc_label_node * an, mkc_label_node * bn)	{
	u64 a;
	u64 b;
	mkc_data * lid;
	mkc_data * rid;

	a = an->checksum0;
	b = bn->checksum0;
	if (a != b)	{
		if (a > b)	{
			return 1;
		}
		else	{
			return -1;
		}
	}

	a = an->checksum1;
	b = bn->checksum1;
	if (a != b)	{
		if (a > b)	{
			return 1;
		}
		else	{
			return -1;
		}
	}

	lid = an->id;
	rid = bn->id;

	if (lid->length != rid->length)	{
		if (lid->length > rid->length)	{
			return 1;
		}
		else	{
			return -1;
		}
	}

	return memcmp(lid->id, rid->id, lid->length);
}

RB_HEAD(MKC_LABEL, _mkc_label_node);
RB_PROTOTYPE_STATIC(MKC_LABEL, _mkc_label_node, rb, __mkc_label_cmp);
RB_GENERATE_STATIC(MKC_LABEL, _mkc_label_node, rb, __mkc_label_cmp);

void
mkc_label_init(mkc_session * session)	{
	struct MKC_LABEL * root;

	GET_ROOT;
	root->rbh_root = NULL;
	return;
}

STATIC void
__mkc_label_checksum(mkc_label_node * p)	{
	u64 checksum0;
	u64 checksum1;
	cc * id;
	int len;
	u8 * uc;
	int i;
	u8 u;

	id = p->id->id;
	len = p->id->length;
	uc = (u8 *)id;
	checksum0 = 0U;
	checksum1 = 0U;

	FOR(i, len)	{
		u = uc[i];
		checksum0 = checksum0 + u;
		checksum1 = (checksum1 << 1) + u;
	}

	p->checksum0 = checksum0;
	p->checksum1 = checksum1;

	return;
}

mkc_tree *
mkc_label_find(mkc_session * session, mkc_data * id)
{
	struct MKC_LABEL * root;
	mkc_label_node fnode;
	mkc_label_node * p;

	GET_ROOT;
	fnode.id = id;
	__mkc_label_checksum(&fnode);
	p = MKC_LABEL_RB_FIND(root, &fnode);
	if (p)	{
		return p->label;
	}
	else	{
		return NULL;
	}
}

int
mkc_label_add(mkc_session * session, mkc_tree * label)	{
	struct MKC_LABEL * root;
	mkc_label_node * p;
	mkc_label_node * e;

	GET_ROOT;
	p = SMALLOC(mkc_label_node);
	if (!p)	{
		MKC_TREE_ERROR(label->left, "out of memory");
		return -1;
	}

	p->label = label;
	p->id = label->left->data;
	__mkc_label_checksum(p);

	e = MKC_LABEL_RB_FIND(root, p);
	if (e)	{
		SFREE(p);
		MKC_TREE_ERROR(label->left, "label '%s' redefined", e->id->id);
		return -1;
	}

	MKC_LABEL_RB_INSERT(root, p);

	return 0;
}

void
mkc_label_free(mkc_session * session)	{
	struct MKC_LABEL * root;
	mkc_label_node * p;

	GET_ROOT;
	while((p = RB_MIN(MKC_LABEL, root)))	{
		MKC_LABEL_RB_REMOVE(root, p);
		SFREE(p);
	};

	return;
}

/* eof */
