#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <string.h>


extern int errno;

#include "err.h"
#include "srv.h"
#include "cli.h"

#define PORT 33033

int client() {
	pcli_t c;
	char   b[101];

	err_debug("pass1\n");

	sleep(1);

	err_debug("pass2\n");

	/*if (!(c = cli_new(cli_host_by_name("localhost"), PORT))) {*/
	if (!(c = cli_new("127.0.0.1", PORT))) {
		err_error(">>> cannot create client, exiting\n");
		exit(66);
	}

	err_debug("pass3\n");
	cli_dump(c);
	err_debug("pass4\n");
	
	cli_send(c, "bonjour", 8);
	cli_recv(c, b, 100);
	printf("cli received: %s\n", b);	
	c = cli_destroy(c);
	return 0;
}

	pid_t p;

void
spool_client() {
	p = fork();

	if (p == 0) {
		client();
	} else if (p < 0) {
		err_error("fork failed (p = %d)\n", p, strerror(errno));
		exit(22);
	}
}

int server() {
	psrv_t s;
	char   b[101];
	int    cid;

	if (!(s = srv_new("*", PORT, 2)))  exit(1);

	printf("server created\n");
	spool_client();	

	
	printf("client spooled\n");
	
	if ((cid = srv_cli_connect(s)) < 0) {
		err_error("cannot connect client\n");
		exit(2);
	}

	printf("client connected\n");
	srv_recv(s, b, 100);

	printf("srv received : %s\n", b);


	srv_cli_disconnect(s);
	printf("srv client disconnected\n");
	s = srv_destroy(s);
	printf("srv client destroyed\n");
	return 0;
}

int 
main(void) {
	err_level_set(err_TRACE);
	return server();
}

