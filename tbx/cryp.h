#ifndef _cryp_
#define _cryp_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	cryp_enc_null = 0,

	cryp_aes_256_cbc,
	cryp_aes_256_ecb,
	cryp_aes_256_cbf,
	cryp_aes_256_ofb,

	cryp_rc4,
	
	cryp_idea_cbc,
	cryp_idea_ecb,
	cryp_idea_cbf,
	cryp_idea_ofb,

	cryp_rc2_cbc,
	cryp_rc2_ecb,
	cryp_rc2_cfb,
	cryp_rc2_ofb,

	cryp_bf_cbc,
	cryp_bf_ecb,
	cryp_bf_cfb,
	cryp_bf_ofb,
	
	cryp_cast5_cbc, 
	cryp_cast5_ecb, 
	cryp_cast5_cfb, 
	cryp_cast5_ofb,

	cryp_last
} cryp_algo_t;


int 			cryp_cipher_keylen(cryp_algo_t algo);
char *			cryp_cipher_name(cryp_algo_t algo);

unsigned char * cryp_encrypt(unsigned char *data, size_t *len, cryp_algo_t algo, char *key, char *salt);
unsigned char * cryp_decrypt(unsigned char *data, size_t *len, cryp_algo_t algo, char *key, char *salt);
int 			cryp_encrypt_file(char *infile, char *outfile, cryp_algo_t algo, char *key, char *salt);
int 			cryp_decrypt_file(char *infile, char *outfile, cryp_algo_t algo, char *key, char *salt);

#ifdef __cplusplus
}
#endif
#endif
