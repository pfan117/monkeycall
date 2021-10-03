/* vi: set sw=4 ts=4: */

#ifndef __MKC_STACK_INCLUDED__
#define __MKC_STACK_INCLUDED__

extern mkc_stack_frame * mkc_new_stack(
		mkc_session * session, unsigned long argc, unsigned long depth);
extern mkc_cb_stack_frame * mkc_new_cb_stack(
		mkc_session * session, unsigned long argc);
extern void mkc_stack_push(mkc_session * session, mkc_stack_frame * stack);
extern int mkc_stack_pop(mkc_session * session);

extern mkc_data * mkc_get_data_by_location(
		mkc_session * session, mkc_location * location);
extern mkc_data *
mkc_find_parameter_from_stack(mkc_stack_frame *, int);

#endif

/* eof */
