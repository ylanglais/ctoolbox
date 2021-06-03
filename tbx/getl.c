/*
	This code is released under GPL terms. 
	Please read license terms and conditions at http://www.gnu.org/

	Yann LANGLAIS

	Changelog:
	12/06/2013	1.0 	initial release
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "srv.h"
#include "cli.h"

char getl_MODULE[]  = "Bufferd fd getline for unix IO (man 2 read), srv and cli components";
char getl_PURPOSE[] = "Bufferd fd getline for unix IO (man 2 read), srv and cli components";
char getl_VERSION[] = "1.0.0";
char getl_DATEVER[] = "12/06/2013";

typedef enum {
	getl_UIO,
	getl_SRV,
	getl_CLI
} getl_type_e;

typedef union {
	int   fd;
	int   cid;
	void *srv;
	void *cli;
} getl_src_u;

typedef struct {
	getl_type_e  type;
	getl_src_u   src;
	char        *sbuf;
	size_t       sblen;
} getl_t, *pgetl_t;

#define _getl_c_
#include "getl.h" 
#undef  _getl_c_

size_t
getl_sizeof() {
	return sizeof(getl_t);
}

pgetl_t 
getl_destroy(pgetl_t g) {
	if (g) {
		if (g->sbuf) free(g->sbuf);
		free(g);
	}
	return NULL;
}

pgetl_t
getl_new(getl_type_e type, getl_src_u src, size_t max) {
	pgetl_t g;

	if (type < getl_UIO || type > getl_CLI)   return NULL;
	if (!(g = (pgetl_t) malloc(sizeof(getl_t)))) return NULL;
	g->type = type;
	switch (type) {
	case getl_UIO:
		g->src.fd  = src.fd;
		break;
	case getl_SRV:
		g->src.srv = src.srv;
		break;
	case getl_CLI:
		g->src.cli = src.cli;
		break;
	default:
		break;
	}
	g->sbuf  = NULL;
	g->sblen = max;
	return g;
}

int 
getl_moredata(pgetl_t g) {
	if (!g) return -1;
	if      (g->type == getl_UIO) 
		return     read(g->src.fd,  g->sbuf, g->sblen - 1);
	else if (g->type == getl_SRV) 
		return srv_recv(g->src.srv, g->sbuf, g->sblen - 1);
	else if (g->type == getl_CLI) 
		return cli_recv(g->src.cli, g->sbuf, g->sblen - 1);
	return -2;
}

size_t
gelt_buffer_len(pgetl_t g) {
	if (!g || !g->sbuf) return 0;
	return strlen(g->sbuf);
}

char *
getl_buffer_get(pgetl_t g) {
	char *p;
	if (!g || !g->sbuf) return NULL;
	p = strdup(g->sbuf); 
	g->sbuf[0] = 0;	
	return p;	
}

int 
getl_line(pgetl_t g, char *str, size_t max) {
	char *p, *q, *r;
	int i;
	
	if (!g) return -1;

	memset(str, 0, max);
	p = str;

	do {

		/* check if current buffer has data: */
		if (!g->sbuf || !*g->sbuf) {

			/* current buffer is empty: */
			if (!g->sbuf) {

				/* allocate buff and keep track of initial buffer size */
				if (g->sblen == 0) g->sblen = max;
				g->sbuf = malloc(g->sblen);
			}	

			memset(g->sbuf, 0, g->sblen);

			/* fill buffer with socket data: */
			if (getl_moredata(g) <= 0)
				/* socket closed / no more data: */
				return 0;
		}

		/* 
         * Copy content of internal buffer to output buffer until:
		 * - max - 1 chars copied to output,
         * - end (i.e. \0) of internal buffer reached,
         * - a \n occurs.
		 */

		for (q = g->sbuf, i = 0; i < max - 1 && *q && *q != '\n'; i++) 
			*p++ = *q++;

		/* add the \n: */
		if (*q == '\n') {
			*p++ = *q++;
	 		i++;
		}

		/* decrement max: */
		max -= i;
		
		/* check if we still have chars in internal buffer: */
		if (*q == 0) {

			/* all chars consumed => truncate internal buffer: */
			*g->sbuf = 0;
		} else {

			/* move data from p to g->sbuf from p to \0: */
			r = g->sbuf; 
			while (*q) 
				*r++ = *q++;
			*r = 0;
		}

	    /* loop if neither max nor \n has been reached: */ 
	} while (max && p[-1] != '\n');

	/* need to terminate output string: */	
	return p - str;
}

#ifdef _test_getl_
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;

#include "err.h"

#define PORT 33033

int
srv() {
	int i = 1;
	psrv_t s;
	char   b[101];
	pgetl_t g;
	getl_src_u src;

	if (!(s = srv_new(NULL, PORT, 1)))  return 1;
	if ((src.cid = srv_cli_connect(s)) < 0) {
		err_error("cannot connect client\n");
		return 2;
	}
	src.srv = s;
	printf("client connected\n");
	
	if (!(g = getl_new(getl_SRV, src, 30)))        return 3;
	
	while (getl_line(g, b, 100)) {
		printf("SRV -> %3d : %s", i++, b);
	}
	srv_cli_disconnect(s);
	printf("srv client disconnected\n");
	s = srv_destroy(s);
	printf("srv client destroyed\n");
	return 0;
}

int
main(void) {
	int  i = 1;
	char b[101];
	pgetl_t guio, gcli;
	getl_src_u src1, src2;

	pid_t p;
	p = fork();
	if (p == 0) return srv();
	if (p  < 0) return 1;
	
	// Let time to server to get ready:
	sleep(5);

	if ((src1.fd = open("rgb.txt", O_RDONLY)) <= 0) {
		err_error("cannot open file, reason: %s", strerror(errno));		
		return 2;
	}
	if (!(guio = getl_new(getl_UIO, src1, 30)))   return 3;
	if (!(src2.cli = cli_new("127.0.0.1", PORT))) return 4;
	if (!(gcli = getl_new(getl_CLI, src2, 50)))   return 5;
	
	
	while (getl_line(guio, b, 100)) {
		printf("UIO -> %3d : %s", i++, b);
		cli_send(src2.cli, b, strlen(b));
	}
	i = 1;
	close(src1.fd);
	guio = getl_destroy(guio);
	gcli = getl_destroy(gcli);
	cli_destroy(src2.cli);

	return 0;
} 
#endif
