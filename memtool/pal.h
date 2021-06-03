#ifndef _pal_h_
#define _pal_h_

void *pmalloc(char *filename, size_t size);
void *preload(char *filename);
void *prealloc(void *ptr, size_t size);
void *pfree(void *ptr);

#endif
