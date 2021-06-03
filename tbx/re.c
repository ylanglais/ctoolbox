#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "err.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	19/07/2016  2.0  use pcre2 or pcre or stdlib
	04/08/2016  2.1  add changelog 
*/   

char re_MODULE[]  = "Simple regular expression encapsulation";
char re_PURPOSE[] = "Simple regular expression encapsulation w/ posix or pcre 1 or 2 engines";
char re_VERSION[] = "2.1";
char re_DATEVER[] = "04/08/2016";

struct _rematch_t;

#define _re_c_
#include "re.h"
#undef  _re_c_

#include "re_posix.h"
#include "re_pcre.h"
#include "re_pcre2.h"

#define reopAMPERSAND 1
#define reopSUB       2

#if 0
static char *re_engines[] = {
	"posix",
#ifdef HASPCRE2
	"pcre",
#endif
#ifdef HASPCRE2
	"pcre2",
#endif
	""
};
#endif

enum {
	rePOSIX,
#ifdef HASPCRE1
	rePCRE,
#endif 
#ifdef HASPCRE2
	rePCRE2,
#endif
};

prematch_t 
rematch_new(int n) {
	prematch_t m;
	if (!(m = (prematch_t)       mem_zmalloc(sizeof(rematch_t))))   return NULL;	
	if (!(m->subs = (presub_t)   mem_zmalloc(n * sizeof(resub_t)))) return rematch_destroy(m);
	m->nsubs = n;
	return m;
}

prematch_t 
rematch_destroy(prematch_t m) {
	if (m) {
		if (m->subs) m->subs = mem_free(m->subs);
		m = mem_free(m);
	}
	return NULL;
}

/* Convert regmatch structure into an array of at most RE_MAX_SUB matching sub strings: */
char **
remstrs_new(pre_t r, prematch_t m) {
	int i;
	char **s;
	if (!r) return NULL;
	if (!m) return NULL;
	if (!m->nsubs) return NULL;
	if (!(s = (char **) mem_zmalloc(RE_MAX_SUB * sizeof(char *)))) return NULL;
	for (i = 0; i < m->nsubs; i++) {
		int j;
		if (!(s[i] = (char *) mem_zmalloc(m->subs[i].eo - m->subs[i].so + 1))) return remstrs_destroy(s);
		for (j = m->subs[i].so; i <= m->subs[i].eo; j++) s[i][j - m->subs[i].so] = r->buffer[j];
	}
	return s;
}

/* Destroy matching substrings array: */
char **
remstrs_destroy(char **s) {
	int i;
	if (s) {
		for (i = 0; i < RE_MAX_SUB; i++) {
			if (s[i]) mem_free(s[i]);
		}
	}
	return NULL;
}

size_t re_sizeof() { return sizeof(re_t); }

pre_t
re_new(char *buffer, char *rexp, int flags) {
	pre_t r;
	if (!rexp || !*rexp) return NULL;
	if (!(r = (pre_t) mem_zmalloc(re_sizeof()))) return NULL;

#if defined(HASPCRE2)
	r->re_mod_new     = re_pcre2_new;
	r->re_mod_free    = re_pcre2_free;
	r->re_mod_compile = re_pcre2_compile;
	r->re_mod_match   = re_pcre2_match;
#elif defined(HASPCRE1) 
	r->re_mod_new     = re_pcre_new;
	r->re_mod_free    = re_pcre_free;
	r->re_mod_compile = re_pcre_compile;
	r->re_mod_match   = re_pcre_match;
#else 
	r->re_mod_new     = re_posix_new;
	r->re_mod_free    = re_posix_free;
	r->re_mod_compile = re_posix_compile;
	r->re_mod_match   = re_posix_match;
#endif

	r->re_mod_new(r);
	r->rexp  = mem_strdup(rexp);
	r->flags = flags;
	
	if (buffer && *buffer) {
		r->buffer = mem_strdup(buffer);
	}

	if (r->re_mod_compile(r, flags)) {
		return re_destroy(r); 
	}
	return r;
}

pre_t
re_destroy(pre_t r) {
	if (r) {
		if (r->buffer) r->buffer = mem_free(r->buffer);
		if (r->rexp)   r->rexp   = mem_free(r->rexp);
		r->re_mod_free(r);
		mem_free(r);
	}
	return NULL;
}

char *
re_buffer(pre_t r) {
	if (!r) return NULL;
	return r->buffer;
}

char *
re_substr(pre_t r, int from, int to) {
	int i;
	char *s;
	if (!r) return NULL;
	if (from < 0) return NULL;
	if (from > strlen(r->buffer)) return NULL;
	if (to < 0)   return NULL;
	if (to > strlen(r->buffer)) to = strlen(r->buffer) - 1;
	if (from > to) return NULL;
	if (!(s = mem_zmalloc(to - from + 1))) return NULL;
	for (i = from; i < to; i++) {
		s[i - from] = r->buffer[i];
	}
	return s;
}

prematch_t 
re_find(pre_t r, char *p) {
	if (!r) return NULL;
	if (!p) return NULL;

	return r->re_mod_match(r, p);
}

prematch_t
re_find_first(pre_t r) {
	if (!r)         return NULL;
	if (!r->buffer) return NULL;

	return re_find(r, r->buffer);
}

prematch_t
re_find_next(pre_t r, char *ptr) {
	prematch_t m;
	if (!r)                                  return NULL;
	if (!ptr)                                return NULL;
	if (!r->buffer)                          return NULL;
	if (ptr < r->buffer)                     return NULL;
	if (ptr > r->buffer + strlen(r->buffer)) return NULL;
	
	int i, offset;
	offset = (ptr - r->buffer);
	m = re_find(r, ptr);
	if (!m) return NULL;

	for (i = 0; i < m->nsubs; i++) {
		//printf("so : %d, eo %d, offset %d\n", m->subs[i].so, m->subs[i].eo, offset);
		m->subs[i].so += offset;
		m->subs[i].eo += offset;
	}
	return m;
}

int
re_find_foreach(pre_t r, void *user_data, re_match_processor_f process) {
	int         i = 0;
	char       *p;
	prematch_t  m;
	
	if (!r)         return -1;
	if (!r->buffer) return -2;

	p = r->buffer; 
	while ((m = re_find_next(r, p))) {
		if (process) process(r, user_data, m);
		p = r->buffer + m->subs[0].eo;
		//printf("///%s\n", p);
		m = rematch_destroy(m);
		i++;
	}
	return i;	
}

int re_match_count(pre_t r) {
	return  re_find_foreach(r, NULL, NULL);
}

char *
re_subst_str(pre_t r, prematch_t m, char *tmpl) {
	char *p, *q, *str;
	int s;

	if (!r)        return NULL;
	if (!m)        return NULL;
	if (!m->nsubs) return NULL;
	if (!tmpl)     return NULL;

	/* 1st pass: compute size of dest string: */
	p = tmpl;
	s = 0;
	while (*p) {
		if (*p == '&' && (p == tmpl || p[-1] != '\\')) {
			s += m->subs[0].eo - m->subs[0].so;
		} else if (*p == '\\' && p[1] >= '0' && p[1] <= '9') {
			if ((p[1] - '0') >= 0 || (p[1] -'0') < m->nsubs) {
				s += m->subs[p[1] -'0'].eo - m->subs[p[1] -'0'].so;
			}
			p++;
		} else {
			s++;
		}
		p++;
	}
	s++;

	/* Reserve mem and actually forge dest string: */
	if (!(str = mem_zmalloc(s))) return NULL;
	p = tmpl;
	q = str;
	while (*p) {
		int i;
		if (*p == '&' && (p == tmpl || p[-1] != '\\')) {
			for (i = m->subs[0].so; i < m->subs[0].eo; i++) {
				*q++ = r->buffer[i];
			}
		} else if (*p == '\\' && p[1] >= '0' && p[1] <= '9') {
			if ((p[1] - '0') >= 0 || (p[1] -'0') < m->nsubs) {
				for (i = m->subs[p[1]-'0'].so; i < m->subs[p[1]-'0'].eo; i++) {
					*q++ = r->buffer[i];
				}
			}
			p++;
		} else {
			*q++ = *p;
		}
		p++;
	}
	return str;
}

char *
re_replace_next(pre_t r, char *from, char *tmpl) {
	prematch_t m;

	if (!r)            return NULL;
	if (!tmpl) return NULL;
	
	if (((m = re_find_next(r, from)) && m->nsubs > 0)) {
		char *dst, *repl;
		int i, j, b_len, r_len, delta, offset;

		repl = re_subst_str(r, m, tmpl);

		b_len = strlen(r->buffer);
		r_len = strlen(repl);

		delta = r_len + m->subs[0].so - m->subs[0].eo;
		offset = m->subs[0].eo + delta;

		if (!(dst = mem_zmalloc(b_len + delta + 1))) return NULL;

		/* Copy from start to matching string: */
		for (i = 0; i < m->subs[0].so; i++) dst[i] = r->buffer[i];

		/* Apend replacement: */ 
		for (j = 0; j < r_len; j++) dst[i + j] = repl[j];

		/* Then apend what remains: */
		for (i = m->subs[0].eo; i < b_len; i++) dst[i + delta] = r->buffer[i];

		m = rematch_destroy(m);
		mem_free(r->buffer);
		mem_free(repl);
		r->buffer = dst;

		return r->buffer + offset;
	}
	return NULL;

}

char *
re_replace_first(pre_t r, char *tmpl) {
	if (!r)    return 0;
	if (!tmpl) return 0;
	return re_replace_next(r, r->buffer, tmpl); 
}

int
re_replace_all(pre_t r, char *tmpl) {
	char *p;
	int i;
	if (!r)    return 0;
	if (!tmpl) return 0;
	p = r->buffer; 
	for (i = 0; (p = re_replace_next(r, p, tmpl)); i++);
	
	return i;
}

#ifdef _test_re_
#include <stdio.h>

int main(void) {
	static char b[] = "\
0 http://www.google.com/d/index.html?aa=oo&ti=ta&u=1\n\
0 http://www.google.com/d/e/f/g/index.html          \n\
1 ** ab0246cdABXef                                  \n\
2 ** ab135cdef                                      \n\
3 ** abcdefgh                                       \n\
4 ** toto totototo toto  toto                       \n\
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

	pre_t r;
	int i;

	char *re1[] = {
		"a", 
		"toto", 
		"[aA]",
		"^[0-9] ", 
		"^[0-9] ", 
		"^\\([0-9]\\) ", 
		"^[0-9] [0-9]*$",
		"^[0-9] [0-9]{50}$",
		"^[0-9] [0-9]\\{50\\}$", 
		""
	};



	printf("\n------------------------\nPart 1: finding (NEWLINE)\n");
	printf("buffer = %s\n", b);

	for (i = 0; strlen(re1[i]) > 0; i++) {
		printf("regexp = %s\n", re1[i]);
		r = re_new(b, re1[i], reNEWLINE);
		printf("%d matches found.\n", re_match_count(r));
		r = re_destroy(r);
	}

	printf("\n------------------------\nPart 2: exploring submatch (NEWLINE)\n");
	char *re2[] = {
		"toto",
		"(toto)",
		"\\(toto\\)",
		"\\(to\\)to",
		"http://.*$",
		"http[s]\\{0,1\\}://\\(.*\\)$",
		"\\(http[s]\\{0,1\\}\\)://\\(.*\\)$",
		"\\(http[s]\\{0,1\\}\\)://\\([^/]*\\)/\\(.*\\)/\\([^?]*\\)?\\{0,1\\}\\(.*\\)$",
		"\\(http[s]\\{0,1\\}\\)://\\([^/]*\\)/\\(.*\\)/\\([^?]*\\)?\\(.*\\)$",
		""
	};
	for (i = 0; strlen(re2[i]) > 0; i++) {
		prematch_t m;

		printf("\nregexp = %s\n", re2[i]);
		r = re_new(b, re2[i], reNEWLINE);

		if ((m = re_find_first(r))) {
			int j;
			char *s;
			
			for (j = 0; j < m->nsubs; j++) {
				printf("sub[%d] (from %d to %d) = %s\n", j, m->subs[j].so, m->subs[j].eo, s = re_substr(r, m->subs[j].so, m->subs[j].eo)); 
				if (s) free(s);
			}
			m = rematch_destroy(m);
		} else { printf("0 match\n"); }

		r = re_destroy(r);
	}

	printf("\n------------------------\nPart 3: exploring submatch (NEWLINE + EXTENDED)\n");
	char *re3[] = {
		"toto",
		"(toto)",
		"\\(toto\\)",
		"(to)to",
		"(tot|tat)",
		"http://.*$",
		"http[s]{0,1}://(.*)$",
		"(http[s]{0,1})://(.*)$",
		"(http[s]{0,1})://([^/]*)/(.*)/([^?]*)\\?(.*)$",
		"(https|http)://([^/]*)/(.*)/([^?]*)\\?(.*)$",
		""
	};
	for (i = 0; strlen(re3[i]) > 0; i++) {
		prematch_t m;

		printf("\nregexp = %s\n", re3[i]);
		r = re_new(b, re3[i], reEXTENDED|reNEWLINE);

		if ((m = re_find_first(r))) {
			int j;
			char *s;
			
			for (j = 0; j < m->nsubs; j++) {
				printf("sub[%d] (from %d to %d) = %s\n", j, m->subs[j].so, m->subs[j].eo, s = re_substr(r, m->subs[j].so, m->subs[j].eo)); 
				if (s) free(s);
			}
			m = rematch_destroy(m);
		} else { printf("0 match\n"); }
		r = re_destroy(r);
	}

	printf("\n------------------------\nPart 4: replacing \n");
	char *re4[]     = {
		"toto",
		"t[oi]t[oi]",
		"\\(t[oi]\\)\\1",
		"toto",
		"ab.*cdef",
		"ab\\([0-9]*\\)\\{0,1\\}cdef",
		""
	};
	char *subs4[]  = {
		"titi",
		"tata",
		"t\\1a",
		"tt",
		" & --> &",
		"& --> \\1",
		""
	};
	for (i = 0; strlen(re4[i]) && strlen(subs4[i]); i++) {
		printf("regexp = %s\n",     re4[i]);
		printf("replac = \"%s\"\n", subs4[i]);
		r = re_new(b, re4[i], reNEWLINE);
		int c;
		c = re_replace_all(r, subs4[i]);
		if (!c) { printf("no replacement\n"); } else {
			printf("%d repacements\n", c);
			printf("new buff = \n%s\n", re_buffer(r));
		}
		
		r = re_destroy(r);
	}
	return 0;
}
#endif
