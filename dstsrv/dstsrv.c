
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
 
#include <tbx/err.h>
#include <tbx/srv.h>
#include <tbx/option.h>

#include "cmd.h"

typedef char srvstring_t[100];

typedef struct { 
	int	  cid;
	pid_t pid;
	hostident_t  ident;
} client_t, *pclient_t; 

typedef struct {
	psrv_t		srv;
	int		    maxclient;
	int 		maxrerun;
	long 	    port;
	int         debug;
	int 	    shkey;
	int			taskcount;
	tstamp_t	pingdelay;
	tstamp_t	lasttalk;
	tstamp_t	timeout;
	tstamp_t    cnxwait;
	ptmr_t		timer;
	size_t		cmd_size;
	char 		*cmd;
	char		errfile[256];
	char		newspec[256];
	char		report[256];
} dstsrv_t, *pdstsrv_t;

static dstsrv_t _dstsrv_;

typedef struct {
	char		*data;
	int			line;
} *ppayload_t;

void
dstsrv_init() {
	_dstsrv_.maxclient  = 50;
	_dstsrv_.maxrerun   = 1;
	_dstsrv_.port       = 9911;
	_dstsrv_.debug 	    = 0;
	_dstsrv_.shkey      = 9911;
	_dstsrv_.pingdelay  = tstamp_zero();
	_dstsrv_.pingdelay.tv_sec = 30;
	_dstsrv_.taskcount  = 0;
	_dstsrv_.cmd_size   = cmd_sizeof_n(200); 
	_dstsrv_.lasttalk   = tstamp_zero();
	_dstsrv_.timeout    = tstamp_zero();
	_dstsrv_.timeout.tv_sec = 30;
	_dstsrv_.timeout    = tstamp_zero();
	_dstsrv_.cnxwait.tv_sec = 300;
	_dstsrv_.timer      = tmr_new();
	tmr_start(_dstsrv_.timer);
}

void 
dstsrv_exit() {
}

void 
dstsrv_dump() {
	char b[100];
	err_debug("maxclient       = %d",  _dstsrv_.maxclient);
	err_debug("port            = %d",  _dstsrv_.port);
	err_debug("debug           = %d",  _dstsrv_.debug);
	err_debug("shkey           = %d",  _dstsrv_.shkey);
	err_debug("taskcout        = %d",  _dstsrv_.taskcount);
	err_debug("maxrerun        = %d",  _dstsrv_.maxrerun);
	err_debug("uptime          = %s",  tstamp_duration_fmt(b, tmr_elapsed(_dstsrv_.timer)));
	err_debug("lasttalk        = %s",  tstamp_duration_fmt(b, tstamp_sub(tstamp_get(), _dstsrv_.lasttalk)));
	err_debug("pingdelay       = %d",  _dstsrv_.pingdelay.tv_sec);
	err_debug("timeout         = %d", _dstsrv_.timeout.tv_sec);
}

int
dstsrv_must_stop(void *data) {
	if (!cmd_remaining(_dstsrv_.cmd)) return 1;
	return 0;
}

/* Lecture du ficher de paramétrage (i.e. liste des commandes avec leurs temps de calcul) */

int
dstsrv_srv_cli_dialog(psrv_t s, int srvnum, int cid, void *data) {
	//ppayload_t pl;
	char msg[4096];
	char clientid[sizeof(srvstring_t) + 10];
	enum {LISTEN, TALK};
	int state = LISTEN;

	//pl = (ppayload_t) data;

	/* first get the connection string: */ 
	err_debug("srv %d waiting for client connection string", srvnum);		
	if (srv_recv_to(s, msg, 1046, _dstsrv_.timeout) <= 0) {
		err_warning("srv %d fell in timout while listening to client connection string", srvnum);
		return 1;
	}
	err_debug("srv %d recieved: \"%s", srvnum, msg);
	if (!strncmp(msg, "CONNECT", 7)) {	
		/* this is a new connection: */
		sscanf(msg, "CONNECT %s", clientid);
		err_debug("srv %d: client %s connected and willing to process", srvnum, clientid);
		state = TALK;
	} else if (!strncmp(msg, "RECONNECT", 9)) {
		char status[50];
		int  token;
		/* client reconnects: */
		sscanf(msg, "RECONNECT %s with status %s on token %d", clientid, status, &token);
		err_debug("srv %d: client %s reconnected and has some result to transmit for token %d", srvnum, clientid, token);
		cmd_resume(_dstsrv_.cmd, token);
		
		state = LISTEN;
	} else {
		/* client talks bad!: */
		err_error("srv %d: clients talks unfriendly dialect (said \"%s\")", msg);
		err_error("must not talk to unknown blokes => disconnect");
		return 2;
	}	

	while (1) {
		if (state == LISTEN) {
			msg[0] = 0;
			/* wait for cli data: */
			if (srv_recv(s, msg, 4096) < 0) {
				err_warning("srv %d : client %s appears to be dead");
				cmd_client_died(_dstsrv_.cmd, clientid);
			}
			err_debug("srv %d: received \"%s\"", srvnum, msg);
			
			/* client may only ping or report result: */
			if (!strncmp(msg, "PING", 4)) {
				char clistate[60];
				sscanf(msg, "PING with status %s", clistate); 
				err_debug("srv %d: client %s is pinging with status %s", srvnum, clientid, clistate);

				if (srv_send_to(s, "PONG", 5, _dstsrv_.timeout) <= 0) {
					err_warning("srv %d: lost connection with client %s",   srvnum, clientid)	;
					return 3;
				}
					
				err_debug("srv %d PONGed client %s", srvnum, clientid);
				state = LISTEN;
			} else if (!strncmp(msg, "REPORT", 6)) {
				int r, token;
				long elapsed;

				sscanf(msg, "REPORT token %d returned %d after %ld seconds", &token, &r, &elapsed);
				err_debug("srv %d : %s has says task %d ended with status %d after %ld s of runtime", srvnum, clientid, token, r, elapsed);
				cmd_end(_dstsrv_.cmd, token, r, tstamp_get(), elapsed);
				//cmd_dump_all(_dstsrv_.cmd);
				state = TALK;
			} else {
				err_error("srv %d : ignoring %s's rude language (\"%s\")", srvnum, clientid, msg);
				cmd_client_died(_dstsrv_.cmd, clientid);
				return 4;
			}
		} else if (state == TALK) {
			char *cmd, cmdl[4096];
			int token;
			/* prepare to talk to client: */
			cmd = cmd_next(_dstsrv_.cmd, &token, _dstsrv_.maxrerun);
			if (token < 0) {
				srv_stop(_dstsrv_.srv);	
				err_debug("srv %d sending %s: DONE", srvnum, clientid);
				srv_send(s, "DONE", 5);
				break;
			}
			sprintf(cmdl, "PLEASE RUN token %d: %s", token, cmd);
			err_debug("srv %d sending %s: %s", srvnum, clientid, cmdl);
			if (srv_send_to(s, cmdl, strlen(cmdl) + 1, _dstsrv_.timeout) <=0) { 
				err_debug("respool command %d", token);
				cmd_respool(_dstsrv_.cmd, token);
			} else { 	
				err_debug("set command %d to run on %s", token, clientid);
				cmd_start(_dstsrv_.cmd, token, clientid, tstamp_get());
				//cmd_dump_all(_dstsrv_.cmd);
			}
			state = LISTEN;
		}
	}
	return 0;
}

void 
dstsrc_cli_check(void *foo) {
	err_debug("dstsrv checker thread launched with pingdelay = %d", _dstsrv_.pingdelay.tv_sec);
	do {
		sleep(_dstsrv_.pingdelay.tv_sec * 1.1);
	} while (!cmd_running_check(_dstsrv_.cmd, _dstsrv_.pingdelay));
}

void dstsrv_loop() {
	srv_cli_connect_loop_to(_dstsrv_.srv, dstsrv_srv_cli_dialog, dstsrv_must_stop, NULL, _dstsrv_.cnxwait);
}
	
int 
main(int n, char *a[]) {
	int i, r, cleanshm = 0;
	popt_t opt;
	char *conffile = NULL, *specfile = NULL;
	pthread_t tid1, tid2;
	pthread_attr_t attr1, attr2;
	pthread_attr_init(&attr1);
	pthread_attr_init(&attr2);

	dstsrv_init();

	/* Parse command line : */
	for (i = 1; i < n; i++) {
		if (!strcmp(a[i], "-c")) {
			conffile = a[++i];			
		} else if (!strcmp(a[i], "-s")) {
			specfile   = a[++i];
		} else if (!strcmp(a[i], "-e")) {
			strcpy(_dstsrv_.errfile, a[++i]);
		} else if (!strcmp(a[i], "-n")) {
			strcpy(_dstsrv_.newspec, a[++i]);
		} else if (!strcmp(a[i], "-r")) {
			strcpy(_dstsrv_.report, a[++i]);
		} else if (!strcmp(a[i], "-m")) {
			cleanshm = 1;
		} else {
			err_error("unknown arguement %s", a[i]);
		}
	}
	
	if (!conffile || !specfile) {
		err_error("conffile and specfile must be specified on command line");
		return 1;
	}
	
	if (!(opt = opt_new("[ \t]*([A-z0-9_]+)[ \t]*=[ \t]*(.*)[ \t]*\n"))) {
		err_error("cannot create option");	
		return 2;
	}

	opt_entry_add(opt, "maxclient", "%d",  sizeof(int),  &_dstsrv_.maxclient,      NULL);
	opt_entry_add(opt, "debug",     "%d",  sizeof(int),  &_dstsrv_.debug,          NULL);
	opt_entry_add(opt, "port",      "%d",  sizeof(int),  &_dstsrv_.port,           NULL);
	opt_entry_add(opt, "shkey",     "%d",  sizeof(int),  &_dstsrv_.shkey,          NULL);
	opt_entry_add(opt, "maxrerun",  "%d",  sizeof(int),  &_dstsrv_.maxrerun,       NULL);
	opt_entry_add(opt, "pingdelay", "%d",  sizeof(int),  &_dstsrv_.pingdelay,      NULL);
	opt_entry_add(opt, "timeout",   "%ld", sizeof(long), &_dstsrv_.timeout.tv_sec, NULL);
	opt_entry_add(opt, "cnxwait",   "%ld", sizeof(long), &_dstsrv_.cnxwait.tv_sec, NULL);

	opt_parse(opt, conffile, &_dstsrv_);
	opt_destroy(opt);

	if (_dstsrv_.debug) err_level_set(err_DEBUG);
	printf("debug = %d\n", _dstsrv_.debug);

	/* initialize command processor: */
	if (cleanshm) 
		_dstsrv_.cmd = cmd_force_init(_dstsrv_.shkey, _dstsrv_.cmd_size, specfile);
	 else 
		_dstsrv_.cmd = cmd_init(_dstsrv_.shkey, _dstsrv_.cmd_size, specfile);

	if (!_dstsrv_.cmd) {
		err_error("cannot initialize command processor");
		return 3;
	}

	/* server creation : */
	if (!(_dstsrv_.srv = srv_new(NULL, _dstsrv_.port, _dstsrv_.maxclient))) {
		err_error("Cannot create server");
		return 4;
	}

	dstsrv_dump();

	/* Mise en place des ping dans un second thread !!! */
	pthread_create(&tid1, &attr1, (void *(*)(void *)) dstsrc_cli_check, NULL);

	/* wait for clients: */
	pthread_create(&tid2, &attr2, (void *(*)(void *)) dstsrv_loop,      NULL); 

	while (cmd_remaining(_dstsrv_.cmd)) sleep(1);

	pthread_cancel(tid1);
	pthread_cancel(tid2);
	 
	/* TODO: write spec after computations */
	cmd_report_write(_dstsrv_.cmd, _dstsrv_.report);
	cmd_spec_write(_dstsrv_.cmd, _dstsrv_.newspec);

	r = cmd_error(_dstsrv_.cmd, _dstsrv_.errfile);

	cmd_exit(_dstsrv_.cmd);

	/* Everything is fine, then exit ok: */
	return r;
}
