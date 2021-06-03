#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "mem.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	04/08/2016  3.1  add changelog & tbx tags
*/   

char str_MODULE[]  = "str";
char str_PURPOSE[] = "string management";
char str_VERSION[] = "3.1";
char str_DATEVER[] = "04/08/2016";

typedef struct {
	char *string;
	size_t len;
} str_t, *pstr_t;

#define _str_c_
#include "str.h"
#undef  _str_c_

size_t str_sizeof() { return sizeof(str_t); }

pstr_t str_free(pstr_t ps) {
	if (!ps) return NULL;
	if (ps->string) mem_free(ps->string);
	ps->len = 0;
	mem_free(ps);
	return NULL;
}

pstr_t str_destroy(pstr_t ps) {
	return str_free(ps);
}

pstr_t str_new(size_t init) {
	pstr_t ps;
	if (!(ps = (pstr_t) mem_zmalloc(sizeof(str_t)))) return NULL;
	if (!(ps->string = (char *) mem_zmalloc(init * sizeof(char)))) return str_free(ps);
	ps->len = init;
	ps->string[0] = '\0';
	return ps;
}

char *
str_string(pstr_t ps) {
	if (!ps) return NULL;
	if (!ps->string) return NULL;
	return mem_strdup(ps->string);
}

size_t
str_len(pstr_t ps) {
	if (!ps) return 0;
	if (!ps->string) return 0;
	return strlen(ps->string);
}

char *
str_subst(pstr_t ps, int start, int stop) {
	char *str;
	if (!ps) return NULL;
	if (start >= str_len(ps)) return NULL;
	if (stop >= str_len(ps)) stop = str_len(ps) - 1;
	if (stop - start < 1) return NULL;
	str = (char *) mem_zmalloc((stop - start + 1) * sizeof(char));
	strncpy(str, ps->string + start, stop - start);
	return str;
}

pstr_t
str_add_char(pstr_t ps, char c) {
	size_t len;
	if (!ps) return NULL;
	len = strlen(ps->string);
	if (len >= ps->len - 1) {
		ps->string = (char *) mem_realloc(ps->string, ps->len + 10);
		ps->len += 10;
	}
	ps->string[len] = c;
	ps->string[len + 1] = '\0';
	return ps;
}

pstr_t
str_add_string(pstr_t ps, char *str) {
	size_t len;
	if (!ps) return NULL;
	if (!str || strlen(str) < 1) return ps;
	len = strlen(ps->string);
	if (len + strlen(str) >= ps->len - 1) {
		ps->string = (char *) mem_realloc(ps->string, ps->len + strlen(str) + 10);
		ps->len += strlen(str) + 10;
	}
	strcat(ps->string, str);
	return ps;
} 

#if 0
pstr_t
str_insert_char(pstr_t ps, int pos, char c) {
	int len, i;
	if (!ps) return NULL;
	len = strlen(ps->string);
	if (pos >= len) return str_add_char(ps, c);
	if (pos < 0) pos = 0;
	if (len >= ps->len - 1) {
		ps->string = (char *) mem_realloc(ps->string, ps->len + 1);
		ps->len += strlen(str) + 1;
	}
	for (i = strlen(ps->string - 1; i >= pos; i--) ps->string[i + 1] = ps->string[i];
	ps->string[pos] = c;
	return ps;
}	
	
pstr_t
str_insert_string(pstr_t ps, int pos, char *string) {
}
#endif

#if defined(__DEBUG__) || defined(_test_str_)
#include <stdio.h>

#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }

#if defined(_test_str_)
int
main(void) {
#else
int str_test(void) {
#endif
	pstr_t p;
	char *s;

	p = str_new(3);

	str_add_char(p, 'a');
	str_add_char(p, 'b');
	str_add_char(p, 'c');

	str_add_char(p, 'd');
	str_add_char(p, 'e');
	str_add_char(p, 'f');
	str_add_char(p, 'g');
	
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'd'));
	echo_cmd(str_add_char(p, 'e'));
	echo_cmd(str_add_char(p, 'f'));
	echo_cmd(str_add_char(p, 'g'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'd'));
	echo_cmd(str_add_char(p, 'e'));
	echo_cmd(str_add_char(p, 'f'));
	echo_cmd(str_add_char(p, 'g'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(str_add_char(p, 'a'));
	echo_cmd(str_add_char(p, 'b'));
	echo_cmd(str_add_char(p, 'c'));
	echo_cmd(str_add_string(p, "1234567890"));
	echo_cmd(s = str_string(p));
	printf("string = `%s'\n", s);
	if (s) s = mem_free(s);

	str_free(p);

return 0;
}

#endif
