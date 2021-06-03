
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    12/10/2012  1.0 Creation
*/   

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "mem.h"
#include "err.h"

char ssl_MODULE[]  = "SSL support";
char ssl_PURPOSE[] = "SSL Support";
char ssl_VERSION[] = "1.0";
char ssl_DATEVER[] = "12/10/2017";

typedef struct {
	SSL_CTX *ctx;
	SSL     *ssl;
	long	 options;
	long     verify_options;
	int		 verify_depth;
	int		 server;
	int		 client;
} ssl_t, *pssl_t;

#define _ssl_c_
#include "ssl.h"
#undef _ssl_c_

static char *
ssl_error() {
	return  (char *) ERR_reason_error_string(ERR_get_error());
}

static int
ssl_certificates(pssl_t ssl, char *certificate, char *key) {
    /* set the local certificate from CertFile */
    if (SSL_CTX_use_certificate_file(ssl->ctx, certificate, SSL_FILETYPE_PEM) <= 0) {
		err_error("cannot read certificate '%s' (%s)", certificate, ssl_error());
		return 1;
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if (SSL_CTX_use_PrivateKey_file(ssl->ctx, key, SSL_FILETYPE_PEM) <= 0) {
		err_error("cannot read key file '%s' (%s)", key, ssl_error());
		return 2;
    }
    /* verify private key: */
    if (!SSL_CTX_check_private_key(ssl->ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
		err_error("Private key fome does not match the public certificate");
		return 3;
    }
	return 0;
}

size_t ssl_sizeof() { return sizeof(ssl_t); }

pssl_t
ssl_new(ssl_method_t method, char *certificate, char *key, long options, long verify_options, int verify_depth) {
	pssl_t ssl;
	SSL_METHOD *mthd = NULL;

	if (SSL_library_init() < 0) return NULL;

	/* Load cryptos, et.al. */
    OpenSSL_add_all_algorithms();  
	/* Bring in and register error messages */
    SSL_load_error_strings();  
	/* Create new client-method instance */

	switch (method) {	
		case ssl_DTLS:
			mthd = (SSL_METHOD *) DTLS_method();
			break;
#if 0
		case ssl_SSLv3: 
			err_warning("SSLv3 is deprecated");
			mthd = (SSL_METHOD *) SSLv3_method();
			break;
#endif
		case ssl_SSLv23:
		case ssl_TLS:
		default:
			if (method != ssl_TLS && method != ssl_SSLv23) err_warning("unkonwn method, defauting to TLS (SSLv23)");
			mthd = (SSL_METHOD *) TLS_method();
	}

	if (!(ssl = (pssl_t) mem_zmalloc(ssl_sizeof()))) return NULL;
	if (!mthd) {
		err_error("could not get the required method");
		return ssl_destroy(ssl);
	}

	/* Create new context */
    if (!(ssl->ctx = SSL_CTX_new(mthd))) {
		err_error("cannot create ssl context");
		return ssl_destroy(ssl);
	}

	if (certificate && key && ssl_certificates(ssl, certificate, key)) {
		err_error("certificate problem");
		return ssl_destroy(ssl);
	}

	ssl->options        = options;
	ssl->verify_depth   = verify_depth;
	ssl->verify_options = verify_options;
	
	return ssl;
}

pssl_t 
ssl_destroy(pssl_t ssl) {
	if (ssl) {
		if (ssl->ssl) ssl_clean_connection(ssl);
		if (ssl->ctx) {
			SSL_CTX_free(ssl->ctx);
			ssl->ctx = NULL;
		}
		ERR_free_strings();
		EVP_cleanup();

		free(ssl);
	}
	return NULL;
}

int ssl_clean_connection(pssl_t ssl) {
	if (ssl || !ssl->ssl) return 1;
	SSL_shutdown(ssl->ssl);
	SSL_free(ssl->ssl);
	ssl->ssl = NULL;
	return 0;	
}

int ssl_connect(pssl_t ssl, int server) {
	if (!ssl) return 1;
	ssl->ssl = SSL_new(ssl->ctx);
	SSL_set_fd(ssl->ssl, server);
	if (SSL_connect(ssl->ssl) == -1) {   /* perform the connection */
        err_error(ssl_error());
	}
	return 0;
}

int    
ssl_accept(pssl_t ssl, int fd) {
	if (!ssl) return 1;
	ssl->ssl = SSL_new(ssl->ctx);
	SSL_set_fd(ssl->ssl, fd);
	if (SSL_accept(ssl->ssl) == -1) {   /* perform the connection */
        err_error(ssl_error());
		return -3;
	}
	return 0;
}

int
ssl_write(pssl_t ssl, char *buf, size_t len) {
	if (!ssl || !ssl->ssl) return -1;
	return SSL_write(ssl->ssl, buf, len);
}

int ssl_read(pssl_t ssl, char *buf, size_t len) {
	if (!ssl || !ssl->ssl) return -1;
	return SSL_read(ssl->ssl, buf, len);
}

char * ssl_cipher_get(pssl_t ssl) {
	if (!ssl || !ssl->ssl) return NULL;
	return (char *) SSL_get_cipher(ssl->ssl);
}

char *ssl_certificate_get(pssl_t ssl) {
	BIO  *bio;
	X509 *cert;
	char *str, *rstr = NULL;

	if (!ssl || !ssl->ssl) 
		return NULL;
	if (!(cert = SSL_get_peer_certificate(ssl->ssl))) 
		return NULL;
	if (!(bio = BIO_new(BIO_s_mem()))) 
		return NULL;
	
	X509_print(bio, cert); 
	BIO_get_mem_data(bio, &str);
	
	if (str) { 
		rstr = strdup(str);
	}
	BIO_free_all(bio);

    return rstr;
}
