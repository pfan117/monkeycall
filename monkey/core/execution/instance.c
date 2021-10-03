/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <strings.h>
#include <pthread.h>
#include "monkeycall.h"
#include "include/internal.h"

typedef struct _mkc_instance	{
	pthread_rwlock_t	lock;
	void				* cbtree;
} mkc_instance;

void *
mkc_instance_get_cbtree(void * instptr)	{
	mkc_instance * instance = instptr;
	return &(instance->cbtree);
}

int
mkc_instance_rlock(void * instance_p)	{
	mkc_instance * instance = instance_p;
	int r;

	r = pthread_rwlock_rdlock(&(instance->lock));
	if (r)	{
		fprintf(stderr, "internal error: %s\n", "failed to rlock instance");
		return -1;
	}

	return 0;
}

int
mkc_instance_wlock(void * instance_p)	{
	mkc_instance * instance = instance_p;
	int r;

	r = pthread_rwlock_wrlock(&(instance->lock));
	if (r)	{
		fprintf(stderr, "internal error: %s\n", "failed to wlock instance");
		return -1;
	}

	return 0;
}

int
mkc_instance_unlock(void * instance_p)	{
	mkc_instance * instance = instance_p;
	int r;

	r = pthread_rwlock_unlock(&(instance->lock));
	if (r)	{
		fprintf(stderr, "internal error: %s\n", "failed to unlock instance");
		return -1;
	}

	return 0;
}

void *
mkc_new_instance(void)	{
	int r;

	mkc_instance * instance;
	instance = (mkc_instance *)MALLOC(sizeof(mkc_instance));
	if (!instance)	{
		fprintf(stderr, "execution error: %s\n", "failed to malloc memory for new instance");
		return NULL;
	}
	bzero(instance, sizeof(mkc_instance));
	r = pthread_rwlock_init(&(instance->lock), NULL);
	if (r)	{
		fprintf(stderr, "execution error: %s\n", "failed to initialize rwlock for new instance");
		FREE(instance);
		return NULL;
	}

	mkc_cbtree_init(instance);

	return instance;
}

void
mkc_free_instance(void * ptr)	{
	mkc_instance * instance = ptr;
	mkc_instance_wlock(instance);
	mkc_cbtree_free(instance);
	mkc_instance_unlock(instance);
	pthread_rwlock_destroy(&(instance->lock));
	FREE(instance);
	return;
}

/* eof */
