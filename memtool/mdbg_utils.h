#ifndef _mdbg_utils_h_
#define _mdbg_utils_h_
char * mdbg_fmt_ptr(char *s, void *ptr);
char * mdbg_fmt_char(char *s, char c);
char * mdbg_fmt_xchar(char *s, char c);
char * mdbg_fmt_u(char *s, unsigned int ui);
char * mdbg_fmt_u_n(char *s, unsigned int ui, int m, char pad);
char * mdbg_str_apend(char *s, char *t);
#endif
