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

typedef struct {
	json_type_t  type;
	char        *name;
	char		*pdata;
	size_t	     len;
} jnode_t, *pjnode_t; 

char *json_type_str(json_type_t type);


int      json_is_separator(char *b);
char *   json_nnsep(char *p, char *limit);
char *   json_pnsep(char *p, char *limit);

char *   json_find_closing_quote(char *b);
char *   json_find_closing_sign(char *b);

int      json_is_valid(char *b);
char *   json_clean(char *json);

size_t   json_sizeof();
pjson_t  json_new(char *str);
pjson_t  json_destroy(pjson_t js);
pjnode_t json_node_add(pjson_t js, char *name, json_type_t type, char *pdata, size_t len);

char *   json_string_dup(char *s, size_t len);
char *   json_parse_string(pjson_t js, char *p);
int      json_parse_value(pjson_t js, char *name);
int      json_parse_pairs(pjson_t js);
int      json_parse_array(pjson_t js, char *name);
int      json_parse_object(pjson_t js, char *name);
int      json_parse(pjson_t js);

ptree_t	 json_tree_get(pjson_t js);
void     json_tree_node_print(void *param, int count, int depth, void *node);
void     json_tree_dump(pjson_t js);

#endif
