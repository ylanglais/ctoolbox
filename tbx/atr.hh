

#ifndef _Atr_h_
#define _Atr_h_

#include <atr.h>

class Atr {
private: 
	patr_t atr;

public:
	Atr()  { atr = atr_new(); };
	~Atr() { atr = atr_destroy(atr); };
	
	void *retrieve(char *key)           { return atr_retrieve(atr, key);          };
	void *operator [] (const char *key) { return atr_retrieve(atr, (char *) key); };
	void *store(char *key, char *data)  { return atr_store(atr, key, data);       };

	void foreach(f_hook_t f_hook)       { return atr_foreach(atr, f_hook);        };

	void dump(f_dumpdata_t f_dumpdata)  { return atr_dump(atr, f_dumpdata);       };
};

#endif
