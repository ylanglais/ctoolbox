#ifndef _str_h_
#define _str_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _str_c_
typedef struct _str_t *pstr_t;
#endif  

size_t str_sizeof();

pstr_t str_new(size_t init);
pstr_t str_free(pstr_t ps);
pstr_t str_destroy(pstr_t ps);

char * str_string(pstr_t ps);
char * str_subst(pstr_t ps, int start, int stop);
size_t str_len(pstr_t ps);

pstr_t str_add_char(pstr_t ps, char c);
pstr_t str_add_string(pstr_t ps, char *str); 

#if defined(__DEBUG__)
#if !defined(_test_str_c_)
int str_test(void);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
