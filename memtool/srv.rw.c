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
#include <err.h>

extern int errno;

typedef struct {
	int		 status;
	int	     socket;
	int      port;
	int      lock;
	int      maxclients;
	int      nclients;
	int		 must_stop;
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
srv_new(int port, int maxclients) {
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

    bzero(&sin, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(s->port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY); 
    /* sin.sin_addr.s_addr = INADDR_ANY; */

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
srv_clis_connected(psrv_t s) {
	if (!s) return -1;
	return s->nclients;
}

void 
srv_stop(psrv_t s) {
	if (s) s->must_stop = 1;
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
			/* Server must exit: */
			err_debug("srv %d has no more clients connected and is now quitting", getpid());
			s->must_stop = 1;
		}
	}
}

int
srv_cli_disconnect(psrv_t s) {
	if (!s) return -1;
	srv_lock(s);	
	s->nclients--;
	srv_unlock(s);
	return 0;
}

int
srv_cli_connect(psrv_t s) {
	int fd;
	struct sockaddr	cli_addr;
	struct sockaddr_in cli_in;
	socklen_t len;
	struct hostent *cli;

	srv_lock(s);

	if (s->nclients >= s->maxclients) {
		err_error("Too many clients connected (%d, max %d), no more client allowed", s->nclients, s->maxclients);
		sleep(1);
		return -1;
	}

	errno = 0;
	if ((fd = accept(s->socket, &cli_addr, &len)) < 0) {
		err_error("cannot accept connections : %s", strerror(errno));
		return -2;
	}
	cli_in = *(struct sockaddr_in *) &cli_addr;
	cli = gethostbyaddr(&cli_in.sin_addr, 4, AF_INET);
	err_debug("srv %d accecped a connection from %s", getpid(), cli->h_name);
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

		if ((fd = accept(s->socket, &cli_addr, &len)) <= 0) {
			err_error("cannot accept connections : %s", strerror(errno));
			srv_unlock(s);
			return -2;
		}
		cli_in = *(struct sockaddr_in *) &cli_addr;
		cli = gethostbyaddr(&cli_in.sin_addr, 4, AF_INET);
		err_debug("srv %d accecped a connection from %s", getpid(), cli->h_name);
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
srv_cli_connect_loop(psrv_t s, srv_cli_dialog_f srv_cli_dialog_cb, srv_must_stop_f srv_must_stop_cb, void *data, tstamp_t timeout) {
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
				//close(s->socket);
				r = srv_cli_dialog_cb(s->nclients, fd, data);
				exit(r);
			} else if (pid < 0) {
				err_error("srv %d cannot for srv process : %s", getpid(), strerror(errno));
			} else {
				err_debug("srv %d is closing file descriptor #%d", getpid(), fd);
				close(fd);
			}
		} else if (fd <= 0) {
			char b[100];
			if (s->nclients == 0) {
				err_error("Still no connection after %s", tstamp_duration_fmt(b, timeout));
				return 1; 
			} else if (fd == -1) {
				/* TODO : add wait_before_retry */
				err_error("select return -1 after %s", tstamp_duration_fmt(b, timeout));
				sleep(1);
			}
		}
		if (srv_must_stop_cb) s->must_stop = srv_must_stop_cb(data); 
	}
	return 0;
}

/* Functions called by srv children: */

int
srv_disconnect_client(int cid) {
	close(cid);
	return 0;
}

int
srv_recv_to(int cid, char *data, size_t size, tstamp_t timeout) {
	int r;
	fd_set fdread;

	FD_ZERO(&fdread);
	FD_SET(cid, &fdread);
	
	r = select(cid + 1, &fdread, (fd_set *) 0, (fd_set *) 0, (struct timeval *) &timeout);
		
	if (r == 0) {
		err_error("timeout occured aftrer %d.%d seconds", timeout.tv_sec, timeout.tv_usec);
		return 0;
	}
	if (r < 0) {
		err_error("select error : %s", strerror(errno));
		return 0;
	}
		
	return read(cid, data, size);
}

int
srv_recv(int cid, char *data, size_t size) {
	if (!data) return 0;
	return read(cid, (void *) data, size);
}

int
srv_send_to(int cid, char *data, size_t size, tstamp_t timeout) {
	int r;
	fd_set fdwrite;

	FD_ZERO(&fdwrite);
	FD_SET(cid, &fdwrite);
	
	r = select(cid + 1, (fd_set *) 0,  &fdwrite, (fd_set *) 0, (struct timeval *) &timeout);
	if (r < 0) {
		err_error("select error : %s", strerror(errno));
		return 0;
	} else if (r == 0) {
		err_error("timeout occured aftrer %d.%d seconds", timeout.tv_sec, timeout.tv_usec);
		return 0;
	}
		
	return write(cid, (void *) data, size);
}

int
srv_send(int cid, char *data, size_t size) {
	return write(cid, (void *) data, size);
}

