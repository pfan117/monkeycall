/* vi: set sw=4 ts=4: */

#ifndef __MKC_TRANSLATE_HEADER_INCLUDED__
#define __MKC_TRANSLATE_HEADER_INCLUDED__

extern int mkc_translate_expr(mkc_session *, mkc_tree * tree, mkc_tree ** rp);
extern int mkc_translate_stmts(mkc_session *, mkc_tree * tree, mkc_tree ** rp);
extern int mkc_translate_function(mkc_session *, mkc_symbol_function *);
extern int __mkc_translate_function(mkc_session *, mkc_symbol_function *);

#endif
