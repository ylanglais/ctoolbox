#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tbx/err.h>
#include "ssl.h"
#include "srv.h"

int
srv_cli_dialog(psrv_t srv, int srvnum, int cid, void *foo) {
	char buf[1024];
    char reply[1024];
    int  bytes;
    const char *HTMLecho = "<html><body><pre>%s</pre></body></html>\n\n";
	char *s;

	if (!srv) return 1;

	err_debug("client connected to srv %d (pid %d)", srvnum, getpid());

	if ((s = srv_cipher(srv))) { 
		err_info("cipher is      '%s'", srv_cipher(srv));
	}
	if ((s  = srv_certificate(srv))) {
		err_info("certificate is :\n%s'", srv);
		free(s);
	}

	bytes = srv_recv(srv, buf, sizeof(buf));    /* get request */
	if (bytes > 0) {
		buf[bytes] = 0;
		printf("Client msg: \"%s\"\n", buf);
		sprintf(reply, HTMLecho, buf);  /* construct reply */
		srv_send(srv, reply, strlen(reply));   /* send reply */
	} else err_error("problem recieving data");

	return 0;
}

int
main() {
 	psrv_t srv;
    char cert[] = "./srv/mycert.pem";
    char key[]  = "./srv/mycert.pem";

	
	err_level_set(err_DEBUG);

	if (!(srv = srv_ssl_new(NULL, 5000, 5, ssl_TLS, cert, key, 0, SSL_VERIFY_NONE, 0))) return 1;

	err_debug("server %d created", getpid());

	srv_cli_connect_loop(srv, srv_cli_dialog, NULL, NULL);

#if 0
	tstamp_t	timeout;
	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;
	srv_cli_connect_loop_to(srv, srv_cli_dialog, NULL, NULL, timeout);
#endif

	srv = srv_destroy(srv);
	err_debug("srv %d client destroyed", getpid());
	return 0;
}
