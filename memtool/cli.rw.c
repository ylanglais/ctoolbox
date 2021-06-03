
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

#include <err.h>

typedef struct {
	int   socket;
	int   port;
	struct hostent *host;
} cli_t, *pcli_t;

#define _cli_c_
#include <cli.h>
#undef _cli_c_

struct hostent *cli_host_by_name(char *name) { return gethostbyname(name); }
struct hostent *cli_host_by_ip(char *ip)     { return gethostbyaddr((char *) inet_addr(ip), 4, AF_INET); }

pcli_t
cli_destroy(pcli_t c) {
	if (c) {
		close(c->socket);
		free(c);
	}
	return NULL;
}

size_t cli_sizeof() { return sizeof(cli_t); }

pcli_t
cli_new(struct hostent *host, int port) {
	pcli_t c;
	static int one = 1;
	struct sockaddr_in sin;
	int r;

	if (!host) {
		err_error("cli null host pointer");
		return NULL;
	} else err_debug("cli:  hostname = %s", host->h_name);

	if (!(c = (pcli_t) malloc(sizeof(cli_t)))) {
		err_error("cli no memory for cli structure");
		return NULL;	

	}

	c->host = host;
	c->port = port;

    bzero(&sin, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(c->port);
	memcpy((char *) &sin.sin_addr, c->host->h_addr, c->host->h_length);

	if ((c->socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) { 
		err_error("cli error opening socket: %s", strerror(errno));
		return cli_destroy(c);
	}
/*
	if (setsockopt(c->socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0) {
		err_error("cli error while setting socket options : %s", strerror(errno));
		return cli_destroy(c);
	}
*/
	if ((r = connect(c->socket, (struct sockaddr *) &sin, sizeof(struct sockaddr)))) {
		err_error("cli cannot connect to server %s:%d, reason (connect returned %d) : %s", host->h_name, port, r, strerror(errno));
		return cli_destroy(c);
	}
	return c;
}

struct hostent *
cli_host(pcli_t c) {
	if (!c) return NULL;
	return c->host;
}	

int cli_socket(pcli_t c) {
	if (!c) return -1;
	return c->socket;
}

int cli_port(pcli_t c) {
	if (!c) return -1;
	return c->port;
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
	if ((r = write(c->socket, (void *) data, size)) <= 0) {
		err_error("cannot write to server: %s", strerror(errno));
		return 0;
	}
	return r;
}

int
cli_send(pcli_t c, char *data, size_t size) {
	int r;
	if (!c) return 0;
	if ((r = write(c->socket, (void *) data, size)) <= 0) {
		err_error("cannot write to server: %s", strerror(errno));
		return 0;
	}
	return r;
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
		return 0;
	}
		
	return read(c->socket, data, size);
}

int
cli_recv(pcli_t c, char *data, size_t size) {
	int r;
	if (!data) return 0;
	return read(c->socket, (void *) data, size);
}

