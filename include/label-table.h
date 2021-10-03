/* vi: set sw=4 ts=4: */

#ifndef __MKC_LABEL_TABLE_INCLUDED__
#define __MKC_LABEL_TABLE_INCLUDED__

extern void mkc_label_init(mkc_session * session);
extern mkc_tree * mkc_label_find(mkc_session * session, mkc_data * id);
extern int mkc_label_add(mkc_session * mkc_session, mkc_tree * label);
extern void mkc_label_free(mkc_session * session);

#endif

/* eof */
