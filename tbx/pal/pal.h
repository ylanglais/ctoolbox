#ifndef _pal_h_
#define _pal_h_

#ifdef __cplusplus
extern "C" {
#endif

void *pmalloc(char *filename, size_t size);
void *prealloc(void *ptr, size_t size);
void *pfree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
