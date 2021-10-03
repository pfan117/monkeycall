/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <string.h>
#include "monkeycall.h"
#include "include/types.h"
#include "libs/tree.h"
#include "libs/misc.h"
#include "include/internal.h"

#define GET_ROOT root = mkc_instance_get_cbtree(instance)

typedef struct
_mkc_cbtree_node	{
	u64							checksum0;
	u64							checksum1;
	const char *				id;
	mkc_cb_t					cb;
	RB_ENTRY(_mkc_cbtree_node)	rb;
	int							len;
} mkc_cbtree_node;

STATIC int
__mkc_cbtree_cmp(mkc_cbtree_node * an, mkc_cbtree_node * bn)	{
	u64 a;
	u64 b;

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

	if (an->len != bn->len)	{
		if (an->len > bn->len)	{
			return 1;
		}
		else	{
			return -1;
		}
	}

	return memcmp(an->id, bn->id, an->len);
}

RB_HEAD(MKC_CBTREE, _mkc_cbtree_node);
RB_PROTOTYPE_STATIC(MKC_CBTREE, _mkc_cbtree_node, rb, __mkc_cbtree_cmp);
RB_GENERATE_STATIC(MKC_CBTREE, _mkc_cbtree_node, rb, __mkc_cbtree_cmp);

void
mkc_cbtree_init(void * instance)	{
	struct MKC_CBTREE * root;

	GET_ROOT;
	root->rbh_root = NULL;
	return;
}

STATIC void
mkc_cbtree_checksum(mkc_cbtree_node * p, cc * id, int len)	{
	u64 checksum0;
	u64 checksum1;
	u8 * uc;
	int i;
	u8 u;

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

STATIC mkc_cbtree_node *
__mkc_cbtree_find(struct MKC_CBTREE * root, cc * id, int len)
{
	mkc_cbtree_node fnode;
	mkc_cbtree_node * p;

	fnode.id = id;
	fnode.len = len;
	mkc_cbtree_checksum(&fnode, id, len);
	p = MKC_CBTREE_RB_FIND(root, &fnode);

	return p;
}

/* on success, return cb */
/* on failed, return NULL */
/* len is the strlen() result of the id string */
mkc_cb_t
mkc_cbtree_lookup2(void * instance, cc * id, int len)	{
	struct MKC_CBTREE * root;
	mkc_cbtree_node * p;

	GET_ROOT;
	p = __mkc_cbtree_find(root, id, len);	
	if (p)	{
		return p->cb;
	}
	else	{
		return NULL;
	}
}

/* on success, return cb */
/* on failed, return NULL */
mkc_cb_t
mkc_cbtree_lookup(void * instance, cc * id)	{
	void * r;

	r = mkc_cbtree_lookup2(instance, id, strlen(id));

	return r;
}

/* return 0 for success */
/* return -1 for already exist */
/* return -2 for no memory */
int
mkc_cbtree_add2(void * instance, cc * id, int len, mkc_cb_t cb)	{
	struct MKC_CBTREE * root;
	mkc_cbtree_node * p;
	char * s;
	int r;

	r = mkc_instance_wlock(instance);
	if (r)	{
		fprintf(stderr, "execution error: %s\n", "failed to write lock the instance for cb adding");
		return -3;
	}

	GET_ROOT;
	p = __mkc_cbtree_find(root, id, len);
	if (p)	{
		mkc_instance_unlock(instance);
		fprintf(stderr, "execution error: %s\n", "callback already defined");
		return -1;
	}

	p = (mkc_cbtree_node *)MALLOC(sizeof(mkc_cbtree_node) + len + 1);
	if (!p)	{
		mkc_instance_unlock(instance);
		fprintf(stderr, "execution error: %s\n", "failed to define new callback, no memory");
		return -2;
	}

	s = (char *)(p + 1);
	p->id = s;
	p->len = len;
	mkc_cbtree_checksum(p, id, len);
	p->cb = cb;
	memcpy(s, id, len);
	s[len] = '\0';
	MKC_CBTREE_RB_INSERT(root, p);
	mkc_instance_unlock(instance);

	return 0;
}

/* return 0 for success */
/* return -1 for already exist */
/* return -2 for id too long */
/* return -3 for no memory */
/* return -4 for param error */
int
mkc_cbtree_add(void * instance, cc * id, mkc_cb_t cb)	{
	int r;
	u32 len;

	len = strlen(id);
	r = mkc_cbtree_add2(instance, id, len, cb);

	return r;
}

/* return 0 for success */
/* return -1 for don't exist */
int
mkc_cbtree_del2(void * instance, cc * id, int len)	{
	struct MKC_CBTREE * root;
	mkc_cbtree_node * p;
	int r;

	r = mkc_instance_wlock(instance);
	if (r)	{
		fprintf(stderr, "execution error: %s\n", "failed to write lock the instance for cb delecting");
		return -3;
	}

	GET_ROOT;
	p = __mkc_cbtree_find(root, id, len);
	if (!p)	{
		mkc_instance_unlock(instance);
		return -1;
	}
	MKC_CBTREE_RB_REMOVE((void *)root, p);
	mkc_instance_unlock(instance);

	FREE(p);
	return 0;
}

/* return 0 for success */
/* return -1 for don't exist */
int
mkc_cbtree_del(void * instance, cc * id)	{
	int r;
	u32 len;

	len = strlen(id);
	r = mkc_cbtree_del2(instance, id, len);

	return r;
}

void
mkc_cbtree_free(void * instance)	{
	struct MKC_CBTREE * root;
	mkc_cbtree_node * p;

	GET_ROOT;
	while((p = RB_MIN(MKC_CBTREE, root)))	{
		MKC_CBTREE_RB_REMOVE(root, p);
		FREE(p);
	};

	return;
}

cc *
mkc_cbtree_get_first_id(void * instance, char * buf, int len)	{
	struct MKC_CBTREE * root;
	mkc_cbtree_node * p;
	int r;

	r = mkc_instance_rlock(instance);
	if (r)	{
		fprintf(stderr, "execution error: %s\n", "failed to read lock the instance for get_first");
		return NULL;
	}

	GET_ROOT;
	p = RB_MIN(MKC_CBTREE, root);
	if (p && len > p->len)	{
		memcpy(buf, p->id, p->len + 1);
		mkc_instance_unlock(instance);
		return buf;
	}

	mkc_instance_unlock(instance);
	return NULL;
}

cc *
mkc_cbtree_get_next_id(void * instance, cc * id, char * buf, int len)	{
	struct MKC_CBTREE * root;
	mkc_cbtree_node * p;
	int r;

	r = mkc_instance_rlock(instance);
	if (r)	{
		printf("execution error: %s\n", "failed to read lock the instance for get_next");
		return NULL;
	}

	GET_ROOT;
	p = __mkc_cbtree_find(root, id, strlen(id));
	if (!p)	{
		mkc_instance_unlock(instance);
		return NULL;
	}

	p = MKC_CBTREE_RB_NEXT(p);
	if (p && len > p->len)	{
		memcpy(buf, p->id, p->len + 1);
		mkc_instance_unlock(instance);
		return buf;
	}

	mkc_instance_unlock(instance);
	return NULL;
}

/* eof */
