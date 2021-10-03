/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <openssl/aes.h>
#include "monkeycall.h"
#include "build-in-cbs/cbs.h"

#define AES_GROUP_LEN	(16)

#define AES256
#define CBNAME AES256

int
mkc_cb_aes256_encrypt(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	mkc_data * source;
	mkc_data * key;
	AES_KEY aes;
	char * data;
	int len;
	int r;
	int i;

	result = MKC_CB_RESULT;

	MKC_CB_ARGC_CHECK(2);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_BUF);

	source = MKC_CB_ARGV(0);
	key = MKC_CB_ARGV(1);

	if (!source->length)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": no input");
		result->type = MKC_D_VOID;
		return 0;
	}

	if (key->length < (256 / 8))	{
		MKC_CB_ERROR("runtime error: " CBNAME ": too short key");
		result->type = MKC_D_VOID;
		return 0;
	}

	r = AES_set_encrypt_key((const unsigned char *)key->buffer, 256, &aes);
	if (r < 0)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": failed to set AES encrypt key");
		result->type = MKC_D_VOID;
		return 0;
	}

	len = source->length;
	data = mkc_session_malloc(session, len);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	for (i = 0; (len - i) >= AES_GROUP_LEN; i += AES_GROUP_LEN)	{
		AES_encrypt(((unsigned char *)source->buffer) + i
				, (unsigned char *)data + i, &aes);
	}

	result->type = MKC_D_BUF;
	result->length = len;
	result->buffer = data;

	return 0;
}

#define AES256De
#undef CBNAME
#define CBNAME AES256De

int
mkc_cb_aes256_decrypt(mkc_session * session, mkc_cb_stack_frame * mkc_cb_stack)
{
	mkc_data * result;
	mkc_data * source;
	mkc_data * key;
	AES_KEY aes;
	char * data;
	int len;
	int i;
	int r;

	result = MKC_CB_RESULT;
	MKC_CB_ARGC_CHECK(2);
	MKC_CB_ARG_TYPE_CHECK(0, MKC_D_BUF);
	MKC_CB_ARG_TYPE_CHECK(1, MKC_D_BUF);

	source = MKC_CB_ARGV(0);
	key = MKC_CB_ARGV(1);

	if (!source->length)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": no input");
		result->type = MKC_D_VOID;
		return 0;
	}

	if (key->length < (256 / 8))	{
		MKC_CB_ERROR("runtime error: " CBNAME ": too short key");
		result->type = MKC_D_VOID;
		return 0;
	}

	r = AES_set_decrypt_key((const unsigned char *)key->buffer, 256, &aes);
	if (r < 0)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": failed to set AES encrypt key");
		result->type = MKC_D_VOID;
		return 0;
	}

	len = source->length;
	data = mkc_session_malloc(session, len);
	if (!data)	{
		MKC_CB_ERROR("runtime error: " CBNAME ": out of memory");
		result->type = MKC_D_VOID;
		return 0;
	}

	for (i = 0; (len - i) >= AES_GROUP_LEN; i += AES_GROUP_LEN)	{
		AES_decrypt(((unsigned char *)source->buffer) + i
				, (unsigned char *)data + i, &aes);
	}

	result->type = MKC_D_BUF;
	result->length = len;
	result->buffer = data;

	return 0;
}

/* eof */
