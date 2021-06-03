#ifndef _index_h_
#define _index_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <tbx/list.h>

#ifndef _index_c_
typedef void *pindex_t;
#endif

typedef void (*f_index_hook_t)(void *data); 

size_t	 index_sizeof();
pindex_t index_new();
pindex_t index_destroy(pindex_t i);

int      index_add(pindex_t i, char *key, void *data);
void *   index_retrieve_first(pindex_t i, char *key);
void *   index_retrieve_next(pindex_t i, char *key);

plist_t  index_list_retrieve(pindex_t i, char *k);
void index_foreach(pindex_t i, f_index_hook_t hook);


#ifdef __cplusplus
}
#endif

#endif
