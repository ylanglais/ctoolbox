#ifndef _pat_h_
#define _pat_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _pat_c_
typedef struct _pat_t *ppat_t;
#endif

/* 
 *  This indexing library is intended to provide a fast data retrieval  
 *  from simple [small] dictionaries. 
 */

typedef void (*f_hook_t)(int level, char *key, void *data);
typedef void (*f_dumpdata_t)(void *);

size_t  pat_sizeof();

ppat_t 	pat_new(char *spec);
ppat_t 	pat_destroy(ppat_t p);
void   *pat_retrieve(ppat_t p, char *key);
void   *pat_store(ppat_t p, char *key, void *data);

void    pat_foreach(ppat_t p, f_hook_t f_hook);

/* for debugging purpose: */
void    pat_dump(ppat_t p, f_dumpdata_t f_dumpdata);

#ifdef __cplusplus
}
#endif

#endif
