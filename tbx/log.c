#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#ifdef HASSYSLOG
#include <syslog.h>
#endif
#ifdef HASJOURNALD
#include <systemd/sd-journal.h>
#endif

#include "mem.h"
#include "re.h"
#include "tstmr.h"
#include "patr.h"
#include "storage.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	21/07/2016  1.0  creation
*/   

char log_MODULE[]  = "log";
char log_PURPOSE[] = "Advanced loging to stdio, file syslog and/or journald with different log levels according to tags and environment variables";
char log_VERSION[] = "1.0";
char log_DATEVER[] = "21/07/2016";

char *logLEVEL_TXT[] = {
	"MESSAGE",
	"LOG",
	"ERROR",
	"WARNING",
	"INFO",
	"DEBUG",
	"TRACE"
};

char *logWHERE_TXT[] = {
	"undef0",
	"STDIO",
	"FILE",
	"undef1",
	"SYSLOG",
	"undef2",
	"undef3",
	"undef4",
	"JOURNALD",
	""
};

#define logBUFSIZE 1024
#define logTAGSIZE 50
#define logTAGSPEC "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"
#define logDEFAULT "default"


#define logENV_RE "^LOG_([A-Za-z0-9_]{1,50})=(MESSAGE|LOG|ERROR|WARNING|INFO|DEBUG|TRACE)(:(STDIO|FILE|SYSLOG|JOURNALD)(\\|(STDIO|FILE|SYSLOG|JOURNALD){1,2})?)?$"
#define logENV_SRE_TAG         1
#define logENV_SRE_LEVEL       2
#define logENV_SRE_WHERE_START 4
#define logENV_SRE_WHERE_STEP  2

typedef struct { 
	char  tag[logTAGSIZE + 1];
	int   level;	
	int   where;
} lmod_t, *plmod_t;

typedef struct {
	int	     syslogon;
	int	     syslogopt;
	int 	 syslogfacil;	

	FILE    *logfile;

	plmod_t	 lm_default;

	pstorage_t stor;
	ppatr_t  patr;
	FILE	 *file;
} log_t, *plog_t;

static plog_t _plog_ = NULL;

#define _log_c_
#include "log.h"
#undef  _log_c_

#define log_check_return() {if (!_plog_) _log_init(); if (!_plog_) return 1;}

static void _log_exit() {
	if (_plog_) {
		_plog_->patr = patr_destroy(_plog_->patr);
		_plog_->stor = storage_destroy(_plog_->stor);

#ifdef HASSYSLOG		
		if (_plog_->syslogon) {
			closelog();
			_plog_->syslogon = logSYSLOG_OFF;
		}
#endif
		if (_plog_->file) {
			fclose(_plog_->file);
			_plog_->file = NULL;
		}
		_plog_ = NULL;
	}
}

static void _log_init() {
	extern char **environ;
	int i;

	if (!_plog_) {
		if (!(_plog_ = mem_zmalloc(sizeof(log_t)))) {
			fprintf(stderr, "Failed to start logger\n");
			return;
		}
	}
	_plog_->patr = patr_new(logTAGSPEC);
	_plog_->stor = storage_new(sizeof(lmod_t), 20);
	log_config(logDEFAULT, logWARNING, logSTDIO);
	_plog_->lm_default = (plmod_t) patr_retrieve(_plog_->patr, logDEFAULT);

	/** TODO: Loop on environment variables **/
	for (i = 0; environ[i]; i++) {
		prematch_t m;
		pre_t r;
		int j;

		if (!(r = re_new(environ[i], logENV_RE, reEXTENDED))) {
			fprintf(stderr, "oops\n");
			continue; 
		}
		if ((m = re_find_first(r))) {
			char tag[logTAGSIZE + 1], *s = NULL;
			int level, where;
			level = where = 0;

			memset(tag, 0, logTAGSIZE + 1);
			strcpy(tag, s = re_substr(r, m->subs[logENV_SRE_TAG].so, m->subs[logENV_SRE_TAG].eo));
			if (s) free(s);

			s = re_substr(r, m->subs[logENV_SRE_LEVEL].so, m->subs[logENV_SRE_LEVEL].eo);
			if (!strcmp(s, logLEVEL_TXT[logMESSAGE])) {
				level = logMESSAGE;
			} else if (!strcmp(s, logLEVEL_TXT[logLOG])) {
				level = logLOG;
			} else if (!strcmp(s, logLEVEL_TXT[logERROR])) {
				level = logERROR;
			} else if (!strcmp(s, logLEVEL_TXT[logWARNING])) {
				level = logWARNING;
			} else if (!strcmp(s, logLEVEL_TXT[logINFO])) {
				level = logINFO;
			} else if (!strcmp(s, logLEVEL_TXT[logDEBUG])) {
				level = logDEBUG;
			} else if (!strcmp(s, logLEVEL_TXT[logTRACE])) {
				level = logTRACE;
			} else {
				level = _plog_->lm_default->level;
			}
			if (s) free(s);
			
			for (j = logENV_SRE_WHERE_START; j < m->nsubs; j += logENV_SRE_WHERE_STEP) {
				s = re_substr(r, m->subs[j].so, m->subs[j].eo);
				if (!strcmp(s, logWHERE_TXT[logSTDIO])) {	
					where |= logSTDIO;
				} else if (!strcmp(s, logWHERE_TXT[logFILE])) {
					where |= logFILE;
				} else if (!strcmp(s, logWHERE_TXT[logSYSLOG])) {
					where |= logSYSLOG;
				} else if (!strcmp(s, logWHERE_TXT[logJOURNALD])) {
					where |= logJOURNALD;
				} else {
					fprintf(stderr, "Unknown logging facility '%s' ignored\n", s);
				}
				free(s);	
			}
			if (where == 0) where = _plog_->lm_default->where;
			
			log_config(tag, level, where);
			atexit(_log_exit);
		}
		r = re_destroy(r);
	}
}

int log_config(char *tag, int level, int where) {
	plmod_t plm;

	log_check_return();

	if ((plm = (plmod_t) patr_retrieve(_plog_->patr, tag))) {
		plm->level = level;	
		plm->where = where;
	} else {
		lmod_t lm;
		memset(&lm, 0, logTAGSIZE +1);
		strncpy(lm.tag, tag, logTAGSIZE);
		lm.level = level;
		lm.where = where;
		plm = (plmod_t) storage_add(_plog_->stor, (char *) &lm);
		patr_store(_plog_->patr, plm->tag, plm);
	}
	return 0;	
}

int log_syslog(int onoff, int syslog_opt, int facility) {
	log_check_return();
#ifdef HASSYSLOG
	if (!onoff && _plog_->syslogon) {
		closelog(); 
		_plog_->syslogon    = logSYSLOG_OFF;
	} else if (onoff) {
		_plog_->syslogon    = logSYSLOG_ON;
		_plog_->syslogopt   = syslog_opt;
		_plog_->syslogfacil = facility;
		openlog(NULL, syslog_opt, facility);
	}
#endif
	return 0;
}

int log_file(char *filename) {
	log_check_return();
	if (filename) {
		_plog_->file = fopen(filename, "a+");
	}
	return 0;
}

int log_write(char *tag, int level, char *file, int line, char *func, char *fmt, ...) {
	plmod_t lm;

	log_check_return();

	if (!tag || !(lm = (plmod_t) patr_retrieve(_plog_->patr, tag)))
		lm = _plog_->lm_default;

	if (level <= lm->level) {
		if ((lm->where & logSTDIO) || 
			(lm->where & logSYSLOG && _plog_->syslogon) ||
		    (lm->where & logFILE   && _plog_->file))  {
			tstamp_t now;
			va_list args;
			char buf[logBUFSIZE], buf2[logBUFSIZE], ts[logBUFSIZE];
			va_start(args, fmt);

			now = tstamp_get();
			
			if (!strcmp(tag, logDEFAULT));

			if (func) {
				snprintf(buf2, logBUFSIZE, "%s %s in %s (%s:%d): %s", tag, logLEVEL_TXT[level], func, file, line, fmt);
			} else {
				snprintf(buf2, logBUFSIZE, "%s %s at %s:%d: %s", tag, logLEVEL_TXT[level], file, line, fmt);
			}
			vsnprintf(buf, logBUFSIZE, buf2, args);

			if (lm->where & logSTDIO) {
				fprintf(stderr, "%s %s\n", tstamp_fmt(ts, now), buf);
				fflush(stderr);
			}
			if (lm->where & logFILE && _plog_->file) {
				fprintf(_plog_->file, "%s %s\n", tstamp_fmt(ts, now), buf);
				fflush(_plog_->file);
			}
#ifdef HASSYSLOG
			if (lm->where & logSYSLOG && _plog_->syslogon) {
				syslog(_plog_->syslogopt | _plog_->syslogfacil, "%s %s\n", tstamp_fmt(ts, now), buf);
			}
#endif
#ifdef HASJOURNALD
			if (lm->where & logJOURNALD) {
				char b[logBUFSIZE];
				snprintf(b, logBUFSIZE - 1, "%s %s\n", tstamp_fmt(ts, now), buf);
				sd_journal_perror(buf);
			}
#endif
		}
	}
	return 0;
}	

#if 0
static void  _log_init() __attribute__((constructor));
static void  _log_exit() __attribute__((destructor));
#endif

#ifdef _test_log_

#include <unistd.h>
int main(int n, char *a[]) {
	int i = 1;
	
	if (n < 2 || strcmp(a[1], "SECOND_RUN")) {
		putenv("LOG_TEST_FILE_STDIO=DEBUG:FILE|STDIO");
		putenv("LOG_TEST_STDIO=WARNING:STDIO|SYSLOG");
		putenv("LOG_TEST_JD=logTRACE:STDIO|JOURNALD");
		execl(a[0], a[0], "SECOND_RUN", NULL); 
		exit(0);
	}
	log_config("default", logLOG, logSTDIO);

	log_file("test-log.log");
#ifdef HASSYSLOG
	log_syslog(logSYSLOG_ON, LOG_PID|LOG_NDELAY, LOG_LOCAL0);
	//log_syslog(logSYSLOG_ON, LOG_PID|LOG_NDELAY|LOG_PERROR, LOG_LOCAL0);
#endif
	
	log_message(logDEFAULT, "test %d", i++);
	log_log(logDEFAULT, "test %d", i++);
	log_error(logDEFAULT, "test %d", i++);
	log_warning(logDEFAULT, "test %d", i++);
	log_info(logDEFAULT, "test %d", i++);
	log_debug(logDEFAULT, "test %d", i++);
	
	log_message("TEST_FILE_STDIO", "test %d", i++);
	log_log("TEST_FILE_STDIO", "test %d", i++);
	log_error("TEST_FILE_STDIO", "test %d", i++);
	log_warning("TEST_FILE_STDIO", "test %d", i++);
	log_info("TEST_FILE_STDIO", "test %d", i++);
	log_debug("TEST_FILE_STDIO", "test %d", i++);
		
	log_message("TEST_STDIO", "test %d", i++);
	log_log("TEST_STDIO", "test %d", i++);
	log_error("TEST_STDIO", "test %d", i++);
	log_warning("TEST_STDIO", "test %d", i++);
	log_info("TEST_STDIO", "test %d", i++);
	log_debug("TEST_STDIO", "test %d", i++);
		
	log_msg("TEST_JD", "test %d", i++);
	log_log("TEST_JD", "test %d", i++);
	log_err("TEST_JD", "test %d", i++);
	log_wrn("TEST_JD", "test %d", i++);
	log_inf("TEST_JD", "test %d", i++);
	log_dbg("TEST_JD", "test %d", i++);

	log_message("undef", "test %d", i++);
	log_log("undef", "test %d", i++);
	log_error("undef", "test %d", i++);
	log_warning("undef", "test %d", i++);
	log_info("undef", "test %d", i++);
	log_debug("undef", "test %d", i++);
}

#endif

