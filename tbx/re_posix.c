#include <regex.h>

#include "err.h"
#include "mem.h"

#define _re_posix_c_
#include "re_posix.h"
#include "re.h"
#undef  _re_posix_c_

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	19/07/2016  1.0  creation (from re.c v 1.x)
	04/08/2016  1.1  add changelog & tbx std tags
*/   

char re_posix_MODULE[]  = "re_posix";
char re_posix_PURPOSE[] = "posix re engine for re module";
char re_posix_VERSION[] = "1.1";
char re_posix_DATEVER[] = "04/08/2016";

static int
_re_posix_flags(int flags) {
	int pflags = RE_CHAR_CLASSES|REG_EXTENDED;

	if (flags & reIGNORECASE) pflags |= RE_ICASE;
	if (flags & reNEWLINE)    pflags |= REG_NEWLINE;
	if (flags & reNOSUB)      pflags |= RE_NO_SUB;
	if (flags & reDOTALL)     pflags |= RE_DOT_NEWLINE;

	return pflags;
}

int
re_posix_new(pre_t r) {
	if (!r) return 1;
	if (!(r->r = mem_zmalloc(sizeof(regex_t)))) return 2;
	return 0;
}

int
re_posix_compile(pre_t r, int flags) {
	int e;

	if (!r) return 1;

	if ((e = regcomp(r->r, r->rexp, _re_posix_flags(flags)))) {
		char eb[1000];
		regerror(e, r->r, eb, 1000);
		err_error("RE error: bad regular expression (%s)", eb); 
		return 2;
	}
	return 0;
}

void *
re_posix_free(pre_t r) {
	if (r && r->r) {
		regfree((regex_t *) r->r);
		free(r->r);
		r->r = NULL;
	}
	return NULL;	
}

prematch_t
re_posix_match(pre_t r, char *p) {
	int e, i, n;
	prematch_t m;
	regmatch_t rm[RE_MAX_SUB];

	if (!r || !r->r || !p) return NULL;

	if ((e = regexec(r->r, p, RE_MAX_SUB, rm, 0))) {
		if (e != REG_NOMATCH) err_error("RE error: problem while executing regexp (%s)", r->r);
		return NULL;
	}

	for (n = 0; n < RE_MAX_SUB && rm[n].rm_so >= 0; n++);

	if (n < 1) return NULL;

	if (!(m = rematch_new(n))) {
		err_error("cannot allocate rematch result");
		return NULL;
	}

	for (i = 0; i < n; i++) {
		m->subs[i].so = rm[i].rm_so;
		m->subs[i].eo = rm[i].rm_eo;
	}		

	return m;
}	
