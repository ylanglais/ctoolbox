#ifndef _fmt_h_
#define _fmt_h_

#ifdef __cplusplus
extern "C" {
#endif

char * fmt_ptr(char *s, void *ptr);
char * fmt_char(char *s, char c);
char * fmt_xchar(char *s, char c);
char * fmt_u(char *s, unsigned int ui);
char * fmt_u_n(char *s, unsigned int ui, int m, char pad);
char * fmt_xd(char *s, double d);
char * fmt_xld(char *s, long double d);
char * fmt_str_append(char *s, char *t);
void   fmt_stderr_print(char *s);
void   fmt_stdio_print(char *s);
void   fmt_fd_set(int fd);
void   fmt_print(char *s);
void   fmt_dump_bin_data(char *buff, size_t size);
char * fmt_bin(char *data, size_t size);
#ifdef __cplusplus
}
#endif

#endif
