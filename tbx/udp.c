#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <errno.h>

#include "err.h"
#include "ssl.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	16/01/2018  1.0  Initial release
*/   

char udp_MODULE[]  = "udp C/S";
char udp_PURPOSE[] = "UDP Client/Server encapsulation";
char udp_VERSION[] = "1.0";
char udp_DATEVER[] = "16/01/2019";

extern int errno;

#define udp_MAX_ADDR_LEN	50

typedef struct {
	int		 cid;
	char	 cliaddr[udp_MAX_ADDR_LEN + 1];	
	size_t 	 addrlen;
	int		 status;
	int	     socket;
	int      port;
	size_t	 maxsize;

#if 0
	int      lock;

	int      maxclients;
	int      nclients;
	int		 must_stop;
#endif
#if 0
	/** ssl part: **/
	pssl_t			  ssl;
	ssl_method_t	  method;
	char *			  cert;
	char *			  key;
	long			  ssl_opts;
	long			  chk_opts;
	int 			  chk_depth;
#endif
} udp_t, *pudp_t;

#define _udp_c_
//#include "udp.h"
#undef _udp_c_
#if 0
void
udp_lock(pudp_t s) {
	if (!s) return;
	while (s->lock); 
	s->lock = 1;
}

void 
udp_unlock(pudp_t s) {
	if (!s) return;
	if (s->lock) s->lock = 0;
}
#endif
pudp_t 
udp_destroy(pudp_t s) {
	if (s) {
		//s->status = udpUNKNOWN;
		close(s->socket);
		free(s);
	}
	return NULL;
}

size_t udp_sizeof() { return sizeof(udp_t); }

pudp_t
udp_new(size_t maxsize, char *address, int port, int issrv) {
	static long on = 1L;
 	struct sockaddr_in sin;             
	pudp_t s;

	if (!(s = (pudp_t) malloc(sizeof(udp_t)))) {
		err_error("cannot allocate memory for udp"); 
		return NULL;
	}

	memset((void *) s, 0, sizeof(udp_t));

	s->maxsize = maxsize;
	s->port    = port;
	//s->status     = udpCREATING;	

	if ((s->socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		err_error("cannot create socket : %s", strerror(errno));
		return udp_destroy(s);
	}

	//s->must_stop = 0;
	//s->status = udpCONFIGURING;

 	setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(int));
 	setsockopt(s->socket, SOL_SOCKET, SO_KEEPALIVE, (void *) &on, sizeof(int));

	//s->status = udpBINDING;

	if (!address || !address[0] || address[0] == '*') {
		if (!issrv) {
			err_error("client has no server address");
			return udp_destroy(s);
		}
		bzero(&sin, sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(s->port);
		sin.sin_addr.s_addr = htonl(INADDR_ANY); 
	} else {
		struct addrinfo hints;
		char sport[15];
		int r;
		memset(&hints, 0, sizeof(hints));
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_family = AF_INET;
		struct addrinfo * ai = NULL;
		sprintf(sport, "%d", port);
		if ((r = getaddrinfo(address, sport, &hints, &ai))) {
			err_error("Cannot get host/port (%s/%d) information: %s\n", address, port, gai_strerror(r));
			return NULL;	
		}
		sin = * (struct sockaddr_in *) ai->ai_addr;
		freeaddrinfo(ai);
	}

	if (issrv > 0) {
		if (bind(s->socket, (struct sockaddr *) &sin, sizeof(sin))) {
			err_error("cannot bind socket to port : %s", strerror(errno));
			return udp_destroy(s);
		}
	} else {
		memcpy(s->cliaddr, (char *) &sin, sizeof(struct sockaddr_in));
	}
	return s;
}
int
udp_send(pudp_t s, char *data, size_t size) {
	size_t len;
	len = sizeof(struct sockaddr);
	if (!s) return -55;
	return sendto(s->socket, (void *) data, size, 0, (struct sockaddr *) &(s->cliaddr), (socklen_t) len);
}

char *
udp_recv(pudp_t s, size_t *size) {
	char *data;
	size_t len;
	len = sizeof(struct sockaddr);
	if (!s) {
		err_error("no udp data");
		*size = 0;
		return NULL;
	}
	if (!(data = malloc(s->maxsize))) {
		err_error("cannot allocate %ul bytes", s->maxsize);
		*size = 0;
		return NULL;
	}
	*size = /*(ssize_t)*/ recvfrom(s->socket, (void *) data, s->maxsize, 0, (struct sockaddr *) &(s->cliaddr), (socklen_t *) &len);
	err_error("recsize = %ul", *size);	
	return data;
}

#ifdef _test_udp_
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
extern int errno;
#define UPORT	40404

int main(void) {
	int p;
	p = fork();
	if (p < 0) {
		err_error("cannot fork");
		exit(1);
	} else if (p == 0) {
		/* client: */
		pudp_t u;
		if (!(u = udp_new(1020, "localhost", UPORT, 0))) {	
			err_error("client: cannot create connection");
			exit(1);
		}
		printf("client created\n");
		//sleep(2);
		if (udp_send(u, "client: test",	18) < 0) {
			err_error("client: cannot send (%s)", strerror(errno));
			u = udp_destroy(u);
			exit(2);
		}
		char *d;
		size_t s;
		if (!(d = udp_recv(u, &s))) {
			err_error("client: cannot recv");
			u = udp_destroy(u);
			exit(3);
		}
		printf("client: received \"%s\"\n", d);
		if (d) free(d); 
		d = NULL;
		u = udp_destroy(u);
	} else {
		/* Server: */
		pudp_t u;
		if (!(u = udp_new(1020, NULL, UPORT, 1))) {	
			err_error("server: cannot create connection");
			u = udp_destroy(u);
			exit(1);
		}
		printf("server created\n");
		char *d;
		size_t s;
		if (!(d = udp_recv(u, &s))) {
			err_error("server: cannot receive data");
			u = udp_destroy(u);
			exit(2);
		}
		printf("server: received \"%s\"\n", d);
		if (d) free(d); 
		d = NULL;
		if (udp_send(u, "server: test",	18) < 0) {
			err_error("server: cannot send");
			u = udp_destroy(u);
			exit(3);
		}
		u = udp_destroy(u);
	}
	exit(0);
}

#endif


#if 0
pudp_t
udp_ssl_new(char *address, int port, int maxclients, 
	ssl_method_t method, char *certificate, char *key, 
	long options, long verify_options, int verify_depth) {

	pudp_t s;
	pssl_t ssl;

	if (!(ssl = ssl_new(method, certificate, key, options, verify_options, verify_depth))) 
		return NULL;

	if (!(s = udp_new(address, port, maxclients))) {
		ssl_destroy(ssl);
		return NULL;
	}
	s->ssl       = ssl;
	s->method    = method;
	s->cert      = strdup(certificate);
	s->key       = strdup(key);
	s->ssl_opts  = options;
	s->chk_opts  = verify_options;
	s->chk_depth = verify_depth;

	return s;
}
#endif
#if 0
int
udp_port(pudp_t s) {
	if (!s) return 0;
	return s->port;
}

int udp_socket(pudp_t s) {
	if (!s) return 0;
	return s->socket;
}

int udp_maxclients(pudp_t s) {
	if (!s) return 0;
	return s->maxclients;
}

int udp_status(pudp_t s) {
	if (!s) return 0;
	return s->status;
}

int
udp_clients_connected(pudp_t s) {
	if (!s) return -1;
	return s->nclients;
}

void 
udp_stop(pudp_t s) {
	if (s) s->must_stop = 1;
}

char *
udp_cli_connected_host(pudp_t s) {
	if (!s || !*s->cliaddr) return NULL;
	return strdup(s->cliaddr);
}

void
udp_wait_children(pudp_t s) {
	int ret;
	pid_t pid;
	err_debug("udp %d is waiting for children to terminate", (int) getpid());
	while (!s->must_stop) {
		pid = wait(&ret); 
		if (pid < 0) {
			/* TODO : add wait_before_retry */
			sleep(1);
			continue;
		}	
		err_debug("udp %d has been notified that %d exited with status %d", getpid(), pid, ret);
		udp_cli_disconnect(s);
		if (s->nclients == 0) {
			/* cannot respond: */
			err_debug("udp %d cannot connect any more client, wait & retry", getpid());
		}
	}
	pthread_exit(NULL);
}

int
udp_clean_connection(pudp_t udp) {
	if (!udp) return 1;
	if (udp->ssl) ssl_clean_connection(udp->ssl);	
	if (udp->cid) {
		close(udp->cid);
		udp->cid = 0;
	}
	return 0;
}

int
udp_cli_disconnect(pudp_t s) {
	if (!s) return -1;
	udp_lock(s);	
	s->nclients--;
	udp_unlock(s);
	return 0;
}

static int 
_udp_accept(pudp_t udp, struct sockaddr *cli_addr, socklen_t *len) {
	int fd;
	errno = 0;
	if ((fd = accept(udp->socket, cli_addr, len)) < 0) {
		err_error("cannot accept connections : %s", strerror(errno));
		return fd;
	}
	if (udp->ssl) {
		if (ssl_accept(udp->ssl, fd) < 0) {
			return -1;
		}
	}
	udp->cid = fd;
	return fd;
}

int
udp_cli_connect(pudp_t s) {
	int fd;
	struct sockaddr	    cli_addr;
	struct sockaddr_in  cli_in;
	socklen_t           len;
	struct hostent     *cli;
	udp_lock(s);

	len = sizeof(struct sockaddr);

	if (s->nclients >= s->maxclients) {
		err_error("Too many clients connected (%d, max %d), no more client allowed", s->nclients, s->maxclients);
		udp_unlock(s);
		sleep(1);
		return -1;
	}

	errno = 0;
	if ((fd = _udp_accept(s, &cli_addr, &len)) < 0) {
		return -2;
	}
	cli_in = *(struct sockaddr_in *) &cli_addr;
	if (!(cli = gethostbyaddr(&cli_in.sin_addr, 4, AF_INET))) {
		err_debug("cannot resolve client connection");
	} else err_debug("udp %d accecped a connection from %s", getpid(), cli->h_name);

	snprintf(s->cliaddr, udp_MAX_ADDR_LEN, "%s", cli->h_name);
	s->nclients++;

	udp_unlock(s);
	return fd;	
}

int
udp_cli_connect_to(pudp_t s, tstamp_t timeout) {
	int fd = 0, r;
	struct sockaddr	cli_addr;
	struct sockaddr_in cli_in;
	socklen_t len;
	fd_set fdread;
	struct hostent *cli;

	len = sizeof(struct sockaddr);

	if (s->nclients >= s->maxclients) {
		err_error("Too many clients connected (%d, max %d), no more client allowed", s->nclients, s->maxclients);
		return -1;
	}

	udp_lock(s);

	FD_ZERO(&fdread);
	FD_SET(s->socket, &fdread);

	err_debug("udp %d is waiting for socket %d to be sollicited", getpid(), s->socket);

	if ((r = select(s->socket + 1, &fdread, NULL, NULL, &timeout)) > 0) {
		err_debug("select returned %d", r);
		errno = 0;

		if ((fd = _udp_accept(s, &cli_addr, &len)) <= 0) {
			udp_unlock(s);
			return -2;
		}
		cli_in = *(struct sockaddr_in *) &cli_addr;

		if (!(cli = gethostbyaddr(&cli_in.sin_addr, 4, AF_INET))) {
			err_debug("cannot resolve client connection");
		} else err_debug("udp %d accecped a connection from %s", getpid(), cli->h_name);

		s->nclients++;
	} else {
		err_warning("select returned %d as errcore: %s", r, strerror(errno));
		udp_unlock(s);
		return 0;	
	}
	udp_unlock(s);
	return fd;	
}
#endif 

#if 0
void
udp_reap_cli() {
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		err_error("sigaction error: %s", strerror(errno));
		exit(1);
	}
}

int
udp_cli_connect_loop(pudp_t s, udp_cli_dialog_f udp_cli_dialog_cb, udp_must_stop_f udp_must_stop_cb, void *data) {
	int fd;
	pid_t pid;
	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	
	/* spool disconnection management: */
	pthread_create(&tid, &attr, (void *(*)(void *)) udp_wait_children, (void *) s); 

	if (udp_must_stop_cb) s->must_stop = udp_must_stop_cb(data); 

	/* atexit(udp_reap_cli); */
	while (!s->must_stop) {
		if ((fd = udp_cli_connect(s)) > 0) {
			err_debug("udp %d is forking after client connection", getpid());
			if ((pid = fork()) == 0) {
				int r;
				close(s->socket);
				s->socket = 0;
				/* set in udp_accept(); */
				// s->cid    = fd; 
				r = udp_cli_dialog_cb(s, s->nclients, fd, data);
				if (s) udp_destroy(s);
				exit(r);
			} else if (pid < 0) {
				err_error("udp %d cannot for udp process : %s", getpid(), strerror(errno));
			} else {
				err_debug("udp %d is closing file descriptor #%d", getpid(), fd);
				udp_clean_connection(s);
			}
		} 
		if (udp_must_stop_cb) s->must_stop = udp_must_stop_cb(data); 
	}
	return 0;
}

int
udp_cli_connect_loop_to(pudp_t s, udp_cli_dialog_f udp_cli_dialog_cb, udp_must_stop_f udp_must_stop_cb, void *data, tstamp_t timeout) {
	int fd;
	pid_t pid;
	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	
	/* spool disconnection management: */
	pthread_create(&tid, &attr, (void *(*)(void *)) udp_wait_children, (void *) s); 

	if (udp_must_stop_cb) s->must_stop = udp_must_stop_cb(data); 

	/* atexit(udp_reap_cli); */
	while (!s->must_stop) {
		if ((fd = udp_cli_connect_to(s, timeout)) > 0) {
			err_debug("udp %d is forking after client connection", getpid());
			if ((pid = fork()) == 0) {
				int r;
				err_debug("forked close socket");
				close(s->socket);
				r = udp_cli_dialog_cb(s, s->nclients, fd, data);
				if (s) udp_destroy(s);
				exit(r);
			} else if (pid < 0) {
				err_error("udp %d cannot for udp process : %s", getpid(), strerror(errno));
			} else {
				err_debug("udp %d is closing file descriptor #%d", getpid(), fd);
				udp_clean_connection(s);
			}
		} else if (fd <= 0) {
			char b[100];
			if (s->nclients == 0) {
				err_error("Still no connection after %s", tstamp_duration_fmt(b, timeout));
				return 1; 
			} else {
				/* TODO : add wait_before_retry */
				err_error("select return -1 after %s", tstamp_duration_fmt(b, timeout));
				sleep(1);
			}
		}
		if (udp_must_stop_cb) s->must_stop = udp_must_stop_cb(data); 
	}
	return 0;
}
static int
_udp_send(pudp_t s, char *data, size_t size) {
	if (!s) return 0;
	if (!s->ssl) return send(s->cid, (void *) data, size, 0);
	else         return ssl_write(s->ssl, data, size);
}

static int
_udp_recv(pudp_t s, char *data, size_t size) {
	if (!s)      return 0;
	if (!s->ssl) return recv(s->cid, (void *) data, size, 0);
	else         return ssl_read(s->ssl, data, size);
}

#endif
/************************************************
 *
 *	Functions called by udp children: 
 * 
 ************************************************/
#if  0
int
udp_disconnect_client(pudp_t s) {
	if (!s) return 1;
	close(s->cid);
	udp_destroy(s);
	return 0;
}

int
udp_recv_to(pudp_t s, char *data, size_t size, tstamp_t timeout) {
	int r;
	fd_set fdread;
	if (!s) return 1;

	FD_ZERO(&fdread);
	FD_SET(s->cid, &fdread);
	
	r = select(s->cid + 1, &fdread, (fd_set *) 0, (fd_set *) 0, (struct timeval *) &timeout);
		
	if (r == 0) {
		err_error("timeout occured after %d.%d seconds", timeout.tv_sec, timeout.tv_usec);
		return 0;
	}
	if (r < 0) {
		err_error("select error : %s", strerror(errno));
		return 0;
	}
		
	return _udp_recv(s, data, size);
}

int
udp_recv(pudp_t s, char *data, size_t size) {
	if (!data) return 0;
	return _udp_recv(s, (void *) data, size);
}

int
udp_send_to(pudp_t s, char *data, size_t size, tstamp_t timeout) {
	int r;
	fd_set fdwrite;

	FD_ZERO(&fdwrite);
	FD_SET(s->cid, &fdwrite);
	
	r = select(s->cid + 1, (fd_set *) 0,  &fdwrite, (fd_set *) 0, (struct timeval *) &timeout);
	if (r < 0) {
		err_error("select error : %s", strerror(errno));
		return 0;
	} else if (r == 0) {
		err_error("timeout occured aftrer %d.%d seconds", timeout.tv_sec, timeout.tv_usec);
		return 0;
	}
		
	return _udp_send(s, (void *) data, size);
}

int
udp_send(pudp_t s, char *data, size_t size) {
	return _udp_send(s, (void *) data, size);
}

char *
udp_cipher(pudp_t s) {
	if (!s || !s->ssl) return NULL;
	return ssl_cipher_get(s->ssl);
}

char *
udp_certificate(pudp_t s) {
	if (!s || !s->ssl) return NULL;
	return ssl_certificate_get(s->ssl);
}

#endif
