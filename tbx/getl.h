#ifndef _getl_h_
#define _getl_h_

#ifndef _getl_c_
typedef enum {
	getl_UIO,
	getl_SRV,
	getl_CLI
} getl_type_e;

typedef union {
	int   fd;
	int   cid;
	void *cli;
} getl_src_u;

typedef void *pgetl_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif


size_t  getl_sizeof();

pgetl_t getl_new(getl_type_e type, getl_src_u src, size_t max);
pgetl_t getl_destroy(pgetl_t g); 

/* get line from buffer: */
int     getl_line(pgetl_t g, char *str, size_t max);

/* get size of buffered data: */
size_t  gelt_buffer_len(pgetl_t g); 

/* get a copy of bufferd data and flush buffer */

#ifdef __cplusplus
}
#endif
#if 0
Usage sample: 

cat >test-getl.c <<<EOF
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <tbx/getl.h>

int
main(void) {
	int  i = 1;
	char b[101];
	pgetl_t guio;
	getl_src_u src;

	/* open "rgb.txt" file: */
	if ((src.fd = open("rgb.txt", O_RDONLY)) <= 0) {
		fprintf(stderr, "cannot open file, reason: %s\n", strerror(errno));		
		return 1;
	}

	/* create a getl object for Unix IO (UIO), the src data and buffer size of 500 bytes/chars: */
	if (!(guio = getl_new(getl_UIO, src, 500))) return 3;
	
	/* Loop on new lines: */
	while (getl_line(guio, b, 100)) {
		printf("%3d : %s", i++, b);
	}

	/* close file: */
	close(src.fd);

	/*destroy getl object: */
	guio = getl_destroy(guio);

	return 0;
} 
EOF
gcc -g getl.c -o getl -I. -L. -ltbx

This also works with the srv & cli components (see srv.h & cli.h).

WARNING:
This module is BUFFERING lower level incoming data. 
Do not use with lower level functions such as read(2), srv_recv[_to] and 
cli_recv[_to] UNLESS :
- you extract remaining data from the buffer with you *REALLY* *REALLY* know what you do. 
- you get data from the buffer before
Input data is at high risk of corruption when mixing reading functions !!!


#endif

#endif
