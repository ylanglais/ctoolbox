#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
extern int errno;

#include <tbx/futl.h>
#include <tbx/err.h>

typedef struct {
	char   *name;
	int	    status;
	FILE   *fifo;
} pipe_t, *ppipe_t;

#define _pipe_c_
#include "pipe.h"
#undef  _pipe_c_

static char *_pipe_status_strings[] = {
	"unknown state"
	"cannot create",
	"cannot open",
	"cannot receive",
	"cannot send",
	"ready to receive",
	"ready to send",
	"closed"
};

static char *
_pipe_chomp(char *str) {
	char *p;
	for (p = str; *p && *p != '\n'; p++);
	if (*p == '\n') *p = 0;
	return str;
}

char *pipe_status_string(pipe_status_t pstat) {
	if (pstat < pipeUNKNOWN || pstat > pipeCLOSED) return _pipe_status_strings[pipeUNKNOWN];
	return _pipe_status_strings[pstat];
}

ppipe_t 
pipe_destroy(ppipe_t pi) {
	if (pi) {
		pi->status = pipeCLOSED;
		if (pi->fifo) {
			fclose(pi->fifo);
			pi->fifo = NULL;
			pi->status = pipeCLOSED;
		}
		if (pi->name) {
			futl_rm(pi->name);
			free(pi->name);
			pi->name = NULL;	
		}
		free(pi);
	}
	return NULL;
}

ppipe_t
pipe_create(char *file) {
	ppipe_t pi = NULL;

	if (futl_exists(file)) {
		if (!futl_is_fifo(file)) {
			err_error("file %s exists but is not a FIFO", file);
			return NULL;
		}
	} else { 
		if (mkfifo(file, 0666)) {
			err_error("cannot create fifo : %s", strerror(errno));
			return NULL;
		}
	}
	if (!(pi = (ppipe_t) malloc(sizeof(pipe_t)))) {
		err_error("cannot allocate memory for pipe structure");
		return NULL;	
	}
	if (!(pi->fifo = fopen(file, "r"))) {
		err_error("cannot open fifo for receiving : %s", strerror(errno));
		return pipe_destroy(pi);
	}

	pi->name = strdup(file);

	pi->status = pipeREADY_TO_RECEIVE;
	return pi;
}	

ppipe_t 
pipe_close(ppipe_t pi) {
	if (!pi)       return NULL;
	if (pi->fifo) fclose(pi->fifo);
	pi->status = pipeCLOSED;
	if (pi->name)  {
		free(pi->name);
		pi->name = NULL;
	}
	free(pi);
	return NULL;
}

ppipe_t
pipe_open(char *file) {
	ppipe_t pi;
	if (!(pi = (ppipe_t) malloc(sizeof(pipe_t)))) {
		err_error("cannot allocate memory for pipe structure");
		return NULL;	
	}
	/* Wait for fifo to be created: */
	while (!futl_exists(file)) sleep(1);
	if (!futl_is_fifo(file)) {
		err_error("%s is not a fifo", file);
		return pipe_close(pi);
	}
	if (!(pi->fifo = fopen(file, "a"))) {
		err_error("cannot open fifo for sending : %s", strerror(errno));
		return pipe_close(pi);
	}

	pi->status = pipeREADY_TO_SEND;
	pi->name   = strdup(file);

	return pi;
}

int
pipe_write(ppipe_t pi, char *data) {
	if (!pi) {
		err_error("no pipe");
		return 1;
	}	
	if (!pi->fifo) {
		err_error("no fifo");
		return 2;
	}
	fprintf(pi->fifo, "%s\n", data);
	return 0;
}

char *
pipe_read(ppipe_t pi) {
	char    *p = NULL;
	size_t len = 0;
	if (!pi)          return NULL;
	if (!pi->fifo)    return NULL;
	if (getline(&p, &len, pi->fifo) < 0) {
		if (p) free(p);
		return NULL;
	}
	return _pipe_chomp(p);
}

#ifdef _test_pipe_

#define FIFO "/tmp/pipe_test.fifo"



void writer() {
	char *data[] = {
		"One",
		"Two",
		"Three",
		"Four",
		"Five",
		NULL
	};

	int i;
	ppipe_t pi;

	printf("writer: open fifo\n");
	pi = pipe_open(FIFO);

	printf("writer: fifo opened, send data\n");
	for (i = 0; data[i]; i++) {
		if (pipe_write(pi, data[i])) {
			err_error("could not send '%s'", data[i]);
		} else {
			printf("writer: sent '%s'\n", data[i]);
		}
	}
	printf("everything sent, close fifo\n");
	pipe_close(pi);
}

void listener() {
	char *str;
	ppipe_t pi;
	printf("listener: create fifo\n");
	if (!(pi = pipe_create(FIFO))) {
		err_error("cannot open fifo: %s", FIFO);
		return;
	}
	printf("listener: fifo create, receiving\n");
	while ((str = pipe_read(pi))) {
		printf("listener: received '%s'\n", str);
		free(str);
	}
	printf("listener: nothing left to receive\n");
	pipe_destroy(pi);
}

int
main(void) {
	int p;

	p = fork(); 

    if (p == 0) { 
		writer();
		return 0;
    } else if (p < 0) {
        printf("fork failed (p = %d)\n", p);
        exit(22);
    }
//	sleep(1);
	listener();
	return 0;
}


#endif
