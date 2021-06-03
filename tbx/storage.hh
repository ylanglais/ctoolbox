#ifndef _Storage_h_
#define _Storage_h_

#include <storage.h>

class Storage {
private:
	pstorage_t store;

public:
	Storage(size_t size, size_t grain) { store = storage_new(size, grain); };
	~Storage()                         { store = storage_destroy(store);   };

	char * add(char *pdata)    { return storage_add(store, pdata); };
	char * get(int i)          { return storage_get(store, i);     };
	char * operator [] (int i) { return get(i);                    };

	int used()      { return storage_used(store);      };
	int allocated() { return storage_allocated(store); };
	int available() { return storage_available(store); };
	int grain()     { return storage_grain(store);     };
	int unit()      { return storage_unit(store);      };
};

#endif
	
