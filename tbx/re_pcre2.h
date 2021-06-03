#ifndef _re_pcre2_h_
#define _re_pcre2_h_

#ifdef __cplusplus
extern "C" {
#endif 

#include <tbx/re.h>

int        re_pcre2_new(pre_t r);
int        re_pcre2_compile(pre_t r, int flags);
void *     re_pcre2_free(pre_t r);
prematch_t re_pcre2_match(pre_t r, char *p);

#ifdef __cplusplus
};
#endif

#endif
