#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "ssl.h"
#include "cli.h"

int main(void) {
	char  buf[1024];
    char  hostname[] = "127.0.0.1";
    int   port 		= 5000;

    char  cert[]     = "./cli/mycert.pem";
    char  key[]      = "./cli/mycert.pem";

	char *msg        = "Hello???";
	char *s; 

	pcli_t c;

	err_init(NULL, err_TRACE);

	if (!(c = cli_ssl_new(hostname, port, ssl_TLS, cert, key, 0, SSL_VERIFY_NONE, 0))) {
		return 1;
	}

	err_info("connected");

	if ((s = cli_cipher(c))) { 
		err_info("cipher is      '%s'", cli_cipher(c));
	}
	if ((s  = cli_certificate(c))) {
		err_info("certificate is :\n%s'", s);
		free(s);
	}
	cli_send(c, msg, strlen(msg));        /* encrypt & send message */
	cli_recv(c, buf, sizeof(buf)); /* get reply & decrypt */

	err_info("recieved: '%s'", buf);
	cli_destroy(c);
	return 0;

}
