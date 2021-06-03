#include <string.h>

#include "err.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#define _re_pcre2_c_
#include "re.h"
#include "re_pcre2.h"
#undef  _re_prcre_c_

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	19/07/2016  1.0  creation
	04/08/2016  1.1  add changelog & tbx std tags
*/   

char re_pcre2_MODULE[]  = "re_pcre2";
char re_pcre2_PURPOSE[] = "pcre2 re engine for re module";
char re_pcre2_VERSION[] = "1.1";
char re_pcre2_DATEVER[] = "04/08/2016";

enum {
	repcre2NOERROR,
	repcre2NOMATCH,
	repcre2PARTIAL,
	repcre2UTF8_ERR1,
	repcre2UTF8_ERR2,
	repcre2UTF8_ERR3,
	repcre2UTF8_ERR4,
	repcre2UTF8_ERR5,
	repcre2UTF8_ERR6,
	repcre2UTF8_ERR7,
	repcre2UTF8_ERR8,
	repcre2UTF8_ERR9,
	repcre2UTF8_ERR10,
	repcre2UTF8_ERR11,
	repcre2UTF8_ERR12,
	repcre2UTF8_ERR13,
	repcre2UTF8_ERR14,
	repcre2UTF8_ERR15,
	repcre2UTF8_ERR16,
	repcre2UTF8_ERR17,
	repcre2UTF8_ERR18,
	repcre2UTF8_ERR19,
	repcre2UTF8_ERR20,
	repcre2UTF8_ERR21,
	repcre2UTF16_ERR1,
	repcre2UTF16_ERR2,
	repcre2UTF16_ERR3,
	repcre2UTF32_ERR1,
	repcre2UTF32_ERR2,
	repcre2BADDATA,
	repcre2MIXEDTABLES,
	repcre2BADMAGIC,
	repcre2BADMODE,
	repcre2BADOFFSET,
	repcre2BADOPTION,
	repcre2BADREPLACEMENT,
	repcre2BADUTFOFFSET,
	repcre2CALLOUT,
	repcre2DFA_BADRESTART,
	repcre2DFA_RECURSE,
	repcre2DFA_UCOND,
	repcre2DFA_UFUNC,
	repcre2DFA_UITEM,
	repcre2DFA_WSSIZE,
	repcre2INTERNAL,
	repcre2JIT_BADOPTION,
	repcre2JIT_STACKLIMIT,
	repcre2MATCHLIMIT,
	repcre2NOMEMORY,
	repcre2NOSUBSTRING,
	repcre2NOUNIQUESUBSTRING,
	repcre2NULL,
	repcre2RECURSELOOP,
	repcre2RECURSIONLIMIT,
	repcre2UNAVAILABLE,
	repcre2UNSET,
	repcre2BADOFFSETLIMIT,
	repcre2BADREPESCAPE,
	repcre2REPMISSINGBRACE,
	repcre2BADSUBSTITUTION,
	repcre2BADSUBSPATTERN,
	repcre2TOOMANYREPLACE,
	repcre2UNDOCUMENTED
};

static char *_re_pcre2_exec_error_str[] = {
	"No error",
    "No match", 	                /* NOMATCH           */
    "Partial", 		                /* PARTIAL           */
    "utf8 err1", 	                /* UTF8_ERR1         */
    "utf8 err2", 	                /* UTF8_ERR2         */
    "utf8 err3", 	                /* UTF8_ERR3         */
    "utf8 err4", 	                /* UTF8_ERR4         */
    "utf8 err5", 	                /* UTF8_ERR5         */
    "utf8 err6", 	                /* UTF8_ERR6         */
    "utf8 err7", 	                /* UTF8_ERR7         */
    "utf8 err8",                    /* UTF8_ERR8         */
    "utf8 err9",                    /* UTF8_ERR9         */
    "utf8 err10",                   /* UTF8_ERR10        */
    "utf8 err11",                   /* UTF8_ERR11        */
    "utf8 err12",                   /* UTF8_ERR12        */
    "utf8 err13",                   /* UTF8_ERR13        */
    "utf8 err14",                   /* UTF8_ERR14        */
    "utf8 err15",                   /* UTF8_ERR15        */
    "utf8 err16",                   /* UTF8_ERR16        */
    "utf8 err17",                   /* UTF8_ERR17        */
    "utf8 err18",                   /* UTF8_ERR18        */
    "utf8 err19",                   /* UTF8_ERR19        */
    "utf8 err20",                   /* UTF8_ERR20        */
    "utf8 err21",                   /* UTF8_ERR21        */
    "utf16 err1",                   /* UTF16_ERR1        */
    "utf16 err2",                   /* UTF16_ERR2        */
    "utf16 err3",                   /* UTF16_ERR3        */
    "utf32 err1",                   /* UTF32_ERR1        */
    "utf32 err2",                   /* UTF32_ERR2        */
    "Bad data",                     /* BADDATA           */
    "mixed tables",                 /* MIXEDTABLES       */
    "Bad magic",                    /* BADMAGIC          */
    "Bad mode",                     /* BADMODE           */
    "Bad offset",                   /* BADOFFSET         */
    "Bad option",                   /* BADOPTION         */
    "Bad replacement",              /* BADREPLACEMENT    */
    "Bad utf offset",               /* BADUTFOFFSET      */
    "Call out",                     /* CALLOUT           */
    "DFA bad restart",              /* DFA_BADRESTART    */
    "DFA recurse",                  /* DFA_RECURSE       */
    "DFA UCOND",                    /* DFA_UCOND         */
    "DFA UFUNC",                    /* DFA_UFUNC         */
    "DFA UITEM",                    /* DFA_UITEM         */
    "DFA WSSIZE",                   /* DFA_WSSIZE        */
    "Internal error",               /* INTERNAL          */
    "JIT bad option",               /* JIT_BADOPTION     */
    "JIT stack limit",              /* JIT_STACKLIMIT    */
    "Match limit",                  /* MATCHLIMIT        */
    "No memory",                    /* NOMEMORY          */
    "No substring",                 /* NOSUBSTRING       */
    "No unique substring",          /* NOUNIQUESUBSTRING */
    "NULL", 			            /* NULL              */
    "Recursive loop",               /* RECURSELOOP       */
    "Recursion limit",              /* RECURSIONLIMIT    */
    "Unavailable",                  /* UNAVAILABLE       */
    "Unset",                        /* UNSET             */
    "Bad offset limit",             /* BADOFFSETLIMIT    */
    "Bad repmacement escape",       /* BADREPESCAPE      */
    "Missing brace in replacement", /* REPMISSINGBRACE   */
    "Bad substitution",             /* BADSUBSTITUTION   */
    "Bad subpattern",               /* BADSUBSPATTERN    */
    "Too many replacement",         /* TOOMANYREPLACE    */
	"Undocumented error code"
};

static char *
_re_pcre2_match_err_str(int code) {
	if (code >= 0) return _re_pcre2_exec_error_str[repcre2NOERROR];
	switch(code) {
	case PCRE2_ERROR_NOMATCH :           return _re_pcre2_exec_error_str[repcre2NOMATCH];
    case PCRE2_ERROR_PARTIAL :           return _re_pcre2_exec_error_str[repcre2PARTIAL];
    case PCRE2_ERROR_UTF8_ERR1 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR1];
    case PCRE2_ERROR_UTF8_ERR2 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR2];
    case PCRE2_ERROR_UTF8_ERR3 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR3];
    case PCRE2_ERROR_UTF8_ERR4 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR4];
    case PCRE2_ERROR_UTF8_ERR5 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR5];
    case PCRE2_ERROR_UTF8_ERR6 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR6];
    case PCRE2_ERROR_UTF8_ERR7 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR7];
    case PCRE2_ERROR_UTF8_ERR8 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR8];
    case PCRE2_ERROR_UTF8_ERR9 :         return _re_pcre2_exec_error_str[repcre2UTF8_ERR9];
    case PCRE2_ERROR_UTF8_ERR10 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR10];
    case PCRE2_ERROR_UTF8_ERR11 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR11];
    case PCRE2_ERROR_UTF8_ERR12 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR12];
    case PCRE2_ERROR_UTF8_ERR13 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR13];
    case PCRE2_ERROR_UTF8_ERR14 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR14];
    case PCRE2_ERROR_UTF8_ERR15 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR15];
    case PCRE2_ERROR_UTF8_ERR16 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR16];
    case PCRE2_ERROR_UTF8_ERR17 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR17];
    case PCRE2_ERROR_UTF8_ERR18 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR18];
    case PCRE2_ERROR_UTF8_ERR19 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR19];
    case PCRE2_ERROR_UTF8_ERR20 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR20];
    case PCRE2_ERROR_UTF8_ERR21 :        return _re_pcre2_exec_error_str[repcre2UTF8_ERR21];
    case PCRE2_ERROR_UTF16_ERR1 :        return _re_pcre2_exec_error_str[repcre2UTF16_ERR1];
    case PCRE2_ERROR_UTF16_ERR2 :        return _re_pcre2_exec_error_str[repcre2UTF16_ERR2];
    case PCRE2_ERROR_UTF16_ERR3 :        return _re_pcre2_exec_error_str[repcre2UTF16_ERR3];
    case PCRE2_ERROR_UTF32_ERR1 :        return _re_pcre2_exec_error_str[repcre2UTF32_ERR1];
    case PCRE2_ERROR_UTF32_ERR2 :        return _re_pcre2_exec_error_str[repcre2UTF32_ERR2];
    case PCRE2_ERROR_BADDATA :           return _re_pcre2_exec_error_str[repcre2BADDATA];
    case PCRE2_ERROR_MIXEDTABLES :       return _re_pcre2_exec_error_str[repcre2MIXEDTABLES];
    case PCRE2_ERROR_BADMAGIC :          return _re_pcre2_exec_error_str[repcre2BADMAGIC];
    case PCRE2_ERROR_BADMODE :           return _re_pcre2_exec_error_str[repcre2BADMODE];
    case PCRE2_ERROR_BADOFFSET :         return _re_pcre2_exec_error_str[repcre2BADOFFSET];
    case PCRE2_ERROR_BADOPTION :         return _re_pcre2_exec_error_str[repcre2BADOPTION];
    case PCRE2_ERROR_BADREPLACEMENT :    return _re_pcre2_exec_error_str[repcre2BADREPLACEMENT];
    case PCRE2_ERROR_BADUTFOFFSET :      return _re_pcre2_exec_error_str[repcre2BADUTFOFFSET];
    case PCRE2_ERROR_CALLOUT :           return _re_pcre2_exec_error_str[repcre2CALLOUT];
    case PCRE2_ERROR_DFA_BADRESTART :    return _re_pcre2_exec_error_str[repcre2DFA_BADRESTART];
    case PCRE2_ERROR_DFA_RECURSE :       return _re_pcre2_exec_error_str[repcre2DFA_RECURSE];
    case PCRE2_ERROR_DFA_UCOND :         return _re_pcre2_exec_error_str[repcre2DFA_UCOND];
    case PCRE2_ERROR_DFA_UFUNC :         return _re_pcre2_exec_error_str[repcre2DFA_UFUNC];
    case PCRE2_ERROR_DFA_UITEM :         return _re_pcre2_exec_error_str[repcre2DFA_UITEM];
    case PCRE2_ERROR_DFA_WSSIZE :        return _re_pcre2_exec_error_str[repcre2DFA_WSSIZE];
    case PCRE2_ERROR_INTERNAL :          return _re_pcre2_exec_error_str[repcre2INTERNAL];
    case PCRE2_ERROR_JIT_BADOPTION :     return _re_pcre2_exec_error_str[repcre2JIT_BADOPTION];
    case PCRE2_ERROR_JIT_STACKLIMIT :    return _re_pcre2_exec_error_str[repcre2JIT_STACKLIMIT];
    case PCRE2_ERROR_MATCHLIMIT :        return _re_pcre2_exec_error_str[repcre2MATCHLIMIT];
    case PCRE2_ERROR_NOMEMORY :          return _re_pcre2_exec_error_str[repcre2NOMEMORY];
    case PCRE2_ERROR_NOSUBSTRING :       return _re_pcre2_exec_error_str[repcre2NOSUBSTRING];
    case PCRE2_ERROR_NOUNIQUESUBSTRING : return _re_pcre2_exec_error_str[repcre2NOUNIQUESUBSTRING];
    case PCRE2_ERROR_NULL :              return _re_pcre2_exec_error_str[repcre2NULL];
    case PCRE2_ERROR_RECURSELOOP :       return _re_pcre2_exec_error_str[repcre2RECURSELOOP];
    case PCRE2_ERROR_RECURSIONLIMIT :    return _re_pcre2_exec_error_str[repcre2RECURSIONLIMIT];
    case PCRE2_ERROR_UNAVAILABLE :       return _re_pcre2_exec_error_str[repcre2UNAVAILABLE];
    case PCRE2_ERROR_UNSET :             return _re_pcre2_exec_error_str[repcre2UNSET];
    case PCRE2_ERROR_BADOFFSETLIMIT :    return _re_pcre2_exec_error_str[repcre2BADOFFSETLIMIT];
    case PCRE2_ERROR_BADREPESCAPE :      return _re_pcre2_exec_error_str[repcre2BADREPESCAPE];
    case PCRE2_ERROR_REPMISSINGBRACE :   return _re_pcre2_exec_error_str[repcre2REPMISSINGBRACE];
    case PCRE2_ERROR_BADSUBSTITUTION :   return _re_pcre2_exec_error_str[repcre2BADSUBSTITUTION];
    case PCRE2_ERROR_BADSUBSPATTERN :    return _re_pcre2_exec_error_str[repcre2BADSUBSPATTERN];
    case PCRE2_ERROR_TOOMANYREPLACE :    return _re_pcre2_exec_error_str[repcre2TOOMANYREPLACE];
    default :                            return _re_pcre2_exec_error_str[repcre2UNDOCUMENTED];
    }
}

static int
_re_pcre2_flags(int flags) {
	int cflags = 0;
	if (flags & reIGNORECASE) cflags |= PCRE2_CASELESS;
	if (flags & reNEWLINE)    cflags |= PCRE2_MULTILINE;
	if (flags & reNOSUB)	  cflags |= PCRE2_NO_AUTO_CAPTURE;
	if (flags & reDOTALL)	  cflags |= PCRE2_DOTALL;
	if (flags & reEXTENDED)   cflags |= PCRE2_EXTENDED;
	return cflags;
}

int
re_pcre2_new(pre_t r) {
	if (!r) return 1;
	return 0;
}

int
re_pcre2_compile(pre_t r, int flags) {
	int    errnum;
	size_t erroffset;

	if (!r) return 1;

	if (!(r->r = (void *) pcre2_compile((PCRE2_SPTR) r->rexp, PCRE2_ZERO_TERMINATED, _re_pcre2_flags(flags), &errnum,  &erroffset, NULL))) {
		err_error("RE error: %s at offset %d", _re_pcre2_match_err_str(errnum), erroffset); 
		return 2;
	}
	return 0;
}

void *
re_pcre2_free(pre_t r) {
	if (r && r->r) {
		pcre2_code_free(r->r);
		r->r = NULL;
	}	
	return NULL;	
}

prematch_t
re_pcre2_match(pre_t r, char *p) {
	int i, n;
	pcre2_match_data *md;
	prematch_t m;
	size_t *vect;

	if (!r || !r->r || !p) return NULL;

	if (!(md = pcre2_match_data_create(RE_MAX_SUB + 1, NULL))) return NULL;

	if ((n = pcre2_match((const pcre2_code *) r->r, (PCRE2_SPTR) p, strlen(p), 0, 0, md, NULL)) < 0) {
		if (n != PCRE2_ERROR_NOMATCH) {	
			err_error("RE error: %s", _re_pcre2_match_err_str(n));
		}
		return NULL;
	}

	if (!(vect = pcre2_get_ovector_pointer(md))) {
		pcre2_match_data_free(md);
		return NULL;
	}

	if (!(m = rematch_new(n))) {
		err_error("cannot allocate rematch result");
		pcre2_match_data_free(md);
		return NULL;
	}
	
	for (i = 0; i < n && i < RE_MAX_SUB; i++) {
		m->subs[i].so = vect[2 * i    ];
		m->subs[i].eo = vect[2 * i + 1];
	}		

	pcre2_match_data_free(md);
	return m;
} 
