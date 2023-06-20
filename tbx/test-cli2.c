#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include "tstmr.h"
#include "err.h"
#include "cli.h"


void
get(pcli_t c) {
	char b[4096];
	
	tstamp_t	TO;
	TO.tv_sec  = 300;
	TO.tv_usec = 0;

	cli_send(c, "GET", 3);
	if (!cli_recv_to(c, b, 4096, TO)) {
		err_error("cli time out on GET\n");
	} else {
		printf("cli GET got %s\n", b);	
	}
}
	
void
hlp(pcli_t c) {
	char b[4096];

	tstamp_t	TO;
	TO.tv_sec  = 300;
	TO.tv_usec = 0;

	cli_send(c, "HLP", 3);
	if (!cli_recv_to(c, b, 4096, TO)) {
		err_error("cli time out on HLP\n");
	} else {
		printf("cli HLP got %s\n", b);	
	}
}

void
bye(pcli_t c) {
	char b[4096];
	tstamp_t	TO;
	TO.tv_sec  = 300;
	TO.tv_usec = 0;

	cli_send(c, "BYE", 3);
	if (!cli_recv_to(c, b, 4096, TO)) {
		err_error("cli time out on BYE\n");
	} else {
		printf("cli BYE got %s\n", b);	
	}
	cli_destroy(c);
	exit(0);
}

typedef void (*f_t)(pcli_t);
 
f_t  fcn[] = { get, hlp, bye };

int
main(void) {
	pcli_t c;
	char b[101];

	srand(getpid());

	if (!(c = cli_new("localhost", 3303))) {
		err_error("cannot create client\n");
		return 1;
	}

	cli_recv(c, b, 100);

	if (strcmp(b, "connected")) {
		err_error("bad server response (%s) instead of \"connected\"\n", b);
		return 2;
	} else printf("cli srv response \"%s\" on connection\n", b);

	while (1) fcn[rand() % 3](c);

	return 0;
}

