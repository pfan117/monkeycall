/* vi: set sw=4 ts=4: */

#ifndef __MKC_INTERNAL_INCLUDED__
#define __MKC_INTERNAL_INCLUDED__

extern const char * mkc_tree_type_name(mkc_tree_type type);
extern const char * mkc_data_type_name(mkc_data_type type);
extern mkc_tree * mkc_tree_new(mkc_session *, mkc_tree_type, const char *, int);
extern mkc_data * mkc_data_new(mkc_session *, mkc_data_type, const char *, int);
extern mkc_tree * mkc_tree_join(mkc_tree * a, mkc_tree * b);
extern void mkc_tree_loop_break(mkc_tree * root);
extern void mkc_tree_self_loop(mkc_tree * root);
extern void mkc_dump_tree_list(mkc_tree * sequence);

extern void mkc_session_free_all_mm(mkc_session *);
extern void * mkc_instance_get_cbtree(void * instance);
extern int mkc_instance_rlock(void * instance);
extern int mkc_instance_wlock(void * instance);
extern int mkc_instance_unlock(void * instance);
extern void mkc_cbtree_init(void * instance);
extern void mkc_cbtree_free(void * instance);

#endif

/* eof */
