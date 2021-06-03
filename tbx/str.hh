#ifndef _Str_h_
#define _Str_h_

#include <str.h>

class Str {
private:
	pstr_t str;

public:
	Str(size_t init)  { str = str_new(init);    }
	~str()            { str = str_destroy(str); }

	char *string()    { return str_string(str); }
	char * operator * { return str_sting(str);  }
	char *subbstr(int start, int stop) { return str_subst(str, start, stop); }
	size_t len()      { return str_len(str); }

	char operator [] (int i) { return str[i]; }

	Str add_char(char c)   { str_add_char(str, c); return this; }
	Str add_string(Str s)  { str_add_string(str, s.str); }
	Str operator + (Str s) { str_add_string(str, s.str); }
}

#endif
