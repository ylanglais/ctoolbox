

#ifndef _Atree_h_
#define _Atree_h_

#include "atree.h"

class Atr {
private: 
	patree_t atree;

public:
	Atr()  { atree = atree_new(); };
	~Atr() { atree = atree_destroy(atree); };
	
	void *retrieve(char *key)           { return atree_retrieve(atree, key);          };
	void *operator [] (const char *key) { return atree_retrieve(atree, (char *) key); };
	void *store(char *key, char *data)  { return atree_store(atree, key, data);       };

	void foreach(f_hook_t f_hook)       { return atree_foreach(atree, f_hook);        };

	void dump(f_dumpdata_t f_dumpdata)  { return atree_dump(atree, f_dumpdata);       };
};

#endif
