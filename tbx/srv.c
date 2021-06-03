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
	30/05/2016  2.1  add tbx std tags
	04/08/2016  2.2  add changelog
	11/10/2017	3.0	 add ssl support
	16/10/2017	3.1	 add bind address & store client addr to server struct:
*/   

char srv_MODULE[]  = "Server";
char srv_PURPOSE[] = "Client/Server Server network socket encapsulation";
char srv_VERSION[] = "3.1";
char srv_DATEVER[] = "16/10/2017";

extern int errno;

#define srv_MAX_ADDR_LEN	50

typedef struct {
	int		 cid;
	char	 cliaddr[srv_MAX_ADDR_LEN + 1];	
	int		 status;
	int	     socket;
	int      port;
	int      lock;
	int      maxclients;
	int      nclients;
	int		 must_stop;

	/** ssl part: **/
	pssl_t			  ssl;
	ssl_method_t	  method;
	char *			  cert;
	char *			  key;
	long			  ssl_opts;
	long			  chk_opts;
	int 			  chk_depth;
} srv_t, *psrv_t;

#define _srv_c_
#include "srv.h"
#undef _srv_c_

void
srv_lock(psrv_t s) {
	if (!s) return;
	while (s->lock); 
	s->lock = 1;
}

void 
srv_unlock(psrv_t s) {
	if (!s) return;
	if (s->lock) s->lock = 0;
}

psrv_t 
srv_destroy(psrv_t s) {
	if (s) {
		s->status = srvUNKNOWN;
		close(s->socket);
		free(s);
	}
	return NULL;
}

size_t srv_sizeof() { return sizeof(srv_t); }

psrv_t
srv_new(char *address, int port, int maxclients) {
	static long on = 1L;
 	struct sockaddr_in sin;             
	psrv_t s;

	if (!(s = (psrv_t) malloc(sizeof(srv_t)))) {
		err_error("cannot allocate memory for srv"); 
		return NULL;
	}

	memset((void *) s, 0, sizeof(srv_t));

	s->maxclients = maxclients;
	s->port       = port;
	s->status = srvCREATING;	

	if ((s->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		err_error("cannot create socket : %s", strerror(errno));
		return srv_destroy(s);
	}

	s->must_stop = 0;
	s->status = srvCONFIGURING;

 	setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(int));
 	setsockopt(s->socket, SOL_SOCKET, SO_KEEPALIVE, (void *) &on, sizeof(int));

	s->status = srvBINDING;

	if (!address || !address[0] || address[0] == '*') {
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

	if (bind(s->socket, (struct sockaddr *) &sin, sizeof(sin))) {
		err_error("cannot bind socket to port : %s", strerror(errno));
		return srv_destroy(s);
	}
		
	s->status = srvCONNECTING;

	if (listen(s->socket, maxclients - 1)) {
		err_error("cannot listen on port : %s", strerror(errno));
		return srv_destroy(s);
	}

	s->status = srvLISTENING;
	return s;
}

psrv_t
srv_ssl_new(char *address, int port, int maxclients, 
	ssl_method_t method, char *certificate, char *key, 
	long options, long verify_options, int verify_depth) {

	psrv_t s;
	pssl_t ssl;

	if (!(ssl = ssl_new(method, certificate, key, options, verify_options, verify_depth))) 
		return NULL;

	if (!(s = srv_new(address, port, maxclients))) {
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

int
srv_port(psrv_t s) {
	if (!s) return 0;
	return s->port;
}

int srv_socket(psrv_t s) {
	if (!s) return 0;
	return s->socket;
}

int srv_maxclients(psrv_t s) {
	if (!s) return 0;
	return s->maxclients;
}

int srv_status(psrv_t s) {
	if (!s) return 0;
	return s->status;
}

int
srv_clients_connected(psrv_t s) {
	if (!s) return -1;
	return s->nclients;
}

void 
srv_stop(psrv_t s) {
	if (s) s->must_stop = 1;
}

char *
srv_cli_connected_host(psrv_t s) {
	if (!s || !*s->cliaddr) return NULL;
	return strdup(s->cliaddr);
}

void
srv_wait_children(psrv_t s) {
	int ret;
	pid_t pid;
	err_debug("srv %d is waiting for children to terminate", (int) getpid());
	while (!s->must_stop) {
		pid = wait(&ret); 
		if (pid < 0) {
			/* TODO : add wait_before_retry */
			sleep(1);
			continue;
		}	
		err_debug("srv %d has been notified that %d exited with status %d", getpid(), pid, ret);
		srv_cli_disconnect(s);
		if (s->nclients == 0) {
			/* cannot respond: */
			err_debug("srv %d cannot connect any more client, wait & retry", getpid());
		}
	}
	pthread_exit(NULL);
}

int
srv_clean_connection(psrv_t srv) {
	if (!srv) return 1;
	if (srv->ssl) ssl_clean_connection(srv->ssl);	
	if (srv->cid) {
		close(srv->cid);
		srv->cid = 0;
	}
	return 0;
}

int
srv_cli_disconnect(psrv_t s) {
	if (!s) return -1;
	srv_lock(s);	
	s->nclients--;
	srv_unlock(s);
	return 0;
}

static int 
_srv_accept(psrv_t srv, struct sockaddr *cli_addr, socklen_t *len) {
	int fd;
	errno = 0;
	if ((fd = accept(srv->socket, cli_addr, len)) < 0) {
		err_error("cannot accept connections : %s", strerror(errno));
		return fd;
	}
	if (srv->ssl) {
		if (ssl_accept(srv->ssl, fd) < 0) {
			return -1;
		}
	}
	srv->cid = fd;
	return fd;
}

int
srv_cli_connect(psrv_t s) {
	int fd;
	struct sockaddr	    cli_addr;
	struct sockaddr_in  cli_in;
	socklen_t           len;
	struct hostent     *cli;
	srv_lock(s);

	len = sizeof(struct sockaddr);

	if (s->nclients >= s->maxclients) {
		err_error("Too many clients connected (%d, max %d), no more client allowed", s->nclients, s->maxclients);
		srv_unlock(s);
		sleep(1);
		return -1;
	}

	errno = 0;
	if ((fd = _srv_accept(s, &cli_addr, &len)) < 0) {
		return -2;
	}
	cli_in = *(struct sockaddr_in *) &cli_addr;
	if (!(cli = gethostbyaddr(&cli_in.sin_addr, 4, AF_INET))) {
		err_debug("cannot resolve client connection");
	} else err_debug("srv %d accecped a connection from %s", getpid(), cli->h_name);

	snprintf(s->cliaddr, srv_MAX_ADDR_LEN, "%s", cli->h_name);
	s->nclients++;

	srv_unlock(s);
	return fd;	
}

int
srv_cli_connect_to(psrv_t s, tstamp_t timeout) {
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

	srv_lock(s);

	FD_ZERO(&fdread);
	FD_SET(s->socket, &fdread);

	err_debug("srv %d is waiting for socket %d to be sollicited", getpid(), s->socket);

	if ((r = select(s->socket + 1, &fdread, NULL, NULL, &timeout)) > 0) {
		err_debug("select returned %d", r);
		errno = 0;

		if ((fd = _srv_accept(s, &cli_addr, &len)) <= 0) {
			srv_unlock(s);
			return -2;
		}
		cli_in = *(struct sockaddr_in *) &cli_addr;

		if (!(cli = gethostbyaddr(&cli_in.sin_addr, 4, AF_INET))) {
			err_debug("cannot resolve client connection");
		} else err_debug("srv %d accecped a connection from %s", getpid(), cli->h_name);

		s->nclients++;
	} else {
		err_warning("select returned %d as errcore: %s", r, strerror(errno));
		srv_unlock(s);
		return 0;	
	}
	srv_unlock(s);
	return fd;	
}

#if 0
void
srv_reap_cli() {
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		err_error("sigaction error: %s", strerr(errno));
		exit(1);
	}
}
#endif

int
srv_cli_connect_loop(psrv_t s, srv_cli_dialog_f srv_cli_dialog_cb, srv_must_stop_f srv_must_stop_cb, void *data) {
	int fd;
	pid_t pid;
	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	
	/* spool disconnection management: */
	pthread_create(&tid, &attr, (void *(*)(void *)) srv_wait_children, (void *) s); 

	if (srv_must_stop_cb) s->must_stop = srv_must_stop_cb(data); 

	/* atexit(srv_reap_cli); */
	while (!s->must_stop) {
		if ((fd = srv_cli_connect(s)) > 0) {
			err_debug("srv %d is forking after client connection", getpid());
			if ((pid = fork()) == 0) {
				int r;
				close(s->socket);
				s->socket = 0;
				/* set in srv_accept(); */
				// s->cid    = fd; 
				r = srv_cli_dialog_cb(s, s->nclients, fd, data);
				if (s) srv_destroy(s);
				exit(r);
			} else if (pid < 0) {
				err_error("srv %d cannot for srv process : %s", getpid(), strerror(errno));
			} else {
				err_debug("srv %d is closing file descriptor #%d", getpid(), fd);
				srv_clean_connection(s);
			}
		} 
		if (srv_must_stop_cb) s->must_stop = srv_must_stop_cb(data); 
	}
	return 0;
}

int
srv_cli_connect_loop_to(psrv_t s, srv_cli_dialog_f srv_cli_dialog_cb, srv_must_stop_f srv_must_stop_cb, void *data, tstamp_t timeout) {
	int fd;
	pid_t pid;
	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	
	/* spool disconnection management: */
	pthread_create(&tid, &attr, (void *(*)(void *)) srv_wait_children, (void *) s); 

	if (srv_must_stop_cb) s->must_stop = srv_must_stop_cb(data); 

	/* atexit(srv_reap_cli); */
	while (!s->must_stop) {
		if ((fd = srv_cli_connect_to(s, timeout)) > 0) {
			err_debug("srv %d is forking after client connection", getpid());
			if ((pid = fork()) == 0) {
				int r;
				err_debug("forked close socket");
				close(s->socket);
				r = srv_cli_dialog_cb(s, s->nclients, fd, data);
				if (s) srv_destroy(s);
				exit(r);
			} else if (pid < 0) {
				err_error("srv %d cannot for srv process : %s", getpid(), strerror(errno));
			} else {
				err_debug("srv %d is closing file descriptor #%d", getpid(), fd);
				srv_clean_connection(s);
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
		if (srv_must_stop_cb) s->must_stop = srv_must_stop_cb(data); 
	}
	return 0;
}
static int
_srv_send(psrv_t s, char *data, size_t size) {
	if (!s) return 0;
	if (!s->ssl) return send(s->cid, (void *) data, size, 0);
	else         return ssl_write(s->ssl, data, size);
}

static int
_srv_recv(psrv_t s, char *data, size_t size) {
	if (!s)      return 0;
	if (!s->ssl) return recv(s->cid, (void *) data, size, 0);
	else         return ssl_read(s->ssl, data, size);
}

/************************************************
 *
 *	Functions called by srv children: 
 * 
 ************************************************/

int
srv_disconnect_client(psrv_t s) {
	if (!s) return 1;
	close(s->cid);
	srv_destroy(s);
	return 0;
}

int
srv_recv_to(psrv_t s, char *data, size_t size, tstamp_t timeout) {
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
		
	return _srv_recv(s, data, size);
}

int
srv_recv(psrv_t s, char *data, size_t size) {
	if (!data) return 0;
	return _srv_recv(s, (void *) data, size);
}

int
srv_send_to(psrv_t s, char *data, size_t size, tstamp_t timeout) {
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
		
	return _srv_send(s, (void *) data, size);
}

int
srv_send(psrv_t s, char *data, size_t size) {
	return _srv_send(s, (void *) data, size);
}

char *
srv_cipher(psrv_t s) {
	if (!s || !s->ssl) return NULL;
	return ssl_cipher_get(s->ssl);
}

char *
srv_certificate(psrv_t s) {
	if (!s || !s->ssl) return NULL;
	return ssl_certificate_get(s->ssl);
}

