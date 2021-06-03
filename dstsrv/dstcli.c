#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

extern int errno;

#include <tbx/cli.h>
#include <tbx/option.h>
#include <tbx/err.h>
#include <tbx/tstmr.h>

typedef enum {
	dstcliNONE,
	dstcliSRVWAIT,
	dstcliCONNECTED,
	dstcliTASKWAIT,
	dstcliTASKRUN,
	dstcliTASKDONE,
	dstcliTASKFAIL,
	dstcliTASKABORT
} dstcli_state_t;

char *dstcli_state_code[] = {
	"NONE",
	"WAITSRV",
	"CONNECTED",
	"TASKWAIT",
	"TASTKRUN",
	"TASKDONE",
	"TASKFAIL",
	"TASKABORTED"
};

char *dstcli_state_strings[] = {
	"Unkown",
	"Wait server to connect",
	"Connected to server",
	"Wait for a task",
	"Run a task",
	"task done",
	"task failed",
	"task ended but status is uncertain"
};

typedef char srvstring_t[100];
typedef char hostident_t[110];

typedef struct {
	pcli_t			cli;
	int			    taskcount;
	hostident_t		cli_ident;
	srvstring_t	    srvname;
	long	 		srvport;
	dstcli_state_t  state;	
	int             srvcnct_sleep;
	tstamp_t		cnxwait;
	tstamp_t		timeout;
	tstamp_t		pingdelay;
	int				debug;
	int 			lock;
	tstamp_t		lasttalk;
	ptmr_t			timer;
	int				token;
	char 			task_command[1024];
} dstcli_t, *pdstcli_t;

static dstcli_t _dstcli_;

void
dstcli_dump() {
	char b[100];
	err_debug("cli_ident       = %s", _dstcli_.cli_ident);
	err_debug("servername      = %s", _dstcli_.srvname);
	err_debug("serverport      = %d", _dstcli_.srvport);
	err_debug("timeout         = %d", _dstcli_.timeout.tv_sec);
	err_debug("srvcnct_sleep   = %d", _dstcli_.srvcnct_sleep);
	err_debug("taskcount       = %d", _dstcli_.taskcount);
	err_debug("uptime          = %s", tstamp_duration_fmt(b, tmr_elapsed(_dstcli_.timer)));
	err_debug("lasttalk        = %s", tstamp_duration_fmt(b, tstamp_sub(tstamp_get(), _dstcli_.lasttalk)));
	err_debug("pingdelay       = %d", _dstcli_.pingdelay.tv_sec);
	err_debug("state           = %s", dstcli_state_code[_dstcli_.state]);
}

typedef struct {
	int		token;
	int 	code;
	long	elapsed;
} taskret_t;

int
dstcli_ident() {
	srvstring_t host;
 	gethostname(host, sizeof(srvstring_t));
 	sprintf(_dstcli_.cli_ident, "%s.%d", host, getpid());
	return 0;
}

void
dstcli_init() {
	_dstcli_.taskcount       = 0;
	strcpy(_dstcli_.srvname,  "localhost");
	_dstcli_.srvport         = 9911;
	_dstcli_.state           = dstcliNONE;
	_dstcli_.srvcnct_sleep   = 30;
	_dstcli_.cnxwait.tv_sec = 300;
	_dstcli_.timeout.tv_sec  = 30;
	_dstcli_.pingdelay.tv_sec = 30;
	_dstcli_.debug           = 0;
	_dstcli_.lock            = 0;
	_dstcli_.lasttalk        = tstamp_zero();
	_dstcli_.timer           = tmr_new();
	dstcli_ident();
	tmr_start(_dstcli_.timer);
}

void
dstcli_fini() {
	cli_destroy(_dstcli_.cli);
	tmr_stop(_dstcli_.timer);
	dstcli_dump();
	tmr_destroy(_dstcli_.timer);
	exit(0);
}

void
dstcli_lock() {
	while (_dstcli_.lock) sleep(1);
	_dstcli_.lock = 1;
}

void
dstcli_unlock() {
	_dstcli_.lock = 0;
}

char **
dstcli_task_args_prepare() {
	int  n, i;
	char **a, *p, *q, c;
	
	if (!*_dstcli_.task_command) {
		err_error("null task command");
		return NULL;
	}
	
	err_trace("splitting \"%s\"", _dstcli_.task_command);

	/* count args */
	
	q = p = _dstcli_.task_command;
	n = 0;
	while (*p) {
		switch (*p) {
		case '"':
			while (*++p != '"' && *p);
			if (!*p) {
				err_error("unbalanced '\"' in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '\'':
			while (*++p != '\'' && *p);
			if (!*p) {
				err_error("unbalanced \"'\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '`':
			while (*++p != '`' && *p);
			if (!*p) {
				err_error("unbalanced \"`\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case ' ':
		case '\t':
			if (*(p - 1) != '\\') { 
				n++;
				p++;
				while (*p == ' ' || *p == '\t') p++;
				--p;	
			}
			break;
		}
		p++;
		if (!*p && (*(p-1) != ' ' && *(p-1) != '\t') && *(p-2) != '\\') n++;
	}

	err_trace("line contains %d arguments", n);

	if (!(a = (char **) malloc((n + 1) * sizeof(char *)))) {
		err_error("cannot allocate memory for argv array");
		return NULL;
	}	
	
	a[n] = NULL;

	p = q;
	i = 0;
	
	/* split args */
	while (*p) {
		switch (*p) {
		case '"':
			while (*++p != '"' && *p);
			if (!*p) {
				err_error("unbalanced '\"' in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '\'':
			while (*++p != '\'' && *p);
			if (!*p) {
				err_error("unbalanced \"'\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '`':
			while (*++p != '`' && *p);
			if (!*p) {
				err_error("unbalanced \"`\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case ' ':
		case '\t':
			if (*(p - 1) != '\\') {
				c = *p;
				*p = 0;
				a[i++] = strdup(q);
				*p = c;
				p++;
				while (*p == ' ' || *p == '\t') p++;
				q = --p;	
				while (*q == ' ' || *q == '\t') q++; 
			}
		
			break;
		}
		p++;
		if (!*p && (*(p-1) != ' ' && *(p-1) != '\t') && *(p-2) != '\\') a[i++] = strdup(q);
	}

	for (i = 0; i < n; i++) err_trace("a[%d] = %s", i, a[i]);

	return a;
}

static int
_dstcli_server_connect_with_message(char *message) {
	if (!(_dstcli_.cli = cli_new(_dstcli_.srvname, _dstcli_.srvport))) {
		err_error("Cannot connect to sever %s:%d", _dstcli_.srvname, _dstcli_.srvport);
		return 1;
	}

	dstcli_lock();
  	if (cli_send_to(_dstcli_.cli, message, strlen(message) + 1, _dstcli_.timeout) <= 0) {
		err_error("Bad connection");
  		dstcli_unlock();
		return 1;
	}
	_dstcli_.lasttalk = tstamp_get();
  	dstcli_unlock();

	return 0;
}

int
dstcli_server_connect() {
	char b[1000];
	int r;
	ptmr_t timer;
	timer = tmr_new();

	sprintf(b,"CONNECT %s", _dstcli_.cli_ident);

	tmr_start(timer);	
	_dstcli_.state = dstcliSRVWAIT;

	while ((r = _dstcli_server_connect_with_message(b)) && tstamp_cmp(_dstcli_.cnxwait, tmr_elapsed(timer)) > 0)
		sleep(_dstcli_.srvcnct_sleep);

	tmr_destroy(timer);
	if (r) return 1;
	_dstcli_.state = dstcliCONNECTED;
	return 0;
}

int
dstcli_server_reconnect() {
	char b[1000];
	int r;
	ptmr_t timer;
	timer = tmr_new();

	/* force unlock : */
	dstcli_unlock();
	err_debug("try to reconnect to the server...");
	sprintf(b,"RECONNECT %s with status %s on token %d", _dstcli_.cli_ident, dstcli_state_code[_dstcli_.state], _dstcli_.token);
	err_debug("sending: %s", b);

	tmr_start(timer);	

	while ((r = _dstcli_server_connect_with_message(b)) && tstamp_cmp(_dstcli_.cnxwait, tmr_elapsed(timer)) > 0)
		sleep(_dstcli_.srvcnct_sleep);

	tmr_destroy(timer);
	if (r) return 1;
	_dstcli_.state = dstcliCONNECTED;
	return 0;
}

void
dstcli_ping() {
	tstamp_t t;

	err_debug("dstcli ping thread launched with pingdelay = %d", _dstcli_.pingdelay.tv_sec);

	while (1) {
		sleep(_dstcli_.pingdelay.tv_sec / 4);
		dstcli_lock();
		t = tstamp_get();	
		t = tstamp_sub(t, _dstcli_.lasttalk); 	
		if (t.tv_sec > _dstcli_.pingdelay.tv_sec) {
			char msg[4096];

			err_debug("PINGing server");
			sprintf(msg, "PING with status %s", dstcli_state_code[_dstcli_.state]);

			/* gestion des déconnexions */
			while (cli_send_to(_dstcli_.cli, msg, strlen(msg) + 1, _dstcli_.timeout) <= 0) {
				err_warning("client %s cannot send to server, try reconnection is needed...", _dstcli_.cli_ident);
				dstcli_unlock();
				if ((dstcli_server_reconnect())) return;
				dstcli_lock();
			}
			err_debug("PING sent");

			*msg = 0;
			if (cli_recv_to(_dstcli_.cli, msg, 5, _dstcli_.timeout) <= 0) {
				err_warning("client %s has no reply from the server, reconnection is needed...", _dstcli_.cli_ident);
				dstcli_unlock();
				if ((dstcli_server_reconnect())) return;
				dstcli_lock();
			} else {
				err_debug("PONG recieved");
				_dstcli_.lasttalk = tstamp_get();
			}
		}
		dstcli_unlock();
	}
}

taskret_t
dstcli_task_run() {
	int			code;
	char 		b[100];
	pid_t 		pid;
	ptmr_t		timer;
	taskret_t	taskret;
	tstamp_t	ts;

	char 	  **argv;

	taskret.token = _dstcli_.token;

	if (!(argv = dstcli_task_args_prepare())) {
		err_error("unable to create command line");
		taskret.code = 255;
		taskret.elapsed = 0;
		return taskret;	
	} 

	_dstcli_.taskcount++;
	_dstcli_.state = dstcliTASKRUN;
	timer = tmr_new();
	err_debug("running \"%s\"", _dstcli_.task_command);
	tmr_start(timer);
	pid = fork(); 
	if (pid < 0) {
		/* fork error: */
		err_error("cannot fork a new process");
		tmr_destroy(timer);
	} else if (!pid) {
		/* child part */
		if (execvp(argv[0], argv)) {
			err_error("******** child execution faild : %s", strerror(errno));
			exit(1);
		}
	} else {
		int childpid;
		pthread_attr_t attr;
		pthread_t tid;

		/* Mise en place des ping dans un second thread !!! */
		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, (void *(*)(void *)) dstcli_ping, NULL);

		sleep(1);
		/* parent part : wait for child to terminate */
		if ((childpid = waitpid(pid, &code, 0)) == pid) {
			if (code != 0) 	
				_dstcli_.state = dstcliTASKFAIL;
			else 
				_dstcli_.state = dstcliTASKDONE;

		} else _dstcli_.state = dstcliTASKABORT;

		pthread_cancel(tid);

		err_debug("waiting %d returned %d errcode = %d", pid, childpid, code); 
	}
	tmr_stop(timer);
	err_info("task ran in %s (pid = %d)", tstamp_duration_fmt(b, tmr_elapsed(timer)), getpid());

	dstcli_dump();

	ts = tmr_elapsed(timer);
	taskret.elapsed = ts.tv_sec;
	taskret.code = code;
	tmr_destroy(timer);
	return taskret;	
}

int
dstcli_task_wait() {
	char b[4100];
	size_t s;

	*b = 0;

	dstcli_lock();
	_dstcli_.state = dstcliTASKWAIT;
	while (strncmp(b, "PLEASE RUN", 10) && strncmp(b, "DONE", 4)) {
		if ((s = cli_recv_to(_dstcli_.cli, b, 4100, _dstcli_.timeout)) <= 0) {
			err_warning("%s has not recieve task after %d seconds", _dstcli_.cli_ident, _dstcli_.timeout.tv_sec); 
			return 1;
		}
	}
	err_debug("%s received %d bytes (\"%s\")", _dstcli_.cli_ident, s, b);
	if (!strncmp(b, "DONE", 4)) {
		_dstcli_.lasttalk = tstamp_get();
		dstcli_unlock();
		return 55;			
	}
	_dstcli_.lasttalk = tstamp_get();
	dstcli_unlock();

	sscanf(b, "PLEASE RUN token %d: %[^\n]", &_dstcli_.token, _dstcli_.task_command);  

	err_debug("token = %d, task_command = \"%s\"", _dstcli_.token, _dstcli_.task_command);

	return 0;
}

int
dstcli_task_report(taskret_t tr) {
	char b[1000];
	sprintf(b, "REPORT token %d returned %d after %ld seconds", tr.token, tr.code, tr.elapsed);
	err_debug("sending \"%s\"", b);
	dstcli_lock();
	if (cli_send_to(_dstcli_.cli, b, strlen(b) + 1, _dstcli_.timeout) <= 0) {
		err_error("Problem while sending report to the server");
		return 1;
	}
	err_debug("successfully sent \"%s\"", b);
	dstcli_unlock();

/* evt TODO: émission des logs et fichiers de résultat ? */

	_dstcli_.lasttalk = tstamp_get();
	return 0;
}

int
main(int n, char *a[]) {
	int i, r, done;
	popt_t opt;
	char *conffile;
	dstcli_init();
	atexit(dstcli_fini);

	done = 0;

	/* Parse command line : */
	for (i = 1; i < n; i++) {
		if (!strcmp(a[i], "-c")) {
			conffile = a[++i];			
		} else {
			err_error("unknown arguement %s", a[i]);
		}
	}
	
	if (!conffile) {
		err_error("conffile must be specified on command line (%s -c conffile", a[0]);
		return 1;
	}

	/* TODO: fichier de log dstcli */
	/* TODO: gestion des pertes de connexion/reconnexions !!! */
	/* TODO: gestion des reconnexions plantage du dstsrv => reconnect et fournit le résultat... */
	/* TODO: polling de l'état d'avancement du dstcli...: */

	if (!(opt = opt_new("[ \t]*([A-z0-9_]+)[ \t]*=[ \t]*(.*)[ \t]*\n"))) {
		err_error("cannot create option");	
		return 2;
	}

	dstcli_ident();
	opt_entry_add(opt, "address",         "%s", sizeof(srvstring_t),  &_dstcli_.srvname,         NULL);
	opt_entry_add(opt, "port",            "%d", sizeof(int),          &_dstcli_.srvport,         NULL);
	opt_entry_add(opt, "debug",           "%d", sizeof(int),          &_dstcli_.debug,           NULL);
	opt_entry_add(opt, "srvcnct_sleep",   "%d", sizeof(int),          &_dstcli_.srvcnct_sleep,   NULL);
	opt_entry_add(opt, "timeout",         "%ld", sizeof(int),         &_dstcli_.timeout.tv_sec,  NULL);
	opt_entry_add(opt, "cnxwait",         "%ld", sizeof(long),        &_dstcli_.cnxwait.tv_sec,    NULL);
	opt_entry_add(opt, "srv_ping_delay",  "%d", sizeof(int),          &_dstcli_.pingdelay.tv_sec,       NULL);
	opt_parse(opt, conffile, &_dstcli_);
	opt_destroy(opt);

	if (_dstcli_.debug) err_level_set(err_DEBUG);

	dstcli_dump();

	while (dstcli_server_connect()) {
		sleep(_dstcli_.srvcnct_sleep);
	}

	/* evt TODO: Timeout sur lock */

	while (!done) {
		taskret_t tr;
		while ((r = dstcli_task_wait())) {
			if (r == 55) { 
				done = 1;
				break;
			}
			if (dstcli_server_reconnect()) {
				err_error("cannot reconnect to the server, client exiting");
				return 3;
			}
		}

		tr = dstcli_task_run();
		
		while (!done && dstcli_task_report(tr)) {
			if (dstcli_server_reconnect()) {
				err_error("cannot reconnect to the server, client exiting");
				return 4;
			}
		}
	}
	return 0;
}

