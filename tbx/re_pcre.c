#include <string.h>

#include "err.h"
#include "pcre.h"

#define _re_pcre_c_
#include "re.h"
#include "re_pcre.h"
#undef  _re_prcre_c_

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	19/07/2016  1.0  creation
	04/08/2016  1.1  add changelog & tbx std tags
*/   

char re_pcre_MODULE[]  = "re_pcre";
char re_pcre_PURPOSE[] = "pcre re engine for re module";
char re_pcre_VERSION[] = "1.1";
char re_pcre_DATEVER[] = "04/08/2016";

enum {
	repcreNOERROR,
	repcreNOMATCH,
	repcreNULL,
	repcreBADOPTION,
	repcreBADMAGIC,
	repcreUNKNOWN_NODE,
	repcreNOMEMORY,
	repcreUNDOCUMENTED
};

static char *_re_pcre_exec_error_str[] = {
	"No error",
	"No match found",
	"Something was null", 
	"A bad option was passed",
	"Magic number bad (compiled re corrupt?)",
	"Something kooky in the compiled re",
	"No memory",
	"Unknown error",
	"Undocumented error code"
};

static char *
_re_pcre_match_err_str(int code) {
	if (code >= 0) return _re_pcre_exec_error_str[repcreNOERROR];
	switch(code) {
	case PCRE_ERROR_NOMATCH      : return _re_pcre_exec_error_str[repcreNOMATCH];
	case PCRE_ERROR_NULL         : return _re_pcre_exec_error_str[repcreNULL];
    case PCRE_ERROR_BADOPTION    : return _re_pcre_exec_error_str[repcreBADOPTION];
    case PCRE_ERROR_BADMAGIC     : return _re_pcre_exec_error_str[repcreBADMAGIC];
    case PCRE_ERROR_UNKNOWN_NODE : return _re_pcre_exec_error_str[repcreUNKNOWN_NODE];
    case PCRE_ERROR_NOMEMORY     : return _re_pcre_exec_error_str[repcreNOMEMORY];
    default                      : return _re_pcre_exec_error_str[repcreUNDOCUMENTED];
    }
}

static int
_re_pcre_flags(int flags) {
	int cflags = 0;
	if (flags & reIGNORECASE) cflags |= PCRE_CASELESS;
	if (flags & reNEWLINE)    cflags |= PCRE_MULTILINE;
	if (flags & reNOSUB)	  cflags |= PCRE_NO_AUTO_CAPTURE;
	if (flags & reUTF8)		  cflags |= PCRE_UTF8;
	if (flags & reDOTALL)	  cflags |= PCRE_DOTALL;
	if (flags & reEXTENDED)   cflags |= PCRE_EXTENDED;
	return cflags;
}

int
re_pcre_new(pre_t r) {
	if (!r) return 1;
	return 0;
}

int
re_pcre_compile(pre_t r, int flags) {
	char *errstr;
	int errnum;

	if (!r) return 1;
	if (!(r->r = (void *) pcre_compile(r->rexp, _re_pcre_flags(flags), (const char **) &errstr, &errnum,  NULL))) {
		err_error("RE error: bad regular expression (%s)", errstr); 
		return 2;
	}
	return 0;
}

void *
re_pcre_free(pre_t r) {
	if (r && r->r) {
		pcre_free((pcre *) r->r);
		r->r = NULL;
	}	
	return NULL;	
}

prematch_t
re_pcre_match(pre_t r, char *p) {
	int e;
	int i, n;
	int retvect[RE_MAX_SUB * 2 + 1];
	prematch_t m;
	
	if (!r || !r->r || !p) return NULL;

	memset(retvect, -1, (RE_MAX_SUB * 2 + 1) * sizeof(int));

	if ((e = pcre_exec(r->r, NULL, p, strlen(p), 0, 0, retvect, RE_MAX_SUB * 2)) < 0) {
		if (e != PCRE_ERROR_NOMATCH) {	
			err_error("RE error: %s", _re_pcre_match_err_str(e));
		}
		return NULL;
	}
		
	for (n = 0; n < RE_MAX_SUB * 2 && retvect[n] >= 0; n++);
	n = (n + 1) / 2;

	if (n < 1) return NULL;

	if (!(m = rematch_new(n))) {
		err_error("cannot allocate rematch result");
		return NULL;
	}
	
	for (i = 0; i < n; i++) {
		m->subs[i].so = retvect[2 * i    ];
		m->subs[i].eo = retvect[2 * i + 1];
	}		

	return m;
} 
