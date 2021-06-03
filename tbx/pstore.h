#ifndef _pstore_h_
#define _pstore_h_

#ifdef __cplusplus
extern "C" {
#endif

/* 
 *	Persistant storage management based on memory mapped files: 
 */

#ifndef _pstore_c_
typedef void *ppstore_t;
#endif

size_t    pstore_sizeof();
ppstore_t pstore_new(char *filename, size_t unit, size_t grain);
ppstore_t pstore_destroy(ppstore_t ps);

char *    pstore_add(ppstore_t ps, char *pdata);
char *    pstore_get(ppstore_t ps, int i);
char *    pstore_set(ppstore_t ps, int i, char *pdata);

int       pstore_used(ppstore_t ps);
int       pstore_allocated(ppstore_t ps);
int       pstore_available(ppstore_t ps);
int       pstore_grain(ppstore_t ps);
int       pstore_unit(ppstore_t ps);
int       pstore_pages(ppstore_t ps);

#ifdef __cplusplus
}
#endif

#endif
