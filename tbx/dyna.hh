#ifndef _Dyna_h_
#define _Dyna_h_

#include <dyna.h>

class Dyna {
private: 
	pdyna_t dyna;

public:

	Dyna(size_t unit, size_t grain)   { dyna = dyna_new(unit, grain);       };
	~Dyna()                           { dyna = dyna_destroy(dyna);          };
	
	char * add(char *pdata)           { return dyna_add(dyna, pdata);       };
	char * insert(int i, char *pdata) { return dyna_insert(dyna, i, pdata); };
	int    remove(int i)              { return dyna_delete(dyna, i);        };
    int    remove(void *ptr) 		  { return dyna_delete_ptr(dyna, ptr);  };

	char * push(char *pdata)          { return dyna_push(dyna, pdata);      };
	char * pop_first()                { return dyna_pop_first(dyna);        };
	char * pop_last()                 { return dyna_pop_last(dyna);         };

	char * get(int i)				  { return dyna_get(dyna, i);           };
	char * operator[](int i)          { return dyna_get(dyna, i);           };
	char * set(int i, char *pdata)    { return dyna_set(dyna, i, pdata);    };

	void * data_ptr(int i)            { return dyna_data_ptr(dyna, i);      };

	int    used()                     { return dyna_used(dyna);             };
	int    count()                    { return dyna_count(dyna);            };
	int    allocated()                { return dyna_allocated(dyna);        };
	int    availalble()               { return dyna_available(dyna);        };
	int    grain()                    { return dyna_grain(dyna);            };
	int    unit()                     { return dyna_unit(dyna);             };
	int    pages()                    { return dyna_pages(dyna);            };
};

#endif
