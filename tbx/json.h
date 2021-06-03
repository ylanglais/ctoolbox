#ifndef _json_h_
#define _json_h_

#define jsonFALSE 0
#define jsonTRUE  1
#define jsonNULL NULL

#include <tbx/tree.h>

typedef enum {
	json_NULL = 0,
	json_FALSE,
	json_TRUE,
	json_E,
	json_DIGIT,
	json_DIGITS,
	json_19DIGIT,	
	json_19DIGITS,	
	json_HEXDIGIT,
	json_HEXDIGITS,
	json_EXP,
	json_FRAC,
	json_INT,
	json_NUMBER,
	json_CHAR,
	json_CHARS,
	json_STRING,
	json_ARRAY,
	json_OBJECT,
	json_VALUE,
	json_ELEMENT,
	json_ELEMENTS,
	json_PAIR,
	json_MEMBER 
} json_type_t;

#ifndef _json_c_
typedef void *pjson_t;
#endif

typedef struct _jnode_t {
	json_type_t  type;
	char        *name;
	char		*pdata;
	size_t	     len;
} jnode_t, *pjnode_t; 

/* type string: */
char *json_type_str(json_type_t type);

/* Clean and validate json string: */
int      json_is_valid(char *b);
char *   json_clean(char *json);

/* sizeof: */
size_t   json_sizeof();

/* constructor & destructor: */
pjson_t  json_new(char *str);
pjson_t  json_destroy(pjson_t js);

/* Parsing: */
int      json_parse(pjson_t js);

/* Navigation: */
ptree_t	 json_tree_get(pjson_t js);

#endif
