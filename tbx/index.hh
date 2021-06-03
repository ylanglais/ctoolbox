#ifndef _Index_h_
#define _Index_h_

#include <list.hh>

typedef void (*f_index_hook_t)(void *data); 

class Index {
private:
	pindex_t index;

public:
	Index()                             { index = index_new();                    }
	~Index()                            { index = index_destroy(index);           }

	int    add(char *key, void *data)   { return list_add(index, key, data);      }
	void * retrieve_fist(char *key)     { return list_retrieve_first(index, key); }
	void * retrieve_next(char *key)     { return list_retrieve_first(index, key); }
	
	List   retrieve(char *key)          { return List(index_retrieve_list(index, key)); }	
	List   operator [] (char *key)      { return List(index_retrieve_list(index, key)); }	

	void   foreach(f_index_hook_t hook) { index_foreach(index, hook);             }
};

#endif
