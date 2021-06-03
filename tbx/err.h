#ifndef _err_h_
#define _err_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

enum {
    err_NONE = 0,
	err_MESSAGE,
	err_LOG,
	err_ERROR,
	err_WARNING,
	err_INFO,
	err_DEBUG,
	err_TRACE,
	err_COVERAGE
};

void  err_init(char *filename, int level);
int   err_level_set(int level);

#ifdef __GNUC__

#ifdef __HAS_FUNC__
#define err_message(fmt, ...) err_write(err_MESSAGE, __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define err_log(fmt, ...) 	  err_write(err_LOG    , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define err_error(fmt, ...)   err_write(err_ERROR  , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define err_warning(fmt, ...) err_write(err_WARNING, __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define err_info(fmt, ...)    err_write(err_INFO   , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define err_debug(fmt, ...)   err_write(err_DEBUG  , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#define err_trace(fmt, ...)   err_write(err_TRACE  , __FILE__, __LINE__, (char *) __func__, fmt , ## __VA_ARGS__) 
#else /* __HAS_FUNC__ */
#define err_message(fmt, ...) err_write(err_MESSAGE, __FILE__, __LINE__, NULL,     fmt , ## __VA_ARGS__) 
#define err_log(fmt, ...)     err_write(err_LOG    , __FILE__, __LINE__, NULL,     fmt , ## __VA_ARGS__) 
#define err_error(fmt, ...)   err_write(err_ERROR  , __FILE__, __LINE__, NULL,     fmt , ## __VA_ARGS__) 
#define err_warning(fmt, ...) err_write(err_WARNING, __FILE__, __LINE__, NULL,     fmt , ## __VA_ARGS__) 
#define err_info(fmt, ...)    err_write(err_INFO   , __FILE__, __LINE__, NULL,     fmt , ## __VA_ARGS__) 
#define err_debug(fmt, ...)   err_write(err_DEBUG  , __FILE__, __LINE__, NULL,     fmt , ## __VA_ARGS__) 
#define err_trace(fmt, ...)   err_write(err_TRACE  , __FILE__, __LINE__, NULL,     fmt , ## __VA_ARGS__) 
#endif /* __HAS_FUNC__ */

void  err_write(int level, char *file, int line, char *func, char *fmt, ...);
#else /* __GNUC__ */

#ifdef _SUN_ 

#define err_message(...) err_write(err_MESSAGE, __FILE__, __LINE__, (char *) __func__, __VA_ARGS__)
#define err_log(...)     err_write(err_LOG    , __FILE__, __LINE__, (char *) __func__, __VA_ARGS__)
#define err_error(...)   err_write(err_ERROR  , __FILE__, __LINE__, (char *) __func__, __VA_ARGS__)
#define err_warning(...) err_write(err_WARNING, __FILE__, __LINE__, (char *) __func__, __VA_ARGS__)
#define err_info(...)    err_write(err_INFO   , __FILE__, __LINE__, (char *) __func__, __VA_ARGS__)
#define err_debug(...)   err_write(err_DEBUG  , __FILE__, __LINE__, (char *) __func__, __VA_ARGS__)
#define err_trace(...)   err_write(err_TRACE  , __FILE__, __LINE__, (char *) __func__, __VA_ARGS__)
void  err_write(int level, char *file, int line, char *func, char *fmt, ...);

#else /* _SUN_ */
void error_write(int level, char *file, int line, char *func, char *fmt, va_list args);
void err_message(char *fmt, ...);
void err_log(char *fmt, ...);
void err_error(char *fmt, ...);
void err_warning(char *fmt, ...);
void err_info(char *fmt, ...);
void err_debug(char *fmt, ...);
void err_trace(char *fmt, ...);
#endif /* _SUN_ */

#endif /* __GNUC__ */

#ifdef __cplusplus
}
#endif

#endif
