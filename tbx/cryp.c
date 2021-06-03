
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    20/11/2017  1.0	Creation
*/   

#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include <openssl/evp.h>

#include "futl.h"

#include "cryp.h"

char cryp_MODULE[]  = "Encrypt/decryp";
char cryp_PURPOSE[] = "Encrypt/decrypt w/ different algorythms, w/ or w/o salt";
char cryp_VERSION[] = "1.0";
char cryp_DATEVER[] = "20/11/2017";

typedef EVP_CIPHER *(*f_cipher_t)(void);

typedef struct {
	cryp_algo_t algo;
	int klen;
	f_cipher_t cipher;
	char name[50];	
} cryp_info_t, *pcryp_info_t;	

static cryp_info_t cryp_infos[] = {
	{cryp_enc_null,      0, (f_cipher_t) EVP_enc_null,    "enc_null"},

	{cryp_aes_256_cbc, 256, (f_cipher_t) EVP_aes_256_cbc, "aes_256_cbc"},
	{cryp_aes_256_ecb, 256, (f_cipher_t) EVP_aes_256_ecb, "aes_256_ecb"},
	{cryp_aes_256_cbf, 256, (f_cipher_t) EVP_aes_256_cfb, "aes_256_cfb"},
	{cryp_aes_256_ofb, 256, (f_cipher_t) EVP_aes_256_ofb, "aes_256_ofb"},

	{cryp_rc4,         128, (f_cipher_t) EVP_rc4,         "rc4"},
	
	{cryp_idea_cbc,    128, (f_cipher_t) EVP_idea_cbc,    "idea_cbc"},
	{cryp_idea_ecb,    128, (f_cipher_t) EVP_idea_ecb,    "idea_ecb"},
	{cryp_idea_cbf,    128, (f_cipher_t) EVP_idea_cfb,    "idea_cfb"},
	{cryp_idea_ofb,    128, (f_cipher_t) EVP_idea_ofb,    "idea_ofb"},

	{cryp_rc2_cbc,       0, (f_cipher_t) EVP_rc2_cbc,     "rc2_cbc"},
	{cryp_rc2_ecb,       0, (f_cipher_t) EVP_rc2_ecb,     "rc2_ecb"},
	{cryp_rc2_cfb,       0, (f_cipher_t) EVP_rc2_cfb,     "rc2_cfb"},
	{cryp_rc2_ofb,       0, (f_cipher_t) EVP_rc2_ofb,     "rc2_ofb"},

	{cryp_bf_cbc,        0, (f_cipher_t) EVP_bf_cbc,      "bf_cbc"},
	{cryp_bf_ecb,        0, (f_cipher_t) EVP_bf_ecb,      "bf_ecb"},
	{cryp_bf_cfb,        0, (f_cipher_t) EVP_bf_cfb,      "bf_cfb"},
	{cryp_bf_ofb,        0, (f_cipher_t) EVP_bf_ofb,      "bf_ofb"},
	
	{cryp_cast5_cbc,     0, (f_cipher_t) EVP_cast5_cbc,   "cast5_cbc"}, 
	{cryp_cast5_ecb,     0, (f_cipher_t) EVP_cast5_ecb,   "cast5_ecb"},
	{cryp_cast5_cfb,     0, (f_cipher_t) EVP_cast5_cfb,   "cast5_cfb"},
	{cryp_cast5_ofb,     0, (f_cipher_t) EVP_cast5_ofb,   "cast5_ofb"},

	{cryp_last,         -1, NULL,                         "invalid cipher algorithm"}
};

int cryp_cipher_keylen(cryp_algo_t algo) {
	if (algo < cryp_enc_null || algo >= cryp_last) {
		return cryp_infos[cryp_last].klen;
	}
	return cryp_infos[algo].klen;
}

f_cipher_t
cryp_cipher_fct(cryp_algo_t algo) {
	if (algo < cryp_enc_null || algo >= cryp_last) {
		return cryp_infos[cryp_last].cipher;
	}
	return cryp_infos[algo].cipher;
}

char *
cryp_cipher_name(cryp_algo_t algo) {
	if (algo < cryp_enc_null || algo >= cryp_last) {
		return cryp_infos[cryp_last].name;
	}
	return cryp_infos[algo].name;
}

/*** https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption ***/

unsigned char *
cryp_encrypt(unsigned char *data, size_t *len, cryp_algo_t algo, char *key, char *salt) {
	unsigned char *output, *p ;
	int olen, wlen;
	char iv[32];
	EVP_CIPHER_CTX *ctx;
	memset(iv, 0, 32);
	
	if (algo < cryp_enc_null || algo >= cryp_last) return NULL;

    if (!(ctx = EVP_CIPHER_CTX_new())) {
		return NULL;
	}

	olen = *len + 16 - (*len % 16);

	if (!(output = (unsigned char *) malloc(olen))) {
		EVP_CIPHER_CTX_free(ctx);
		return NULL;
	}
	memset(output, 0, olen);
	p = output;
	wlen = 0;
    EVP_EncryptInit_ex(ctx, cryp_cipher_fct(algo)(), NULL, NULL, NULL);
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 7, NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, (const unsigned char *) key, (const unsigned char *) iv);
    EVP_EncryptUpdate(ctx, p, &wlen, data, *len);
	EVP_EncryptFinal_ex(ctx, p + wlen, &olen);
	*len = wlen + olen;
	EVP_CIPHER_CTX_free(ctx);
	return output;	
}

unsigned char *
cryp_decrypt(unsigned char *data, size_t *len, cryp_algo_t algo, char *key, char *salt) {
	unsigned char iv[32];
    int olen, olen2;
	unsigned char *out;
	EVP_CIPHER_CTX *ctx;

	if (algo < cryp_enc_null || algo >= cryp_last) return NULL;

	memset(iv, 0, 32);

	if (!(ctx = EVP_CIPHER_CTX_new())) return NULL;

    EVP_DecryptInit_ex(ctx, cryp_cipher_fct(algo)(), NULL, NULL, NULL);
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 7, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, (const unsigned char *) key, (const unsigned char *) iv);

	olen = *len;
	if (!(out = malloc(olen * sizeof(unsigned char)))) {	
		return NULL;	
	}
	memset(out, 0, olen);

    EVP_DecryptUpdate(ctx, out,  &olen, data, *len);

	EVP_DecryptFinal_ex(ctx, out + olen, &olen2);
    *len = olen + olen2;
	EVP_CIPHER_CTX_free(ctx);
	return out;	
}

int
cryp_encrypt_file(char *infile, char *outfile, cryp_algo_t algo, char *key, char *salt) {
	char *in, *out;
	size_t size;
	if (algo < cryp_enc_null || algo >= cryp_last) return 1;
	if (!(in  = futl_load(infile, &size)))         return 2;
	if (!(out = (char *) cryp_encrypt((unsigned char *) in, &size, algo, key, salt))) {
		free(in);
		return 3;
	}
	if (futl_write(outfile, out, size)) {
		free(in); free(out);
		return 4;
	}
	free(in); free(out);
	return 0;
}

int
cryp_decrypt_file(char *infile, char *outfile, cryp_algo_t algo, char *key, char *salt) {
	char *in, *out;
	size_t size;
	if (algo < cryp_enc_null || algo >= cryp_last) return 1;
	if (!(in  = futl_load(infile, &size)))         return 2;
	if (!(out = (char *) cryp_decrypt((unsigned char *) in, &size, algo, key, salt))) {
		free(in);
		return 3;
	}
	if (futl_write(outfile, out, size)) {
		free(in); free(out);
		return 4;
	}
	free(in); free(out);
	return 0;
}

#ifdef _test_cryp_
#include "err.h"
int main(int n, char *a[]) {
	cryp_algo_t i;
	char *raw, *enc, *dec;
	size_t size, es, ds;

	char salt[] = "0123456789abcdef";

	err_init(NULL, err_INFO);
	
	err_info("read %s", a[0]);
	if (!(raw = futl_load(a[0], &size))) {
		err_error("cannot open %s", a[0]);
		return 1;
	}
	err_info("Raw data size: %lo", size);	
	err_info("Checking algos from %d to %d", cryp_enc_null, cryp_last -1); 
	for (i = cryp_enc_null; i < cryp_last; i++) {
		err_info("encrypt w/ algo %d (%s)", i, cryp_cipher_name(i));
		es  = size;
		if (!(enc = (char *) cryp_encrypt((unsigned char *) raw, &es, i, a[0], salt))) {
			err_error("cannot encrypt w/ %s", cryp_cipher_name(i));
		} else {
			err_info("encrypted data size: %lo", es);
			ds = es;
			if (!(dec = (char *) cryp_decrypt((unsigned char *) enc, &ds, i, a[0], salt))) {
				err_error("cannot decrypt w/ %s", cryp_cipher_name(i)); 
			} else {
				err_info("decrypted data size: %lo", ds);
				if (size != ds) {
					err_error("decrypted data has not thre correct size %lo vs raw %lo", ds, size);
				} else if (memcmp(raw, dec, size)) {
					err_error("raw and dec differ w/ %s", cryp_cipher_name(i)); 
				} else {
					err_info("Algo %d (%s) ok", i, cryp_cipher_name(i));
				}
				free(dec);
			}
			free(enc);
		}
	}
	return 0;
}
#endif 
#if 0
#ifdef _cryp_cli_
int main(int n, char *a[]) {
	char *in, *out;
	size_t il, ol;

	if (n < 3) return 1;
	if (!strcmp("test-encryp", basename(a[0]))) {
		if (!(in = futl_load(a[1], &il))) return 2;

		ol = il;

		if (!(out = (char *) cryp_encrypt((unsigned char *) in, &ol, cryp_aes_256_cbc, a[2], salt))) {
			free(in);
			return 3;
		} 
		futl_write("out.enc", out, ol);
		free(in);
		free(out);
	} else if (!strcmp("test-decryp", basename(a[0]))) {
		if (!(in = futl_load(a[1], &il))) return 2;

		ol = il;

		if (!(out = (char *) cryp_decrypt((unsigned char *) in, &ol, cryp_aes_256_cbc, a[2], salt))) {
			free(in);
			return 3;
		} 
	printf("ol = %lu\n", ol);
		futl_write("out.dec", out, ol);
		free(in);
		free(out);
	} else {
		return 4;
	}
	return 0;
}
#endif
#endif /*0*/
