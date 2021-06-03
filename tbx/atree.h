#ifndef __atree_h__
#define __atree_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _atree_c_
typedef struct _atree_t *patree_t;
#endif

/* 
 *  This indexing library is intended to provide a fast data retrieval  
 *  from simple [small] dictionaries. It is memory extensive.
 */

typedef void (*f_hook_t)(int level, char *key, void *data);
typedef void (*f_dumpdata_t)(void *);

size_t	 atree_sizeof();

patree_t atree_new();
patree_t atree_destroy(patree_t p);
void *   atree_retrieve(patree_t p, char *key);
void *   atree_store(patree_t p, char *key, void *data);

void     atree_foreach(patree_t p, f_hook_t f_hook);

/* for debugging purpose: */
void     atree_dump(patree_t p, void (*f_dumpdata)(void *));

#ifdef __cplusplus
}
#endif

#endif
