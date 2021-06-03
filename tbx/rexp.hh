#ifndef _Rexp_h_
#define _Rexp_h_

#include <rexp.h>

class Rexp {
private:
	prexp_t rexp;

public:
	Rexp(char *buffer, char *reg_exp, int flags = rexp_EXTENDED) {
		rexp = rexp_new(buffer, reg_exp, flags);
	}
	~Rexp() { rexp = rexp_destroy(rexp); }	

	char *  buffer()                 { return rexp_buffer_get(rexp); }
	void    buffer(char *buffer)     { rexp = rexp_buffer_set(rexp, buffer); }

	int     count_match()            { return rexp_count_match(rexp); }
	int     find()                   { return rexp_find(rexp); }

	char *  match_get()              { return rexp_match_get(rexp);       }
	int     match_index()            { return rexp_match_index_get(rexp); }
	size_t  nsubs()                  { return rexp_nsubs(rexp); }

	char *  sub_get(int i)           { return rexp_sub_get(rexp, i); }
	char *  operator [] (int i)      { return rexp_sub_get(rexp, i); }

	int     replace_first(char *str) { return rexp_replace_first(rexp, string);     }
	int 	replace_next(char *str)  { return rexp_replace_next(rexp, string);      }
	int 	replace_all(char *str)   { return rexp_replace_all(rexp, char *string); }
}

#endif
