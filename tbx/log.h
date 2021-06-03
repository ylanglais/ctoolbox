#ifndef _log_h_
#define _log_h_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

enum {
	logMESSAGE = 0, 
	logLOG,
	logERROR,
	logWARNING,
	logINFO,
	logDEBUG,
	logTRACE
};

enum {
	logUNDEF0  = 0,
	logSTDIO   = 1,
	logFILE    = 2,
	logUNDEF3,
	logSYSLOG  = 4,
	logUNDEF5,
	logUNDEF6,
	logUNDEF7,
	logJOURNALD = 8
};

enum {
	logSYSLOG_OFF = 0,
	logSYSLOG_ON
};

#define logDEFAULT "default"

/* Tag spec: 
#define logTAGSIZE 50
#define logTAGSPEC "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"
#define logDEFAULT "default"
*/

int log_config(char *tag, int level, int where);
int log_syslog(int onoff, int syslog_opt, int facility);
int log_file(char *filename);
int log_write(char *tag, int level, char *file, int line, char *func, char *fmt, ...);

#define log_message(tag, fmt, ...) log_write(tag, logMESSAGE, __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define log_log(tag, fmt, ...) 	   log_write(tag, logLOG    , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define log_error(tag, fmt, ...)   log_write(tag, logERROR  , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define log_warning(tag, fmt, ...) log_write(tag, logWARNING, __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define log_info(tag, fmt, ...)    log_write(tag, logINFO   , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define log_debug(tag, fmt, ...)   log_write(tag, logDEBUG  , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define log_trace(tag, fmt, ...)   log_write(tag, logTRACE  , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 

#define log_msg  log_message
#define log_err  log_error
#define log_wrn  log_warning
#define log_warn log_warning
#define log_inf	 log_info
#define log_dbg  log_debug
#define log_trc  log_trace


#ifdef __cplusplus
}
#endif
#endif
