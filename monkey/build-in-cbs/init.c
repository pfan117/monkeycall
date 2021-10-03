/* vi: set sw=4 ts=4: */

#include "monkeycall.h"
#include "include/debug.h"
#include "build-in-cbs/cbs.h"

int
mkc_buildin_cbs_init(void * instance)	{
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Print", mkc_cb_print));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Snprint", mkc_cb_snprint));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Scan", mkc_cb_scan));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Recycle", mkc_cb_recycle));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Malloc", mkc_cb_malloc));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Free", mkc_cb_free));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "MallocBalance", mkc_cb_malloc_balance));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Base64En", mkc_cb_base64_encode));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Base64De", mkc_cb_base64_decode));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Strcmp", mkc_cb_strcmp));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "ToUpper", mkc_cb_toupper));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "ToLower", mkc_cb_tolower));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "RandStr", mkc_cb_randstr));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Rand", mkc_cb_rand));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "GetSecond", mkc_cb_get_second));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Cut", mkc_cb_cut));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Dup", mkc_cb_dup));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Stick", mkc_cb_stick));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "UnEsc", mkc_cb_unesc));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Diff", mkc_cb_diff));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "SHA512", mkc_cb_sha512));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "SHA512HMAC", mkc_cb_sha512_hmac));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "AES256", mkc_cb_aes256_encrypt));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "AES256De", mkc_cb_aes256_decrypt));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "Backtrace", mkc_cb_backtrace));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "IsInteger", mkc_cb_is_integer));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "IsBuffer", mkc_cb_is_buffer));
	IF_ERROR_RETURN(mkc_cbtree_add(instance, "IsVoid", mkc_cb_is_void));

	return 0;
}

/* eof */
