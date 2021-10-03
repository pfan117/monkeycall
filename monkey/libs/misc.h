/* vi: set sw=4 ts=4: */

#ifndef __MKC_MISC_HEADER_INCLUDED__
#define __MKC_MISC_HEADER_INCLUDED__

#ifdef FOR
#error FOR been defined before
#else
#define FOR(__i,__n)	for((__i) = 0; (__i) < (__n); (__i) ++)
#endif

extern int __mkc_get_usec(void);

#endif /* __mkc_MISC_HEADER_INCLUDED__ */
