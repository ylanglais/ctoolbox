#include <stdio.h>
#ifndef _err_h_
#include "err.h"
#endif
#include "tstmr.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	28/04/2016  1.3  fixes
	04/08/2016  2.1  add separator in parameter & take text separator into account for counting fields.
*/   

char err_MODULE[]  = "Error messaging";
char err_PURPOSE[] = "Error and loging module";
char err_VERSION[] = "2.1";
char err_DATEVER[] = "04/08/2016";

char *err_TEXT[] = {
    "       ",
	"MESSAGE",
	"LOG    ",
	"ERROR  ",
	"WARNING",
	"INFO   ",
	"DEBUG  ",
	"TRACE  ",
};

#define errBUFSIZE 1024
#define errTSBSIZE   60

#if defined(ERR_LEVEL)
int  err_LEVEL = ERR_LEVEL;
#else
#if defined(__DEBUG__)
int  err_LEVEL = err_DEBUG;
#else 
#if defined(__TRACE__)
int err_LEVEL  = err_TRACE;
#else
int err_LEVEL  = err_WARNING;
#endif
#endif
#endif
 
FILE *err_FILE = NULL;

#if defined(__GNUC__) || defined(_SUN_) 
void 
err_write(int level, char *file, int line, char *func, char *fmt, ...) {
    va_list args;
    char buf[errBUFSIZE], buf2[errBUFSIZE], ts[errTSBSIZE];
    va_start(args, fmt);
	if (!err_FILE) err_init(NULL, -1);
	if (level > err_LEVEL) return;
	if (func) {
		snprintf(buf2, errBUFSIZE, "%s in %s (%s:%d): %s", err_TEXT[level], func, file, line, fmt);
	} else {
		snprintf(buf2, errBUFSIZE, "%s at %s:%d: %s", err_TEXT[level], file, line, fmt);
	}
    vsnprintf(buf, errBUFSIZE, buf2, args);
    fprintf(err_FILE, "%s %s\n", tstamp_fmt(ts, tstamp_get()),buf);
	fflush(err_FILE);
    va_end(args);
}
#else 
void err_message(char *fmt, ...) {
	va_list args; va_start(args,);
	err_write(err_MESSAGE, "NOFILE", -1, NULL, fmt , args);
	va_end(args);
}
void err_log(char *fmt, ...) {
	va_list args; va_start(args,);
	err_write(err_LOG, "NOFILE", -1, NULL, fmt , args);
	va_end(args);
}
void err_error(char *fmt, ...) {
	va_list args; va_start(args,);
	err_write(err_ERROR, "NOFILE", -1, NULL, fmt , args);
	va_end(args);
}
void err_warning(char *fmt, ...) {
	va_list args; va_start(args,);
	err_write(err_WARNING, "NOFILE", -1, NULL, fmt , args);
	va_end(args);
}
void err_info(char *fmt, ...) {
	va_list args; va_start(args,);
	err_write(err_INFO, "NOFILE", -1, NULL, fmt , args);
	va_end(args);
}
void err_debug(char *fmt, ...) {
	va_list args; va_start(args,);
	err_write(err_DEBUG, "NOFILE", -1, NULL, fmt , args);
	va_end(args);
}
void err_trace(char *fmt, ...) {
	va_list args; va_start(args,);
	err_write(err_TRACE, "NOFILE", -1, NULL, fmt , args);
	va_end(args);
}
void 
err_write(int level, char *file, int line, char *func, char *fmt, va_list args) {
    char buf[errBUFSIZE], buf2[errBUFSIZE], ts[errTSBSIZE];
	if (!err_FILE) err_init(NULL, -1);
	if (level > err_LEVEL) return;
	if (func) {
		snprintf(buf2, errBUFSIZE, "%s in %s (%s:%d): %s", err_TEXT[level], func, file, line, fmt);
	} else {
		snprintf(buf2, errBUFSIZE, "%s at %s:%d: %s", err_TEXT[level], file, line, fmt);
	}
    vsnprintf(buf, errBUFSIZE, buf2, args);
    fprintf(err_FILE, "%s %s\n", tstamp_fmt(ts, tstamp_get()), buf);
	fflush(err_FILE);
}
#endif 

void err_init(char *filename, int level) {
    if (filename) err_FILE = fopen(filename, "a");
	if (!err_FILE) err_FILE = stderr;
	err_level_set(level);
	err_message("%s version %s initialisation (level = %s)", err_MODULE, err_VERSION, err_TEXT[err_LEVEL]);
}

int err_level_set(int level) {
	if (!err_FILE) err_init(NULL, level);
    if (level >  err_NONE) err_LEVEL = level;
	else                   err_LEVEL = err_ERROR;
	return err_LEVEL;
}

int err_level_get() {
	return err_LEVEL;
}
