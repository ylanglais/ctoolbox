#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

int errno;

#include "err.h"
#include "ssl.h"
#include "mem.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		30/05/2016	2.1		Add changelog
		11/10/2017	3.0 	Add SSL support

*/

char cli_MODULE[]  = "Client";
char cli_PURPOSE[] = "Client/Server client network socket encapsulation";
char cli_VERSION[] = "3.0";
char cli_DATEVER[] = "11/10/2017";

typedef struct {
	char 			 *hostname;
	struct addrinfo	 *ai;
	int               socket;
	int               port;
	int 			  send_opts;
	int 			  recv_opts;
	/** ssl part: **/
	pssl_t			  ssl;
	ssl_method_t	  method;
	char *			  cert;
	char *			  key;
	long			  ssl_opts;
	long			  chk_opts;
	int 			  chk_depth;
} cli_t, *pcli_t;

void cli_dump(pcli_t c) {
	if (!c) {
		err_debug("c = NULL");
		return;
	}
	err_debug("c                   = %x", c);
	if (!c->hostname) err_debug("c->hostname = NULL");
	else err_debug("c->hostname         = %s", c->hostname);
	if (!c->ai)       err_debug("c->ai = NULL");
	else {
		err_debug("c->ai->ai_flags     = %d", c->ai->ai_flags);
		err_debug("c->ai->ai_family    = %d", c->ai->ai_family);
		err_debug("c->ai->ai_socktype  = %d", c->ai->ai_socktype); 
		err_debug("c->ai->ai_protocol  = %d", c->ai->ai_protocol);
		err_debug("c->ai->ai_addrlen   = %u", c->ai->ai_addrlen);
		err_debug("c->ai->ai_canonname = %S", c->ai->ai_canonname);
		err_debug("c->ai->ssl          = %x", c->ssl);
	}
	err_debug("c->send_opts = %u", c->send_opts);
	err_debug("c->recv_opts = %u", c->recv_opts);
} 

#define _cli_c_
#include "cli.h"
#undef _cli_c_

pcli_t
cli_destroy(pcli_t c) {
	if (c) {
		if (c->hostname) free(c->hostname);
		if (c->ai)       freeaddrinfo(c->ai);
		if (c->ssl)      ssl_destroy(c->ssl);
		if (c->cert)     free(c->cert);
		if (c->key)      free(c->key);
		close(c->socket);
		free(c);
	}
	return NULL;
}

size_t cli_sizeof() { return sizeof(cli_t); }

pcli_t
cli_new(char *host, int port) {
	pcli_t c;
	char sport[15];
	int r;
	struct addrinfo hints;

	if (!(c = (pcli_t) mem_zmalloc(sizeof(cli_t)))) {
		err_error("cli no memory for cli structure");
		return NULL;	
	}
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	sprintf(sport, "%d", port);

	if ((r = getaddrinfo(host, sport, &hints, &c->ai))) {
		err_error("Cannot get host/port (%s/%d) information: %s\n", host, port, gai_strerror(r));
		return NULL;	
	}

	c->hostname = strdup(host);
	c->port = port;

	if ((c->socket = socket(c->ai->ai_family, c->ai->ai_socktype, c->ai->ai_protocol)) <= 0) { 
		err_error("cli error opening socket: %s", strerror(errno));
		return cli_destroy(c);
	}

	if ((r = connect(c->socket, c->ai->ai_addr, c->ai->ai_addrlen))) {
		err_error("cli cannot connect to server %s:%d, reason (connect returned %d) : %s", host, port, r, strerror(errno));
		return cli_destroy(c);
	}
	c->send_opts = 0;
	c->recv_opts = 0;
	return c;
}

pcli_t
cli_ssl_new(char *host, int port, ssl_method_t method, char *certificate, char *key, long options, long verify_options, int verify_depth) {
	pcli_t c;
	pssl_t ssl;

	if (!(ssl = ssl_new(method, certificate, key, options, verify_options, verify_depth))) 
		return NULL;

	if (!(c = cli_new(host, port))) {
		ssl_destroy(ssl);
		return NULL;
	}
	c->ssl       = ssl;
	c->method    = method;
	if (certificate) c->cert = strdup(certificate);
	if (key)         c->key  = strdup(key);
	c->ssl_opts  = options;
	c->chk_opts  = verify_options;
	c->chk_depth = verify_depth;

	if (ssl_connect(c->ssl, c->socket)) {
		err_error("cannot initiate ssl connection");
		c->ssl = ssl_destroy(c->ssl);
		return   cli_destroy(c);
	}
	err_debug("ssl is working");

	return c;
}

pcli_t
cli_reconnect(pcli_t c) {
	pcli_t cl;

	if (!c) return NULL;

	if (c->ssl) 
		cl = cli_ssl_new(c->hostname, c->port, c->method, c->cert, c->key, c->ssl_opts, c->chk_opts, c->chk_depth);
	else
		cl = cli_new(c->hostname, c->port);

	cli_recv_opts_set(cl, c->recv_opts);
	cli_send_opts_set(cl, c->send_opts);
		
	cli_destroy(c);
	return cl;
}

int 
cli_recv_opts_set(pcli_t c, int recv_opts) {
	if (!c) return 1;
	c->recv_opts = recv_opts;	
	return 0;
}

int 
cli_send_opts_set(pcli_t c, int recv_opts) {
	if (!c) return 1;
	c->recv_opts = recv_opts;	
	return 0;
}

char *
cli_hostname(pcli_t c) {
	if (!c) return NULL;
	return c->hostname;
}

int cli_socket(pcli_t c) {
	if (!c) return -1;
	return c->socket;
}

int cli_port(pcli_t c) {
	if (!c) return -1;
	return c->port;
}

char *
cli_cipher(pcli_t c) {
	if (!c || !c->ssl) return NULL;
	return ssl_cipher_get(c->ssl);
}

char *
cli_certificate(pcli_t c) {
	if (!c || !c->ssl) return NULL;
	return ssl_certificate_get(c->ssl);
}

static int _cli_send(pcli_t c, char *data, size_t size) { 
	if (!c) return 0;
	if (!c->ssl) return send(c->socket, (void *) data, size, c->send_opts);
	else         return ssl_write(c->ssl, data, size);
}

int
cli_send_to(pcli_t c, char *data, size_t size, tstamp_t timeout) {
	int r;
	fd_set fdwrite;

	if (!c) return 0;

	FD_ZERO(&fdwrite);
	FD_SET(c->socket, &fdwrite);

	r = select(c->socket + 1, (fd_set *) 0, &fdwrite, (fd_set *) 0, (struct timeval *) &timeout);

	err_debug("r = %d", r);

	if (r < 0) {
		err_error("select error : %s", strerror(errno));
		return 0;
	} else if (r == 0) {
		err_error("timeout occured aftrer %d.%d seconds", timeout.tv_sec, timeout.tv_usec);
		return 0;
	}
	if ((r = _cli_send(c, data, size)) <= 0) {
		err_error("cannot send to server: %s", strerror(errno));
		return r;
	}
	return r;
}

int
cli_send(pcli_t c, char *data, size_t size) {
	int r;
	if (!c) return 0;
	if ((r = _cli_send(c, data, size)) <= 0) {
		err_error("cannot send to server: %s", strerror(errno));
		return r;
	}
	return r;
}

static int
_cli_recv(pcli_t c,  char *data, size_t size) {
	if (!c) return 0;
	if (!c->ssl) return recv(c->socket, (void *) data, size, c->recv_opts);
	else         return ssl_read(c->ssl, data, size);
}

int
cli_recv_to(pcli_t c, char *data, size_t size, tstamp_t timeout) {
	int r;
	fd_set fdread;

	FD_ZERO(&fdread);
	FD_SET(c->socket, &fdread);
	
	r = select(c->socket + 1, &fdread, (fd_set *) 0, (fd_set *) 0, (struct timeval *) &timeout);
	if (!r) {
		err_error("cli_recv_to timeout");
		return 0;
	}
	if (r < 0) {
		err_error("cli_recv_to select error : %s", strerror(errno));
		return r;
	}
		
	return _cli_recv(c, data, size);
}

int
cli_recv(pcli_t c, char *data, size_t size) {
	if (!data) return 0;
	return _cli_recv(c, data, size);
}

