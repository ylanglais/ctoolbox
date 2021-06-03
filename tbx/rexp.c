#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <regex.h>

#include "str.h"
#include "mem.h"
#include "err.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	30/05/2016  2.1  small fix & clean
	04/08/2016  2.2  add changelog & tbx tags
*/   

char rexp_MODULE[]  = "Regular expression encapsulation";
char rexp_PURPOSE[] = "Regular expression encapsulation (old implementation, prefer re module)";
char rexp_VERSION[] = "2.2";
char rexp_DATEVER[] = "04/08/2016";

#define rexp_EXTENDED           1
#define rexp_IGNORECASE         (rexp_EXTENDED << 1)
#define rexp_NL_DO_NOT_MATCH    (rexp_IGNORECASE << 1)

#define MAX_SUB 30

typedef struct _rexp_t {
	int nsubs, isub;
	char *pbuf, *last, *buffer, *regexp;
	regex_t *pre;
	regmatch_t a_match[MAX_SUB];
	size_t nmatch;
} rexp_t, *p_rexp_t; 

static char *
_rexp_extract_substr_(char *string, int start_offset, int stop_offset) {
	char *p;
	int len, i;
	if (!string) return NULL;
	len = strlen(string);
	if (start_offset < 0 || start_offset >= len) return NULL;
	if (stop_offset <= start_offset) return NULL;
	if (stop_offset > len) stop_offset = len;
	len = stop_offset - start_offset;
	if (len <= 0) return NULL; 
	len += 2;
	p = (char *) mem_malloc((size_t) len * sizeof(char));
	for (i = start_offset; i < stop_offset; i++) {
		p[i - start_offset] = string[i];
	}
	p[i - start_offset] = '\0';
	return p;	
}

static char *
_rexp_update_replacement_string_(p_rexp_t prexp, char *string) {
	pstr_t ps = NULL;
	char *new, *p;
	
	p = string;
	ps = str_new(strlen(string));

	while (*p) {
		if (p[0] == '$' && p[1] >= '0' && p[1] <= '9') {
			/* check if quoted by \: */

			int bksl = 0;
			while ((p - 1 - bksl >= string) && (p[-1 - bksl] == '\\')) bksl++;
			if (!(bksl % 2)) { 
				/* $[0-9] is not quoted: */

				int index, i;
				index = (int) (p[1] - '0');

				/* replace $[0-9] by the appropriate substring: */
				/*str_add_string(ps, rexp_sub_get(prexp, index);*/
				for (i = prexp->a_match[index].rm_so; i < prexp->a_match[index].rm_eo; i++) {
					str_add_char(ps, prexp->last[i]);
				}
				p += 2;
			} else {
				str_add_char(ps, *p); p++;
				str_add_char(ps, *p); p++;
			}
		} else {
			str_add_char(ps, *p);
			p++;
		}
	}
	new = str_string(ps);
	str_free(ps);
	return new;
}

p_rexp_t
rexp_free(p_rexp_t prexp) {
	if (!prexp) return NULL;
	if (prexp->buffer) mem_free(prexp->buffer);
	if (prexp->pbuf) prexp->pbuf = NULL;
	if (prexp->last) prexp->last = NULL;
	if (prexp->regexp) mem_free(prexp->regexp);
	if (prexp->pre) {
		regfree(prexp->pre);
		mem_free(prexp->pre);
	}
	mem_free(prexp);
	return NULL;
}	

p_rexp_t
rexp_destroy(p_rexp_t prexp) {
	return rexp_free(prexp);
}

size_t rexp_sizeof() { return sizeof(rexp_t); }

p_rexp_t
rexp_new(char *buffer, char *reg_exp, int flags) {
	p_rexp_t pnew;

	/*
	 *  No since one can prepare the regexp before serialize searches!
	 *	if (!buffer) return NULL;
	 */

	if (!reg_exp) return NULL;
	if (!(pnew = (p_rexp_t) mem_malloc(sizeof(rexp_t)))) return NULL;
	if (!(pnew->pre = (regex_t *) mem_malloc(sizeof(regex_t)))) return rexp_free(pnew); 
	
	if (buffer) {
		if (!(pnew->buffer = mem_strdup(buffer))) return rexp_free(pnew);
		pnew->pbuf = pnew->last = pnew->buffer;
	} else  pnew->pbuf = pnew->last = pnew->buffer = NULL;

	if (!(pnew->regexp = mem_strdup(reg_exp))) return rexp_free(pnew); 
	pnew->nmatch = MAX_SUB;
	
	if ((regcomp(pnew->pre, pnew->regexp, flags))) {
		err_error("REXP error: bad regular expression.\n");
		return rexp_free(pnew); 
	}
	pnew->nsubs = 0;
	pnew->isub  = 0;
	return pnew;
}

char *
rexp_buffer_get(p_rexp_t prexp) {
	if (!prexp) return NULL;
	if (!prexp->buffer) return NULL;
	return mem_strdup(prexp->buffer);
}

p_rexp_t
rexp_buffer_set(p_rexp_t prexp, char *buffer) {
	if (!prexp) return NULL;
	if (prexp->buffer) mem_free(prexp->buffer);
	if (!(prexp->buffer = mem_strdup(buffer))) return rexp_free(prexp);
	/*prexp->buffer = buffer;*/
	prexp->pbuf = prexp->last = prexp->buffer;
	return prexp;
}

int
rexp_find(p_rexp_t prexp) {
	int flag = 0;
	if (!prexp) return 0;
	if (!prexp->buffer) return 0;
	if (!prexp->pre) return 0;
	if (!prexp->pbuf) prexp->pbuf = prexp->last = prexp->buffer;
	if (!prexp->last) prexp->last = prexp->buffer;
	if (prexp->pbuf >= prexp->buffer + strlen(prexp->buffer)) return 0;
	if (prexp->pbuf !=  prexp->buffer) 
		if (prexp->pbuf[-1] != '\n') flag = REG_NOTBOL;
	if ((flag = regexec(prexp->pre, prexp->pbuf, prexp->nmatch, prexp->a_match, flag))) {
		if (flag != REG_NOMATCH) err_error("REXP error: problem while executing regexp \"%s\".\n", prexp->regexp);
		return 0;
	}
	prexp->last = prexp->pbuf;
	prexp->pbuf += prexp->a_match[0].rm_eo;		
	return 1;
}

int
rexp_find_backwards(p_rexp_t prexp) {
	return 0;
}

int
rexp_count_match(p_rexp_t prexp) {
	int count = 0;

	if (!prexp) return 0;
	if (!prexp->buffer) return 0;
	while (rexp_find(prexp)) count++;
	return count;
}

size_t
rexp_nsubs(p_rexp_t prexp) {
	size_t count = 1;
	if (!prexp) return -1;
	if (!prexp->pre) return -2;
	if (prexp->pre->re_nsub > 0) {
		while (prexp->a_match[count].rm_so >= 0) {
			count++;
		}
	}
	return count;
}

char *
rexp_sub_get(p_rexp_t prexp, int index) {
	if (!prexp) return NULL;
	if (index < 0 || index >= MAX_SUB) return NULL;
	if (prexp->a_match[index].rm_so < 0) return NULL;
	return _rexp_extract_substr_(prexp->last, prexp->a_match[index].rm_so,  prexp->a_match[index].rm_eo);
}

char *
rexp_match_get(p_rexp_t prexp) {
	return rexp_sub_get(prexp, 0);
}

int
rexp_match_index_get(p_rexp_t prexp) {
	if (!prexp) return -2;
	if (prexp->a_match[0].rm_so < 0) return -1;
	return (prexp->last - prexp->buffer + prexp->a_match[0].rm_so);
}

int
rexp_replace_next(p_rexp_t prexp, char *string) {
	char *repl_str, *buff;
	char * tmp = NULL;
	pstr_t str = NULL;
	int pbuf_offset; //, last_offset;

	if (!prexp) return 0;
	if (!prexp->buffer) return 0;
	if (!string) return 0;

	/* find next occurence of regular expression: */
	if (!rexp_find(prexp)) /* not found ==> */ return 0;
	
	/* compute size of the new buffer : */

	//size_t ns;
	//ns  = strlen(prexp->buff) + strlen(string) - strlen(rexp_match_get);

	/* prepare replacement: */
	if (!(repl_str = _rexp_update_replacement_string_(prexp, string))) return 0;
	if (!(str = str_new(strlen(prexp->buffer)))) { mem_free(repl_str); return 0; }
		
	/* copy the buffer string until pointer if needed:  */ 
	if (prexp->last > prexp->buffer) {
		tmp =  _rexp_extract_substr_(prexp->buffer, 0, prexp->last - prexp->buffer);
		str_add_string(str, tmp);
		if (tmp) tmp = mem_free(tmp);	
	}
	
	/* copy the part of buffer between last and 1st match: */ 
	tmp = _rexp_extract_substr_(prexp->last, 0, prexp->a_match[0].rm_so);
	str_add_string(str, tmp);
	if (tmp) tmp = mem_free(tmp);	

	/* apend the replacement string (and free replacement string): */ 
	str_add_string(str, repl_str);
	
	/* compute offset for the new last pointer: */
//	last_offset = prexp->last - prexp->buffer;

	
	/* compute offset for the new pbuf pointer: */
	pbuf_offset = str_len(str);	
	
	/* apend the part of buffer after match end offset and buffer length: */ 
	tmp = _rexp_extract_substr_(prexp->last, prexp->a_match[0].rm_eo, strlen(prexp->last));
	str_add_string(str, tmp);
	if (tmp) tmp = mem_free(tmp);

	/* replace the old buffer by the new one: */
	buff = str_string(str);
	rexp_buffer_set(prexp, buff);
	mem_free(buff);

	/* recompute the last pointer: */
	/*prexp->last += last_offset;*/

	/* recompute the pbuf pointer: */
	prexp->pbuf += pbuf_offset;	
	prexp->last = prexp->pbuf;

	/* cleanup: */
	if (str) str_free(str);
	if (repl_str) repl_str = mem_free(repl_str);
	return 1;
}

int
rexp_replace_next_with_func(p_rexp_t prexp, char * (*repl_f)(p_rexp_t)) {
	char *repl_str, *buf = NULL;
	char * tmp = NULL;
	pstr_t str = NULL;
	int pbuf_offset; //, last_offset;
	char *string;

	if (!prexp) return 0;
	if (!prexp->buffer) return 0;

	/* find next occurence of regular expression: */
	if (!rexp_find(prexp)) /* not found ==> */ return 0;
	
	buf = repl_f(prexp);
	if (!buf) return 0;
	string = mem_strdup(buf);
	mem_free(buf);

	if (!string || !strlen(string)) return 0;

	/* prepare replacement: */
	if (!(repl_str = _rexp_update_replacement_string_(prexp, string))) return 0;
	if (!(str = str_new(strlen(prexp->buffer)))) { mem_free(repl_str); return 0; }
	mem_free(string);
		
	/* copy the buffer string until pointer if needed:  */ 
	if (prexp->last > prexp->buffer) {
		tmp = _rexp_extract_substr_(prexp->buffer, 0, prexp->last - prexp->buffer);
		str_add_string(str, tmp);
		if (tmp) tmp = mem_free(tmp);
	}
	
	/* copy the part of buffer between last and 1st match: */ 
	if (prexp->a_match[0].rm_so > 0) {
	 	tmp = _rexp_extract_substr_(prexp->last, 0, prexp->a_match[0].rm_so);
		str_add_string(str, tmp);
		if (tmp) tmp = mem_free(tmp);
	}

	/* apend the replacement string (and free replacement string): */ 
	str_add_string(str, repl_str);
	
	/* compute offset for the new last pointer: */
	//last_offset = prexp->last - prexp->buffer;
	
	/* compute offset for the new pbuf pointer: */
	pbuf_offset = str_len(str);	
	
	/* apend the part of buffer after match end offset and buffer length: */ 
	tmp = _rexp_extract_substr_(prexp->last, prexp->a_match[0].rm_eo, strlen(prexp->last)); 
	str_add_string(str, tmp);
	if (tmp) tmp = mem_free(tmp);
	
	/* replace the old buffer by the new one: */
	buf = str_string(str);
	rexp_buffer_set(prexp, buf);
	mem_free(buf);

	/* recompute the last pointer: */
	/*prexp->last += last_offset;*/

	/* recompute the pbuf pointer: */
	prexp->pbuf += pbuf_offset;	
	prexp->last = prexp->pbuf;


	/* cleanup: */
	if (str) str_free(str);
	if (repl_str) mem_free(repl_str);
	return 1;
}

int
rexp_replace_first(p_rexp_t prexp, char *string) {
	if (!prexp) return 0;
	if (!prexp->buffer) return 0;
	prexp->pbuf = prexp->buffer;
	return rexp_replace_next(prexp, string);
}

int
rexp_replace_all(p_rexp_t prexp, char *string) {
	int count = 0;
	while (rexp_replace_next(prexp, string)) count++;
	return count;
}

int
rexp_replace_all_with_func(p_rexp_t prexp, char * (*repl_f)(p_rexp_t)) {
	int count = 0;
	while (rexp_replace_next_with_func(prexp, repl_f)) count++;
	return count;
}

#if defined(_test_rexp_)
int 
main(void) {
#else
int rexp_test(void) {
#endif
	char *match;	
	char myre[] = "^[^=]+=(.*)";
	char buffer[] = "Base=http://google.com\n\
toto=titi\n\
tutu=!tata\n\
abc=abc\n\
def=test.test.test test\n";
	
	char P2re[] = "ab([0-9]+)cd([A-Z]*)ef";

	char buffer2[] = "\
1 ** ab0246cdABXef ** should match\n\
2 ** ab135cdef ** should match\n\
3 ** abcdefgh ** should not\n\
4 ** toto     ** should not\n\
0 01234567890123456789012345678901234567890123456789\n\
1 01234567890123456789012345678901234567890123456789\n\
2 01234567890123456789012345678901234567890123456789\n\
3 01234567890123456789012345678901234567890123456789\n\
4 01234567890123456789012345678901234567890123456789\n\
5 01234567890123456789012345678901234567890123456789\n\
6 01234567890123456789012345678901234567890123456789\n\
7 01234567890123456789012345678901234567890123456789\n\
8 01234567890123456789012345678901234567890123456789\n\
9 01234567890123456789012345678901234567890123456789\n\
0 01234567890123456789012345678901234567890123456789\n\
1 01234567890123456789012345678901234567890123456789\n\
2 01234567890123456789012345678901234567890123456789\n\
3 01234567890123456789012345678901234567890123456789\n\
4 01234567890123456789012345678901234567890123456789\n\
5 01234567890123456789012345678901234567890123456789\n\
6 01234567890123456789012345678901234567890123456789\n\
7 01234567890123456789012345678901234567890123456789\n\
8 01234567890123456789012345678901234567890123456789\n\
9 01234567890123456789012345678901234567890123456789\n\
0 01234567890123456789012345678901234567890123456789\n\
1 01234567890123456789012345678901234567890123456789\n\
2 01234567890123456789012345678901234567890123456789\n\
3 01234567890123456789012345678901234567890123456789\n\
4 01234567890123456789012345678901234567890123456789\n\
5 01234567890123456789012345678901234567890123456789\n\
6 01234567890123456789012345678901234567890123456789\n\
7 01234567890123456789012345678901234567890123456789\n\
8 01234567890123456789012345678901234567890123456789\n\
9 01234567890123456789012345678901234567890123456789\n\
0 01234567890123456789012345678901234567890123456789\n\
1 01234567890123456789012345678901234567890123456789\n\
2 01234567890123456789012345678901234567890123456789\n\
3 01234567890123456789012345678901234567890123456789\n\
4 01234567890123456789012345678901234567890123456789\n\
5 01234567890123456789012345678901234567890123456789\n\
6 01234567890123456789012345678901234567890123456789\n\
7 01234567890123456789012345678901234567890123456789\n\
8 01234567890123456789012345678901234567890123456789\n\
9 01234567890123456789012345678901234567890123456789\n\
0 01234567890123456789012345678901234567890123456789\n\
1 01234567890123456789012345678901234567890123456789\n\
2 01234567890123456789012345678901234567890123456789\n\
3 01234567890123456789012345678901234567890123456789\n\
4 01234567890123456789012345678901234567890123456789\n\
5 01234567890123456789012345678901234567890123456789\n\
6 01234567890123456789012345678901234567890123456789\n\
7 01234567890123456789012345678901234567890123456789\n\
8 01234567890123456789012345678901234567890123456789\n\
9 01234567890123456789012345678901234567890123456789\n\
";
	/*char replstr[] = "xx$2xx$3xx$1xx/orig = $0";*/
	char replstr[] = "xx$2xx$3xx$1xx/orig = $0/test = \\$88\\\nnl";
	char *s;

	int i, ns;
	p_rexp_t pre;
	
	printf("Part One: finding \n");
	printf("buffer = %s\n", buffer);
	printf("regexp = %s\n", myre);

	pre = rexp_new(buffer, myre, rexp_EXTENDED | rexp_NL_DO_NOT_MATCH);

	printf("%d matches found.\n", rexp_count_match(pre));

	rexp_buffer_set(pre, buffer);

	while ((rexp_find(pre))) {
		match = rexp_match_get(pre);
		printf("match @ %d = `%s'\n", rexp_match_index_get(pre), match);
		if ((ns = rexp_nsubs(pre))) {
			for (i = 1; i <= ns; i++) {
				if ((s = rexp_sub_get(pre, i))) {
					printf("\t%dth sub = `%s'\n", i,s);
					mem_free(s);
				}
			}
		}
		mem_free(match);
	}
	rexp_free(pre);

	printf("\nPart Two: replacing \n");
	pre = rexp_new(buffer2, P2re,  rexp_EXTENDED | rexp_NL_DO_NOT_MATCH);

	printf("buffer  = %s\n", buffer2);
	printf("regexp  = %s\n", P2re);
	printf("replstr = %s\n", replstr);

	printf("%d string replaced\n", rexp_replace_all(pre, replstr));
	printf("after:\n");
	
	s = rexp_buffer_get(pre);
	printf("buffer  = %s\n", s);
	if (s) mem_free(s);
	rexp_free(pre);
	return 0;
}
