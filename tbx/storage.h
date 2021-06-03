#ifndef __storage_h__
#define __storage_h__

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef __storage_c__
typedef struct storage_t *pstorage_t;
#include <stdlib.h>
#endif

size_t storage_sizeof();

/* Constructor: */
pstorage_t storage_new(size_t unit, size_t grain);
/* Destructor :*/
pstorage_t storage_destroy(pstorage_t ps);

/* Add / get methods: */
char * storage_add(pstorage_t ps, char *pdata);
char * storage_alloc(pstorage_t ps);
char * storage_get(pstorage_t ps, int i);
char * storage_set(pstorage_t ps, int i, char *pdata);

/* info / monitoring methods: */
int storage_used(pstorage_t ps);
int storage_allocated(pstorage_t ps);
int storage_available(pstorage_t ps);
int storage_grain(pstorage_t ps);
int storage_unit(pstorage_t ps);

#ifdef __cplusplus
}
#endif

#endif
