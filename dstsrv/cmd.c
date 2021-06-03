#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
 
#include <tbx/err.h>
#include <tbx/shmem.h>

#include "cmd.h"

extern int errno;

typedef enum {
	cmdWAIT,
	cmdWAITRUN,
	cmdRUNNING,
	cmdSUCCESS,
	cmdFAIL
} cmdstatus_t;

static char *cmdstatus_str[] = {
	"WAIT",
	"WAITRUN",
	"RUNNING",
	"SUCCESS",
	"FAIL"
}; 
	
#define cmdLEN 512

typedef struct {
	int			token;
	hostident_t	hostident;
	cmdstatus_t state;
	int			runcount;
	char 		cmd[cmdLEN];
	int			returned;
	tstamp_t 	lasttalk;
	tstamp_t	started;
	tstamp_t	ended;
	tstamp_t 	elapsed;
	tstamp_t 	system;
	tstamp_t 	user;
} cmd_t, *pcmd_t;

typedef struct {
	pid_t 	pid;
	int		lock;
	int 	ncmds;
	int		completed;
	cmd_t 	cmd[];
} data_t, *pdata_t;

size_t
cmd_header_sizeof() {
	return sizeof(data_t);
}

size_t
cmd_sizeof() {
	return sizeof(cmd_t);
}

size_t
cmd_sizeof_n(int numcmds) {
	return sizeof(data_t) + numcmds * sizeof(cmd_t);
}

void
cmd_lock(void *p) {
	pdata_t d;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	d = (pdata_t) p;
	if (!d) return;
	while (d->lock);
	d->lock = 1;
}

void 
cmd_unlock(void *p) {
	pdata_t d;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	d = (pdata_t) p;
	if (!d) return;
	d->lock = 0;
}

void
cmd_data_init(void *p) {
	pdata_t d;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	d = (pdata_t) p;
	d->pid   = getpid();	
	d->lock  = 0;
	d->ncmds = 0;	
	d->completed = 0;	
}

pid_t
cmd_pid_get(void *p) {
	pdata_t d;
	if (!p) {
		err_error("null cmd pointer");
		return 0;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	cmd_unlock(p);
	return d->pid;
}

void
cmd_pid_update(void *p) {
	pdata_t d;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	d->pid = getpid();
	cmd_unlock(p);
}

int 
cmd_number(void *p) {
	pdata_t d;
	int n;

	if (!p) {
		err_error("null cmd pointer");
		return -1;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	n = d->ncmds;
	cmd_unlock(p);
	return n;
}

int
cmd_processed(void *p) {
	int n;
	pdata_t d;
	if (!p) {
		err_error("null cmd pointer");
		return -1;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	n = d->completed;
	cmd_unlock(p);
	return n;
}

int
cmd_remaining(void *p) {
	int n;
	pdata_t d;
	if (!p) {
		err_error("null cmd pointer");
		return -1;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	n =  d->ncmds - d->completed;
	cmd_unlock(p);
	return n;
}

void 
cmd_add(void *p, long duration, char *cmd) {
	pdata_t d;
	pcmd_t  c;

	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);

	d = (pdata_t) p;
	c = &(d->cmd[d->ncmds]);

	c->state    = cmdWAIT;
	c->runcount = 0;
	c->token    = d->ncmds;
	c->returned = 0;
	c->lasttalk = c->elapsed = c->system = c->user = tstamp_zero();
	c->elapsed.tv_sec = duration;
	snprintf(c->cmd, cmdLEN, "%s", cmd); 
	d->ncmds++;
	cmd_unlock(p);	
}

char *
cmd_next(void *p, int *token, int maxrun) {
	int i;
	pdata_t d;
	pcmd_t  c;
	if (!p) {
		err_error("null cmd pointer");
		return NULL;
	}

	cmd_dump_all(p);
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;

	*token = -1;
	for (i = 0; (c->state != cmdWAIT && (c->state != cmdFAIL || c->runcount < maxrun)) && i < d->ncmds; c++, i++)
		err_debug("token %i c->state = %s\tc->runcount = %d", i, cmdstatus_str[c->state], c->runcount);

	err_debug("i = %d", i);
	
	if (i < d->ncmds) {
		c->state = cmdWAITRUN;
		c->lasttalk = tstamp_get();
		*token = i;
		cmd_unlock(p);
		return c->cmd;
	}
	cmd_unlock(p);
	return NULL; 
}

void
cmd_start(void *p, int token, hostident_t host, tstamp_t at) {
	pdata_t d;
	pcmd_t  c;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;
	if (token < 0 || token >= d->ncmds) {
		cmd_unlock(p);
		err_error("bad token");
		return;
	}
	c += token;

	strcpy(c->hostident, host);
	c->started = at;
	c->runcount++;
	c->state = cmdRUNNING;
	c->lasttalk = tstamp_get();
	cmd_unlock(p);
}

void
cmd_resume(void *p, int token) {
	pdata_t d;
	pcmd_t  c;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;
	if (token < 0 || token >= d->ncmds) {
		cmd_unlock(p);
		err_error("bad token");
		return;
	}
	c += token;
	c->lasttalk = tstamp_get();
	cmd_unlock(p);
	err_debug("token %d resumed", token);
}

void 
cmd_respool(void *p, int token) {
	pdata_t d;
	pcmd_t  c;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;
	
	if (token < 0 || token >= d->ncmds) {
		cmd_unlock(p);
		err_error("bad token");
		return;
	}
	c += token;

	c->started = tstamp_zero();
	c->state   = cmdWAIT;
	c->lasttalk = tstamp_zero();
	c->hostident[0] = 0;

	cmd_unlock(p);
}

void
cmd_end(void *p, int token, int r, tstamp_t ended, long elapsed) {
	pdata_t d;
	pcmd_t  c;

	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;

	if (token < 0 || token >= d->ncmds) {
		cmd_unlock(p);
		err_error("bad token");
		return;
	}
	c += token;
	if (!(c->returned = r)) {
		c->state = cmdSUCCESS;
		c->elapsed.tv_sec = elapsed;
		c->lasttalk = tstamp_zero();
		d->completed++;
	} else c->state = cmdFAIL;

	c->ended = ended;	

	cmd_unlock(p);
}

void
cmd_client_died(void *p, char *hostident) {
	pdata_t d;
	pcmd_t  c;
	int i;

	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;

	for (i = 0; i < d->ncmds; i++) {
		if (!strcmp(c[i].hostident, hostident) && (c[i].state == cmdWAITRUN || c[i].state == cmdRUNNING)) {
			err_warning("client %s has been reported dead, respooling task %d", hostident, i);
			cmd_unlock(p);
			cmd_respool(p, i);
			return;
		}
	}
	cmd_unlock(p);
}

int 
cmd_running_check(void *p, tstamp_t pingdelay) {
	pdata_t d;
	pcmd_t  c;
	tstamp_t ts;
	int i;

	err_debug("checking running tasks...");
	
	if (!cmd_remaining(p)) return -1;

	if (!p) {
		err_error("null cmd pointer");
		return -2;
	}
	cmd_lock(p);
	ts = tstamp_get();
	d = (pdata_t) p;
	c = d->cmd;

	for (i = 0; i < d->ncmds; i++) {	
		if (c[i].state == cmdRUNNING || c[i].state == cmdWAITRUN) {
			if (tstamp_cmp(pingdelay, tstamp_sub(ts, c[i].lasttalk)) > 0) {
				/* client is supposedly broken or dead: */
				err_warning("client %s has been reported dead, respooling task %d", c[i].hostident, i);
				cmd_unlock(p);
				cmd_respool(p, i);
				return 0;
			}	
		}
	}
	cmd_unlock(p);
	return 1;
}

void 
cmd_dump_all(void *p) {
	char b[100];
	int	 i;
	pdata_t d;
	pcmd_t  c;
	if (!p) {
		err_error("null cmd pointer");
		return;
	}

	d = (pdata_t) p;
	c = d->cmd;
	cmd_lock(p);
	err_debug("dstsrv shared mem data:");
	err_debug("number of commands: %d", d->ncmds);
	for (i = 0; i < d->ncmds; i++) {
		err_debug("cmd[%d]:", i);
		err_debug("\tstate    = %s", cmdstatus_str[c[i].state]);
		err_debug("\thost     = %s", c[i].hostident);
		err_debug("\truncount = %d", c[i].runcount);
		err_debug("\treturned = %d", c[i].returned);
		err_debug("\tcommand  = %s", c[i].cmd);
		err_debug("\tstarted  = %s", tstamp_fmt(b, c[i].started));
		err_debug("\tended    = %s", tstamp_fmt(b, c[i].ended));
		err_debug("\telapsed  = %s", tstamp_duration_fmt(b, c[i].elapsed));
		err_debug("\tsystem   = %s", tstamp_duration_fmt(b, c[i].system ));
		err_debug("\tuser     = %s", tstamp_duration_fmt(b, c[i].user   ));
	}
	cmd_unlock(p);
}

void
cmd_report_write(void *p, char *filename) {
	FILE *f;
	pdata_t d;
	pcmd_t  c;
	int i;
	char b1[100], b2[100], b3[100];

	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;

	if (!(f = fopen(filename, "wb"))) {
		err_error("cannot open file \"%s\", writing to stdout", filename);
		f = stdout;
	}

	err_debug("report file:");
	for (i = 0; i < d->ncmds; i++) {
		fprintf(f, "%s;%s;%d;%d;%s;%s;%s;%s\n",
			cmdstatus_str[c[i].state],
			c[i].hostident,
			c[i].runcount,
			c[i].returned,
			tstamp_fmt(b1, c[i].started),
			tstamp_fmt(b2, c[i].ended),
			tstamp_duration_fmt(b3, c[i].elapsed),
			c[i].cmd);
	}
	if (f != stdout) fclose(f);
	cmd_unlock(p);
}

void
cmd_spec_write(void *p, char *filename) {
	FILE *f;
	pdata_t d;
	pcmd_t  c;
	int i;

	if (!p) {
		err_error("null cmd pointer");
		return;
	}
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;

	if (!(f = fopen(filename, "wb"))) {
		err_error("cannot open file \"%s\", writing to stdout", filename);
		f = stdout;
	}

	err_debug("new spec file:");
	for (i = 0; i < d->ncmds; i++) {
		fprintf(f, "%ld:%s\n", c[i].elapsed.tv_sec, c[i].cmd);
	}
	if (f != stdout) fclose(f);
	cmd_unlock(p);
}

int
cmd_error(void *p, char *filename) {
	FILE *f;
	pdata_t d;
	pcmd_t  c;
	int i, n;

	if (!p) {
		err_error("null cmd pointer");
		return -1;
	}
	n = 0;
	cmd_lock(p);
	d = (pdata_t) p;
	c = d->cmd;

	if (!(f = fopen(filename, "wb"))) {
		err_error("cannot open file \"%s\", writing to stdout", filename);
		f = stdout;
	}

	err_debug("error file:");
	for (i = 0; i < d->ncmds; i++) {
		if (c[i].state != cmdSUCCESS) {
			n++;
			fprintf(f, "%ld:%s\n", c[i].elapsed.tv_sec, c[i].cmd);
		}
	}
	if (f != stdout) fclose(f);
	cmd_unlock(p);
	return n;
}

typedef struct {
	long duration;
	char cmd[cmdLEN];
} spec_t, *pspec_t;

/* descending order comparison function: */
int 
spec_cmp(const void *p1, const void *p2) {
	pspec_t s1, s2;

	s1 = (pspec_t) p1;
	s2 = (pspec_t) p2;
	
	if (s1->duration > s2->duration) return -1;
	if (s1->duration < s2->duration) return 1;
	return 0;
}

/* read spec file: */
int
cmd_spec_read(void *p, char *filename) {
	int n = 0, i, r;
	FILE *f;
	spec_t spec[500];

	if (!p) {
		err_error("null cmd pointer");
		return 1;
	}
	memset((char *) spec, 0, 500 * sizeof(spec_t));

	err_debug("opening spec file");
	if (!(f = fopen(filename, "r"))) {
		err_error("cannot open spec file \"%s\"", filename);
		return 2;
	}
	
	while ((r = fscanf(f, "%ld:%[^\n]\n", &spec[n].duration, spec[n].cmd)) > 0) {
		err_debug("line %d : %ld:%s", n, spec[n].duration, spec[n].cmd);
		n++;
	}
	err_debug("fscanf read %d values", r);

	fclose(f);	

	/* sort spec descending according to duration: */
	qsort((void *) spec, n, sizeof(spec_t), spec_cmp);

	/* store spec in shared mem: */
	for (i = 0; i < n; i++) {
		err_debug("spec[%d].duration = %ld",    i, spec[i].duration);
		err_debug("spec[%d].cmd      = \"%s\"", i, spec[i].cmd);
		cmd_add(p, spec[i].duration, spec[i].cmd);
	}
	return 0;
}

void *
cmd_shmem_open(int key) {
	return shmem_attach(key);
}

void *
cmd_shmem_new(int key, size_t size) {
	void *p;

	if (!(p = shmem_new(key, size, 0))) {
		err_error("cannot create shared memory segment : %s", strerror(errno));	
		return NULL;
	}
	return p;
}

void *
cmd_shmem_destroy(void *p) {
	return shmem_destroy(p);
}

void *
cmd_init(int key, size_t size, char *filename) {
	void *p;

	/* prepare shared mem: */
	if ((p = cmd_shmem_open(key))) {

		/* Attention: vérifier que nous ne lancons pas en double !!!! */
		if (kill(cmd_pid_get(p), 0) < 0) {
			/* server died update its pid: */
			cmd_pid_update(p);
		} else {
			/* server is not dead, duplicate run !!! */
			err_error("DUPLICATE RUN DETECTED with process %d ==> ABORTING", cmd_pid_get(p));
			return NULL;
		}
	} else if (!(p = cmd_shmem_new(key, size))) {
		return NULL;
	} else {
		cmd_data_init(p);
		cmd_spec_read(p, filename);
	}
	cmd_dump_all(p); 
	return p;
}

void *
cmd_force_init(int key, size_t size, char *filename) {
	void *p;
	if ((p = cmd_shmem_open(key))) cmd_shmem_destroy(p);
	return cmd_init(key, size, filename);
}

void *
cmd_exit(void *p) {
	return cmd_shmem_destroy(p);
}
