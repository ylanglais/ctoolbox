#ifndef _Pstore_h_
#define _Pstore_h_

#include <store.h>

/* 
 *	Persistant storage management based on memory mapped files: 
 */

class Pstore {
private:
	ppstore_t pstore;

public:
	Pstore(char *filename, size_t unit, size_t grain) { pstore = pstore_new(filename, size, grain); }
	~Pstore() { pstore = pstore_destroy(pstore); }
	
	char * add(char *pdata)    { return pstore_add(pstore, pdata); }
	char * get(int i)          { return pstore_get(pstore, i);     }
	char * operator [] (int i) { return pstore_get(pstore, i);     }

	int    used()      { return pstore_used(pstore);      }
	int    allocated() { return pstore_allocated(pstore); }
	int    available() { return pstore_available(pstore); }
	int    grain()     { return pstore_grain(pstore);     }
	int    unit()      { return pstore_unit(pstore);      }
	int    pages()     { return pstore_page(pstore);      }

}

#endif
