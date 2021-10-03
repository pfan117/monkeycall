/* vi: set sw=4 ts=4: */

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"

#define SHA512_LEN	(512 / 8)

#define SHA512
#define CBNAME "SHA512"

/* sha512 */
/* param: buffer, buffer, ... */
int
mkc_cb_sha512(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)	{
	SHA512_CTX sha512ctx;
	mkc_data * result;
	mkc_data * p;
	char * data;
	int argc;
	int len;
	int i;

	result = MKC_CB_RESULT;
	argc = MKC_CB_ARGC;

	if (!argc)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": no input data");
		result->type = MKC_D_VOID;
		return 0;
	}

	len = 0;
	for (i = 0; i < argc; i ++)	{
		p = MKC_CB_ARGV(i);
		if (MKC_D_BUF == p->type)	{
			len += p->length;
		}
		else	{
			MKC_CB_ERROR("runtime error: " CBNAME ": input data should be buffer");
			result->type = MKC_D_VOID;
			return 0;
		}
	}

	if (!len)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": input data length zero");
		result->type = MKC_D_VOID;
		return 0;
	}

	data = (char *)mkc_session_malloc(session, SHA512_LEN);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	SHA512_Init(&sha512ctx);

	for (i = 0; i < argc; i ++)	{
		p = MKC_CB_ARGV(i);
		if (p->length)	{
			SHA512_Update(&sha512ctx, p->buffer, p->length);
		}
		else	{
			continue;
		}
	}

	SHA512_Final((unsigned char *)data, &sha512ctx);
	result->type = MKC_D_BUF;
	result->buffer = data;
	result->length = SHA512_LEN;

	return 0;
}

#define SHA512HMAC
#undef CBNAME
#define CBNAME SHA512HMAC

/* sha512hmac */
/* param: source, key */
int
mkc_cb_sha512_hmac(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	unsigned int data_len;
	unsigned char * data;
	mkc_data * result;
	mkc_data * source;
	mkc_data * key;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(2);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_BUF);

	source = MKC_CB_ARGV(0);
	key = MKC_CB_ARGV(1);

	if (!key->length)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": key length is zero");
		result->type = MKC_D_VOID;
		return 0;
	}

	if (!source->length)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": data length is zero");
		result->type = MKC_D_VOID;
		return 0;
	}

	data = mkc_session_malloc(session, SHA512_LEN);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	data_len = SHA512_LEN;

	/* calculate md5 hmac */
	if (!HMAC(EVP_sha512()
			, key->buffer, key->length
			, (unsigned char *)source->buffer, source->length
			, data, &data_len)
			)
	{
		MKC_CB_ERROR("runtime error: " CBNAME ": failed to calculate");
		result->type = MKC_D_VOID;
		return 0;
	}

	result->type = MKC_D_BUF;
	result->buffer = data;
	result->length = SHA512_LEN;

	return 0;
}

/* eof */
