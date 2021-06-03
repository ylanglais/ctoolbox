#ifndef _atr_h_
#define _atr_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _atr_c_
typedef struct _atr_t *patr_t;
#endif

/* 
 *  This indexing library is intended to provide a fast data retrieval  
 *  from simple [small] dictionaries. 
 */

typedef void (*f_hook_t)(int level, char *key, void *data);
typedef void (*f_dumpdata_t)(void *);

size_t  atr_sizeof();

patr_t 	atr_new();
patr_t 	atr_destroy(patr_t p);
void   *atr_retrieve(patr_t p, char *key);
void   *atr_store(patr_t p, char *key, void *data);

void    atr_foreach(patr_t p, f_hook_t f_hook);

/* for debugging purpose: */
void    atr_dump(patr_t p, f_dumpdata_t f_dumpdata);

#ifdef __cplusplus
}
#endif

#endif
