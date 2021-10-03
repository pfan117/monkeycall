/* vi: set sw=4 ts=4: */

#ifndef __MKC_PLUGINS_INCLUDED__
#define __MKC_PLUGINS_INCLUDED__

extern int mkc_cb_print(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_snprint(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_scan(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_recycle(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_malloc(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_free(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_malloc_balance(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_base64_encode(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_base64_decode(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_strcmp(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_dup(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_stick(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_unesc(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_diff(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_sha512(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_sha512_hmac(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_aes256_encrypt(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_aes256_decrypt(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_backtrace(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_is_integer(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_is_buffer(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_is_void(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_cut(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_randstr(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_toupper(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_tolower(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_rand(mkc_session *, mkc_cb_stack_frame *);
extern int mkc_cb_get_second(mkc_session *, mkc_cb_stack_frame *);

#endif

/* eof */
