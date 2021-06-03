#ifndef _patr_h_
#define _patr_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _patr_c_
typedef struct _patr_t *ppatr_t;
#endif

/* 
 *  This indexing library is intended to provide a fast data retrieval  
 *  from simple [small] dictionaries. 
 */

typedef void (*f_hook_t)(int level, char *key, void *data);
typedef void (*f_dumpdata_t)(void *);

size_t  patr_sizeof();

ppatr_t 	patr_new(char *spec);
ppatr_t 	patr_destroy(ppatr_t p);
void   	   *patr_retrieve(ppatr_t p, char *key);
void       *patr_store(ppatr_t p, char *key, void *data);

void        patr_foreach(ppatr_t p, f_hook_t f_hook);

/* for debugging purpose: */
void        patr_dump(ppatr_t p, f_dumpdata_t f_dumpdata);

#ifdef __cplusplus
}
#endif

#endif
