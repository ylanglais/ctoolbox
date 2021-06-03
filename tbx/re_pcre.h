#ifndef _re_pcre_h_
#define _re_pcre_h_

#ifdef __cplusplus
extern "C" {
#endif 

#include <tbx/re.h>

int        re_pcre_new(pre_t r);
int        re_pcre_compile(pre_t r, int flags);
void *     re_pcre_free(pre_t r);
prematch_t re_pcre_match(pre_t r, char *p);

#ifdef __cplusplus
};
#endif

#endif
