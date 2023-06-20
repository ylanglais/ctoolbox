
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "err.h"
#include "cli.h"

#define PORT 13303

int main(void) {
	pcli_t c;
	int i, r;
	char   b[101];

//	err_init("cli.log", err_DEBUG);
 	err_level_set(err_DEBUG);

	sleep(1);

	for (i = 0; i < 3; i++) {
		c = NULL;
		//sleep(5);
		if (!fork()) {
			if (!(c = cli_new("localhost", PORT))) {
				err_error("cli %d.%d cannot create client, exiting", i, getpid());
				exit(66);
			}
			err_debug("created client %d (pid = %d)", i, getpid());
			
			if ((r = cli_send(c, "bonjour", 8)) <= 0) 
				err_error("cli %d.%d failed to send data to server", i, getpid());
			else 
				err_debug("cli %d.%d sent %d bytes", i, getpid(), r);

			if ((r = cli_recv(c, b, 100)) <= 0) 
				err_error("cli %d.%d failed to read data from server", i, getpid());
			else
				err_debug("cli %d.%d received %d bytes : \"%s\"", i, getpid(), r, b);

			c = cli_destroy(c);
			exit(0);
		}	
	}
	return 0;
}

