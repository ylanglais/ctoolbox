#ifndef _Option_h_
#define _Option_h_

#include <option.h>

class Option {
private:
 	popt_t option;

public:
	Option(char *format) { option = opt_new(format);     }
	~Option()            { option = opt_destroy(option); }

	int entry_add(char *name, char *format, size_t size, void *var, void (*set)(void *, char *)) {
		return opt_entry_add(option, name, format, size, var, set); 
	}

	int parse(char filename, void *conf) {
		return opt_parse(option, filename, conf);
	}
};

#endif
