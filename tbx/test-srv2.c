

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "tstmr.h"
#include "err.h"
#include "srv.h"

void 
dispatch(psrv_t s, int fd, char *m) {
	char b[4096];
	
	if (!strncmp(m, "GET", 3)) {
		sprintf(b, "<html>\n<head>\n<title>Page vide</title>\n</head>\n<body></body>\n</html>\n");
		srv_send(s, b, strlen(b) + 1);
	} else if (!strncmp(m, "HLP", 3)) {
		sprintf(b, "GET => chargement d'une page\nHLP => this message\nBYE => fermer la connexion");
		srv_send(s, b, strlen(b) + 1);
	} else if (!strncmp(m, "BYE", 3)) {
		sprintf(b, "BYE-ACK\n");
		srv_send(s, b, strlen(b) + 1);
		srv_cli_disconnect(s);
		exit(0);
	} else {
		sprintf(b, "Bad request");
		srv_send(s, b, strlen(b) + 1);
	}
}

void 
server(psrv_t s, int fd) {
	char b[101];

	tstamp_t	TO;
	TO.tv_sec  = 300;
	TO.tv_usec = 0;

	printf("created srv %d\n", fd);

	sprintf(b, "connected");
	srv_send(s, b, strlen(b) + 1);
	
	while (1) {
		if (srv_recv_to(s, b, 100, TO)) {
			dispatch(s, fd, b);
		} else {
			printf("srv #%d : no client request after %d sec\n", fd, TO.tv_sec);
		}
	}
}

int
main(void) {
	psrv_t s;
	pid_t p;

	err_init(NULL, err_TRACE);

	/* signal(SIGCHLD, sigchild_catch); */
	signal(SIGCHLD, SIG_IGN);
	
	if (!(s = srv_new("127.0.0.1", 3303, 20))) return 1;

	
	while (1) {
		int fd;

		if ((fd = srv_cli_connect(s)) > 0) {
			p = fork();
			if (p == 0) {
				server(s, fd);
			} else if (p < 0) {
				err_error("fork failed (p = %d)\n", p);
			} else {
				srv_cli_disconnect(s);
			}
		}
	}
	return 0;
}
