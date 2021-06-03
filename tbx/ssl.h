#ifndef _ssl_h_
#define _ssl_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/ssl.h>

/* methods: */
typedef enum {
	ssl_SSLv23,
	ssl_TLS,
	ssl_DTLS
} ssl_method_t;

/* Verify options :

	SSL_VERIFY_NONE               
		Server mode: 
			the server will not send a client certificate request to the client, so the client will not send a 
			certificate.

		Client mode: 
			if not using an anonymous cipher (by default disabled), the server will send a certificate which will 
			be checked. The result of the certificate verification process can be checked after the TLS/SSL 
			handshake using the ssl_get_verify_result(3) function. The handshake will be continued regardless 
			of the verification result. 

	SSL_VERIFY_PEER
    	Server mode: 
			the server sends a client certificate request to the client. The certificate returned (if any) is 
			checked. If the verification process fails, the TLS/SSL handshake is immediately terminated with 
			an alert message containing the reason for the verification failure. The behaviour can be controlled 
			by the additional SSL_VERIFY_FAIL_IF_NO_PEER_CERT and SSL_VERIFY_CLIENT_ONCE flags.

    	Client mode: 
			the server certificate is verified. If the verification process fails, the TLS/SSL handshake is 
			immediately terminated with an alert message containing the reason for the verification failure. 
			If no server certificate is sent, because an anonymous cipher is used, SSL_VERIFY_PEER is ignored. 

	SSL_VERIFY_FAIL_IF_NO_PEER_CERT
		Server mode: 
			if the client did not return a certificate, the TLS/SSL handshake is immediately terminated with a 
			"handshake failure" alert. This flag must be used together with SSL_VERIFY_PEER .

		Client mode: 
			ignored 

	SSL_VERIFY_CLIENT_ONCE
		Server mode: 
			only request a client certificate on the initial TLS/SSL handshake. Do not ask for a client certificate 
			again in case of a renegotiation. This flag must be used together with SSL_VERIFY_PEER .

		Client mode: 
			ignored 

	Example:
		SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT

*/


/* Options :
	Options are the ones set by SSL_set_options() 
	options are aggregated w/ binary ORs (e.g. SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1|SSL_OP_ALL).

	Main options are:
	SSL_OP_CIPHER_SERVER_PREFERENCE  -> Force server side cipher preference (by defaut, client preferd cifer is used)
	SSL_OP_NO_SSLv3					 -> No SSLv3
	SSL_OP_NO_TLSv1				     -> No TLSv1
	SSL_OP_NO_TLSv1_2				 -> No TLSv1.2
	SSL_OP_NO_TLSv1_1				 -> No TLSv1.1
	SSL_OP_NO_DTLSv1				 -> No DTLSv1
	SSL_OP_NO_DTLSv1_2				 -> No DTLSv1.2
	SSL_OP_ALL 						 -> all workarrounds

	For more options, please have a look to openssl/ssl.h file 
	or documention  https://wiki.openssl.org/index.php/Manual:SSL_CTX_set_options(3).
*/
	
#ifndef _ssl_c_
typedef void *pssl_t;
#endif

size_t ssl_sizeof();

pssl_t ssl_new(ssl_method_t method, char *certificate, char *key, long options, long verify_options, int verify_depth);
pssl_t ssl_destroy(pssl_t ssl);
int    ssl_clean_connection(pssl_t ssl);
/* 
int    ssl_verify_option(pssl_t ssl, long option);
int    ssl_option(pssl_t ssl, long options);
int	   ssl_require_valid_client_cert(pssl_t ssl, int true_false);
*/

int    ssl_connect(pssl_t ssl, int server);
int    ssl_accept(pssl_t ssl, int port); 

int    ssl_write(pssl_t ssl, char *buf, size_t len);
int    ssl_read(pssl_t ssl, char *buf, size_t len);

char * ssl_certificate_get(pssl_t ssl);
char * ssl_cipher_get(pssl_t ssl);

#ifdef __cplusplus
}
#endif

#endif
