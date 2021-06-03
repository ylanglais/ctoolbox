#ifndef _re_posix_h_
#define _re_posix_h_

#ifdef __cplusplus
extern "C" {
#endif 

#include <tbx/re.h>

int        re_posix_new(pre_t r);
int        re_posix_compile(pre_t r, int flags);
void *     re_posix_free(pre_t r);
prematch_t re_posix_match(pre_t r, char *p);

#ifdef __cplusplus
};
#endif

#endif
