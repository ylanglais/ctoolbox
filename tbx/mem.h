
#ifndef _mem_h_
#define _mem_h_
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#define mem_malloc(s) 		malloc(s)
#define mem_realloc(p, s) 	realloc((p), (s))
#define mem_clean(p) 		(p) = mem_free((p))
#define mem_strdup(s)       strdup(s)


#ifdef __cplusplus
extern "C" {
#endif

void * mem_zmalloc(size_t s);
void * mem_free(void *p);

#ifdef __cplusplus
}
#endif

#endif
