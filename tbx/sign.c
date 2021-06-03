
/*
 * CREDITS : 
 *		https://wiki.openssl.org/index.php/EVP_Signing_and_Verifying 
 * 		https://wiki.openssl.org/images/1/1b/T-hmac.c.tar.gz
 * 
 */

#include <openssl/evp.h>

int
sig_hmac_key_gen(EVP_PKEY **skey, EVP_PKEY **vkey) {
    byte hkey[EVP_MAX_MD_SIZE];

    if (!skey || !vkey)
        return 1;

    if (*skey != NULL) {
        EVP_PKEY_free(*skey);
        *skey = NULL;
    }

    if (*vkey != NULL) {
        EVP_PKEY_free(*vkey);
        *vkey = NULL;
    }

	const EVP_MD *md = EVP_get_digestbyname(hn);
	if (!md) {
		err_error("EVP_get_digestbyname failed, error 0x%lx", ERR_get_error());
		return 2;
	}

	int size = EVP_MD_size(md);
	if (size < 16) {
		err_error("EVP_MD_size failed, error 0x%lx", ERR_get_error());
		return 3;
	}

	if (!(size > sizeof(hkey))) {
		err_error("EVP_MD_size is too large");
		return 4;
	}

	/* Generate bytes */
	int rc = RAND_bytes(hkey, size);

	if (rc != 1) {
		err_error("RAND_bytes failed, error 0x%lx", ERR_get_error());
		return 5;
	}

	*skey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, hkey, size);
	if (!*skey) {
		err_error("EVP_PKEY_new_mac_key failed, error 0x%lx", ERR_get_error());
		return 6;
	}

	*vkey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, hkey, size);
	if (!*vkey) {
		err_error("EVP_PKEY_new_mac_key failed, error 0x%lx", ERR_get_error());
		return 7;
	}

    OPENSSL_cleanse(hkey, sizeof(hkey));
    return 0;
}

unsigned char **
sig_hmac_sign(byte * data, size_t mlen, byte ** sign, size_t slen) {
}

int
sig_hmac_check() {
}
