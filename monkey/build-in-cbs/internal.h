/* vi: set sw=4 ts=4: */

#ifndef __MKC_INTERNAL_PLUGINS_INCLUDED__
#define __MKC_INTERNAL_PLUGINS_INCLUDED__

extern int encode_base64_core(const char * str, char * dst, int size);
extern int decode_base64_len(char * out, const char * src, int l);

#endif

/* eof */
