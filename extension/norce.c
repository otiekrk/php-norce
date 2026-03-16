/* norce extension for PHP */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "norce_key.h"
#include "php.h"
#include "ext/standard/info.h"
#include "php_norce.h"
#include "norce_arginfo.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ec.h>

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

#define NORCE_FILENAME_MAX_LENGTH 136
#define NORCE_HASH_LENGTH 32
#define NORCE_STR_HASH_LENGTH 65
#define NORCE_SIGNATURE_LENGTH 64

static zend_op_array *(*original_compile_file)(zend_file_handle *file_handle, int type);
static zend_op_array *(*original_compile_string)(zend_string *source_string, const char *filename, zend_compile_position position);

static int hashBytes(char *data, size_t dataLength, unsigned char *hashValue) {
	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

	if (mdctx == NULL) {
		ERR_print_errors_fp(stderr);
		return 0;
	}

	if (1 != EVP_DigestInit_ex2(mdctx, EVP_MD_fetch(NULL, "sha256", NULL), NULL)) {
		ERR_print_errors_fp(stderr);
		return 0;
	}

	if (!EVP_DigestUpdate(mdctx, data, dataLength)) {
		ERR_print_errors_fp(stderr);
		EVP_MD_CTX_free(mdctx);
		return 0;
	}
	unsigned int hashLength;
	if (!EVP_DigestFinal_ex(mdctx, hashValue, &hashLength)) {
		ERR_print_errors_fp(stderr);
		EVP_MD_CTX_free(mdctx);
		return 0;
	}

	EVP_MD_CTX_free(mdctx);

	return (int)hashLength;
}

static int verifySignature(const unsigned char *signature, const size_t signatureLength, const unsigned char *data, const size_t dataLength) {
	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	EVP_PKEY *key = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, norce_key, NORCE_HASH_LENGTH);
	EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new(key, NULL);

	if (mdctx == NULL) {
		return 0;
	}
	if (1 != EVP_DigestVerifyInit_ex(mdctx, &pctx, NULL, NULL, NULL, key, NULL)) {
		return 0;
	}

	int result = EVP_DigestVerify(mdctx, signature, signatureLength, data, dataLength);
	if (1 != result) {
		return 0;
	}

	EVP_PKEY_free(key);
	EVP_MD_CTX_free(mdctx);

	return 1;
}

static zend_op_array *norce_compile_file(zend_file_handle *file_handle, int type) {
	size_t len = ZSTR_LEN(file_handle->filename);
	char *filename = ZSTR_VAL(file_handle->filename);

	if (filename) {
		unsigned char hashValue[NORCE_HASH_LENGTH];
		char hashStrValue[NORCE_STR_HASH_LENGTH];
		int hashLength = hashBytes(filename, len, hashValue);

		for (int i = 0; i < hashLength; i++) {
			sprintf(&hashStrValue[i * 2], "%02x", hashValue[i]);
		}

		char filename_signature[NORCE_FILENAME_MAX_LENGTH];

		sprintf(filename_signature, "/%s/%s.sign", norce_signature_dir, hashStrValue);

		FILE *file_php = fopen(filename,"r");
		FILE *file_signature = fopen(filename_signature, "r");

		if (file_php == NULL) {
			return NULL;
		}

		if (file_signature == NULL) {
			php_printf("NoRCE: Untrusted PHP file.\n");
			return NULL;
		}

		char *data = NULL;
		size_t dataLength;

		fseek(file_php, 0L, SEEK_END);
		dataLength = ftell(file_php);
		fseek(file_php, 0L, SEEK_SET);
		data = (char*)malloc(dataLength);
		fread(data, sizeof(char), dataLength, file_php);

		fclose(file_php);

		unsigned char *signature = NULL;

		size_t signatureLength = NORCE_SIGNATURE_LENGTH;
		signature = (unsigned char*)malloc(signatureLength);
		fread(signature, sizeof(unsigned char), signatureLength, file_signature);

		fclose(file_signature);

		int result = verifySignature(signature, signatureLength, (unsigned char*)data, dataLength);
		free(data);
		free(signature);

		if (result != 1) {
			php_printf("NoRCE: Untrusted PHP file.\n");
			return NULL;
		}

		return original_compile_file(file_handle, type);
	}

	return NULL;
}

static zend_op_array *norce_compile_string(zend_string *string, const char *filename, zend_compile_position position) {
	size_t len = ZSTR_LEN(string);
	char *script_string = ZSTR_VAL(string);

	unsigned char hashValue[NORCE_HASH_LENGTH];
	char hashStrValue[NORCE_STR_HASH_LENGTH];
	int hashLength = hashBytes(script_string, len, hashValue);

	for (int i = 0; i < hashLength; i++) {
		sprintf(&hashStrValue[i * 2], "%02x", hashValue[i]);
	}

	char filename_signature[NORCE_FILENAME_MAX_LENGTH];

	sprintf(filename_signature, "/%s/%s.snip", norce_snippet_dir, hashStrValue);

	FILE *file_signature = fopen(filename_signature, "r");

	if (file_signature == NULL) {
		php_printf("NoRCE: Untrusted PHP script.\n",filename_signature,script_string);
		return NULL;
	}
	size_t signatureLength = NORCE_SIGNATURE_LENGTH;
	unsigned char *signature[signatureLength];
	fread(signature, sizeof(unsigned char), signatureLength, file_signature);

	fclose(file_signature);

	int result = verifySignature(signature, signatureLength, (unsigned char*)script_string, len);

	if (result != 1) {
		php_printf("NoRCE: Untrusted PHP script.\n");
		return NULL;
	}

	return original_compile_string(string, filename, position);
}

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(norce)
{
#if defined(ZTS) && defined(COMPILE_DL_NORCE)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(norce)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "norce support", "enabled");
	php_info_print_table_end();
}
/* }}} */

PHP_MINIT_FUNCTION(norce) {
	original_compile_file = zend_compile_file;
	original_compile_string = zend_compile_string;

	zend_compile_file = norce_compile_file;
	zend_compile_string = norce_compile_string;
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(norce) {
	zend_compile_file = original_compile_file;
	zend_compile_string = original_compile_string;
	return SUCCESS;
}

/* {{{ norce_module_entry */
zend_module_entry norce_module_entry = {
	STANDARD_MODULE_HEADER,
	"norce",					/* Extension name */
	NULL,					/* zend_function_entry */
	PHP_MINIT(norce),							/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(norce),							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(norce),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(norce),			/* PHP_MINFO - Module info */
	PHP_NORCE_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_NORCE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(norce)
#endif
