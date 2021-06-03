/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    30/05/2016  0.8	Creation

*/   
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "mem.h"
#include "re.h"
#include "tree.h"
#include "storage.h"

typedef struct {
	char       *string;
	char       *current;
	pstorage_t  store;
	ptree_t     tree;
} json_t, *pjson_t;

#define _json_c_
#include "json.h"
#undef  _json_c_

char json_MODULE[]  = "Json support";
char json_PURPOSE[] = "Preliminaty json support (parsing)";
char json_VERSION[] = "0.8";
char json_DATEVER[] = "30/05/2016";



static char *jsonTYPE_TXT[] = {
	"null",
	"false",
	"true",
	"E",
	"digit",
	"digits",
	"digit19",
	"digits19",
	"hexdigit",
	"hexdigits",
	"exp",
	"frac",
	"int",
	"number",
	"char",
	"chars",
	"string",
	"array",
	"object",
	"value",
	"element",
	"elements",
	"pair",
	"member"
};

char *json_type_str(json_type_t type) {
	return jsonTYPE_TXT[type];
}

#define jsonRE_NUMBER      "^((-){0,1}(0|([1-9][0-9]{0,}))(\\.[0-9]{0,}){0,1}([eE][+\\-]?[0-9]{1,}){0,1})"
#define jsonRE_VALIDATE    "[^,:{}\\[\\]0-9.\\-+Eaeflnr-u \n\r\t]"

pre_t _json_re_number_ = NULL;
int
json_is_separator(char *b) {
	if (!b) return 0;
	if (*b == ' ' || *b == '\t' || *b == '\n') return 1;
	return 0;
}

char *
json_nnsep(char *p, char *limit) {
	while (*p && p < limit && json_is_separator(p)) p++;
	return p;
}
char *
json_pnsep(char *p, char *limit) {
	while (p > limit &&  json_is_separator(p)) p--;
	return p;
}

char *
json_find_closing_quote(char *b) {
	char l, *p;
	if (*b != '\'' && *b != '"')  return b;
	l = *b;
	for (p = b + 1; *p && *p != l; p++) {
		if (*p == '\\') p++;
	} 
	return p;
}

char *
json_find_closing_sign(char *b) {
	char *p, l;
	int   level = 0;

	if      (*b == '{') l = '}';
	else if (*b == '[') l = ']';
	else if (*b == '(') l = ')';
	else return b;

	for (p = b + 1; *p && (*p != l || level != 0); p++) {
		if (*p == '\'' || *p == '"') {
			p = json_find_closing_quote(p);
		} else if (*p == *b) {
			level++;
		} else if (*p == l) {
			level--;
		}
	}
	if (*p != l || level) return NULL;
	return p;	
}

char *
json_clean(char *json) {
	char *b, *p, *q;
	if (!json || !*json) {
		err_error("null or empty string");
		return NULL;
	}

	if (!(b = mem_zmalloc(sizeof(wchar_t) * strlen(json) + 1))) {
		err_error("no memory");
		return NULL;

	}
	/* copy input string w/o speparators: */
	for (p = json, q = b; *p; p++) {
		/* verbatim copy of strings: */
		if (*p == '\'' || *p == '"') { 
			char *t;
			if (!(t = json_find_closing_quote(p))) {
				err_error("no matching quote at offset %d", p - b);
				return NULL;
			}
			while (p <= t) {
				*q++ = *p++;
			}
		} 
		if (!json_is_separator(p)) {
			*q++ = *p;
		}
	}
	return b;
}

int
json_is_valid(char *b) {
	pre_t r;
	prematch_t p;
	r = re_new(NULL, jsonRE_VALIDATE, reEXTENDED|reUTF8);
	if ((p = re_find(r, b))) {
		p = rematch_destroy(p);
		return 1;
	} 
	return 0;
}

size_t
json_sizeof() { return sizeof(json_t); }

pjson_t
json_destroy(pjson_t js) {
	if (js) {
		if (js->string) js->string = mem_free(js->string);
		if (js->store)  js->store  = storage_destroy(js->store);
		if (js->tree)   js->tree   = tree_destroy(js->tree);
	}
	return NULL;	
}

pjnode_t
json_node_add(pjson_t js, char *name, json_type_t type, char *pdata, size_t len) {
	jnode_t jn;
	
	if (!js) 	    return NULL;
	if (!js->store) return NULL;
	jn.type  = type;
	jn.name  = name;
	jn.pdata = pdata;
	jn.len   = len;

	return (pjnode_t) storage_add(js->store, (char *) &jn);
}

ptree_t 
json_tree_get(pjson_t js) {
	if (!js) return NULL;
	return js->tree;
}

int	
json_tree_destroy(pjson_t js) {
	int i, n;

	if (!js->tree)  return 0;
	if (!js->store) return 0;		

	for (i = 0, n = storage_used(js->store); i < n; i++) {
		pjnode_t jn = (pjnode_t) storage_get(js->store, i);
		if (jn->name) free(jn->name);
	} 
	js->store = storage_destroy(js->store);
	js->tree  = tree_destroy(js->tree);
	return 0;	
}

char *
json_string_dup(char *s, size_t len) {
	char *p;
	if (!(p = mem_zmalloc(len + 1))) return NULL;
	strncpy(p, s, len);
		
	return p;
}

char *
json_parse_string(pjson_t js, char *p) {
	char *s, *e;
	if (!js || !p) return NULL;
	
	if (*p != '"') {
		err_error("invalid char at offset %d (%c), while '\"' was expected", p - js->string, *p);
		return NULL;
	}

	if (!(s = json_find_closing_quote(p))) {
		err_error("cannot file end of string starting at %d", p - js->string);
		return NULL;
	}
	return json_string_dup(p + 1, s - p - 1);
}		

int json_parse_value(pjson_t js, char *name) {
	pjnode_t   jn = NULL;
	prematch_t rm = NULL;

	if (!js || !js->current) return 1;
	if (!strncmp("null", js->current, strlen("null"))) {
		if (!(jn = json_node_add(js, name, json_NULL, js->current, strlen("null")))) {
			return 2;
		} 
		tree_child_add(js->tree, jn);
	} else if (!strncmp("true", js->current, strlen("true"))) {
		if (!(jn = json_node_add(js, name, json_TRUE, js->current, strlen("true")))) {
			return 3;
		} 
		tree_child_add(js->tree, jn);
	} else if (!strncmp("false", js->current, strlen("false"))) {
		if (!(jn = json_node_add(js, name, json_FALSE, js->current, strlen("false")))) {
			return 4;
		} 
		tree_child_add(js->tree, jn);
	} else if (*js->current == '"') {
		char *str = NULL;
		if (!(str = json_parse_string(js, js->current))) {
			return 5;
		}
		if (!(jn = json_node_add(js, name, json_STRING, js->current + 1, strlen(str)))) {
			return 6;
		} 
		js->current += strlen(str) + 2;
		if (str) free(str);
		tree_child_add(js->tree, jn);
	} else if (*js->current == '{') {
		if (json_parse_object(js, name)) {
			return 7;
		}
		js->current++;
	} else if (*js->current == '[') {
		if (json_parse_array(js, name)) {
			return 8;
		}
		js->current++;
	} else if ((rm = re_find(_json_re_number_, js->current))) {
		if (!(jn = json_node_add(js, name, json_NUMBER, js->current, rm->subs[0].eo - rm->subs[0].so))) {
			return 9;
		}
		js->current += rm->subs[0].eo - rm->subs[0].so;
		tree_child_add(js->tree, jn);
	} else {
		err_error("illegal value at %d (%12s...), while any of string, number, object, array, true, false or num was expected",  js->current - js->string, js->current);
		return 10;
	}
	return 0;
}

int json_parse_pairs(pjson_t js) {
	if (!js || !js->current) return 1;

	while (*js->current && *js->current != '}') {
		char *str;
		if (*js->current != '"') {
			err_error("invalid char at offset %d (%c), while '\"\' was expected", js->current - js->string, *js->current);
			return 2;
		}
		if (!(str = json_parse_string(js, js->current))) {
			return 3;
		}
		js->current += strlen(str) + 2;
		if (!*js->current) {
			err_error("premature end of file at offset %d while ':' was expected", js->current - js->string);
			return 4;
		}
		if (*js->current != ':') {
			err_error("invalid char at offset %d (%c), while ':' was expected", js->current, *js->current);
			return 5;
		}	
		js->current++;
		if (json_parse_value(js, str)) {
			return 6;
		}
		if (!*js->current || (*js->current != ',' && *js->current != '}')) {
			return 7;
		}
		if (*js->current == ',') js->current++;
	}
	return 0;
}

int json_parse_array(pjson_t js, char *name) {
	char *e;
	pjnode_t pn;

	if (!js)                           return 1;
	if (!js->current || !*js->current) return 2;
	if (*js->current != '[') 		   return 3;

	if (!(e = json_find_closing_sign(js->current))) {
		err_error("cannot find end of object ('}') starting at offset %d", js->current - js->string);
		return 4;
	}
	
	pn = json_node_add(js, name, json_ARRAY, js->current, e - js->current);
	tree_child_add(js->tree, (void *) pn);

	js->current++;
	if (*js->current == ']') {
		/* empty array */	
		js->current++;
		return 0;
	}

	tree_up(js->tree);	

	while (*js->current && *js->current != ']') {
		if (json_parse_value(js, NULL)) {
			return 5;
		}
		if (!*js->current || (*js->current != ',' && *js->current != ']')) {
			err_error("invalid toker at %d (%c), while a coma was required", js->current - js->string, *js->current);
			return 6;
		}
		if (*js->current == ',') js->current++;
	}
	tree_down(js->tree);
	return 0;
}

int json_parse_object(pjson_t js, char *name) {
	char *e;
	pjnode_t pn;

	if (!js)                           return 1;
	if (!js->current || !*js->current) return 2;
	if (*js->current != '{') 		   return 3;
	
	if (!(e = json_find_closing_sign(js->current))) {
		err_error("cannot find end of object ('}') starting at offset %d", js->current - js->string);
		return 4;
	}

	pn = json_node_add(js, name, json_OBJECT, js->current, e - js->current);
	tree_child_add(js->tree, (void *) pn);

	js->current++;
	if (*js->current == '}') {
		js->current++;
		return 0;
	}

	tree_up(js->tree);	

	while (*js->current && *js->current != '}') {
		char *name = NULL;
		json_type_t type;
		size_t len;

		if (json_parse_pairs(js)) {
			return 8;
		}
	}
	tree_down(js->tree);
	return 0;
}

int json_parse(pjson_t js) {
	pjnode_t jn;

	if (!js) {
		err_error("null json data");
		return 1;
	}
	if (!js->string) {
		err_warning("noting to parse");
		return 0;
	}
	if (js->tree) json_tree_destroy(js);

	if (!(js->tree = tree_new())) {
		err_error("cannot create tree");
		return 1;
	}

	if (!(js->store = storage_new(sizeof(jnode_t), 500))) {
		err_error("cannot create storage");
		js->tree = tree_destroy(js->tree);
		return 2;
	}

 	js->current = js->string;

	if (*js->current != '{' && *js->current != '[') {
		err_error("json doesn't start by a curly brace '{' or a braquet '['");
		json_tree_destroy(js);
		return 3;
	}

	if (*js->current == '{') {
		if (json_parse_object(js, NULL)) {
			json_tree_destroy(js);
			return 4;
		}
	} else {
		if (json_parse_array(js, NULL)) {
			json_tree_destroy(js);
			return 5;
		}
	}
	return 0;
}

pjson_t
json_new(char *str) {
	pjson_t js;
	char *b = NULL;

	if (str) {
		if (!json_is_valid(str)) {	
			err_error("invalid json string passed");
			return NULL;
		} 
		if (!(b = json_clean(str))) {
			err_error("bad json string passed");
			return NULL;
		}
	}	
	
	if (!(js = mem_zmalloc(json_sizeof()))) {
		err_error("no memory");
		return NULL;
	}
	js->string  = b;
	js->current = b;

	if (!_json_re_number_) {
		if (!(_json_re_number_ = re_new(NULL, jsonRE_NUMBER, reEXTENDED|reUTF8))) {
			err_error("cannot create regular expression for json number matching");
			return json_destroy(js);;
		}
	}

	if (js->string) {
		if (json_parse(js)) {
			err_error("broken json string");
			return json_destroy(js);	
		}
	}	

	return js;
}

#if 0 
char *
json_node_from_path(pjson_t j, char *path) {
	char *e, *p;
	pjnode_t n;

	n = (pjnode_t) tree_root(j->tree);
	if n->(
	
	for (e = p = path; *e; e++) {
		if (*e == '.') {
		}
	}
#endif

#include <stdio.h>

void
json_tree_node_print(void *param, int count, int depth, void *node) {
	int i;
	char *t;
	pjnode_t jn;

	printf("%d: ", depth);
	jn = *(pjnode_t*) node;
	if (depth >= 1) printf(" ");
	for (i = 1; i < depth; i++) printf(" | ");
	if (depth > 0) printf(" |- ");
	if (jn->name) printf("%s: ", jn->name);	
	switch (jn->type) {
	case json_STRING:
	case json_NUMBER:
		t = json_string_dup(jn->pdata, jn->len);
		printf("%s (%s)\n", t, jsonTYPE_TXT[jn->type]);
		free(t);
		break;
	case json_NULL:
	case json_TRUE:
	case json_FALSE:
		printf("%s (%s)\n", jsonTYPE_TXT[jn->type], jsonTYPE_TXT[jn->type]);
		break;
	case json_ARRAY:
	case json_OBJECT:
		printf("%s:\n", jsonTYPE_TXT[jn->type]);
		//for (i = 0; i < depth; i++) printf("  ");
		//printf("  +");
		break;
	default:
		printf("invalid json node data\n");
	}		
}

void 
json_tree_dump(pjson_t js) {
	if (!js->tree) return;
	tree_foreach(js->tree, NULL, json_tree_node_print);
}

#ifdef _test_json_
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;

static char tests_json[] = "\
{ \"numbers\" : { \"int\" : 123456, \"negint\" : -12345, \"float1\": 0.123, \"negfloat\": -0.123, \"exp\": 0.123e12, \"negexp\" : -0.123E5, \"nebexpneg\":-0.123e-5 }, \n\
  \"menu\": {\n\
  \"id\": \"file\",\n\
  \"value\": \"File\",\n\
  \"popup\": {\n\
    \"menuitem\": [\n\
      {\"value\": \"New\",   \"onclick\": \"CreateNewDoc()\"},\n\
      {\"value\": \"Open\",  \"onclick\": \"OpenDoc()\"},\n\
      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}\n\
    ]\n\
  }\n\
}}";

#define ENUM_STR(x) (#x)

int main(int n, char *a[]) {
	char *b;
	pjson_t js;
	char *ts, *tf = NULL;
	
	if (n > 2 && !strcmp(a[1], "-f")) {
		int f;
		struct stat stb;
		if (stat(a[2], &stb)) {	
			printf("cannot open file %s\n", a[2]);
			return 1;
		}	
		if ((f = open(a[2], O_RDONLY)) < 0) {
			printf("cannot read file %s\n (%s)", a[2],strerror(errno));
			return 2;
		}
		
		if (!(tf = malloc(stb.st_size + 1))) {
			printf("cannot allocate memory to read file %s\n", a[2]);
			close(f);
			return 3;
		}
		if (read(f, tf, stb.st_size) != stb.st_size) {
			printf("problem reading file %s\n", a[2]);
			free(tf);
			close(f);
			return 4;
		}
		close(f);
		ts = tf;
	} else if (n > 1) ts = a[1];
	else ts = tests_json;

	printf("Test string:\n%s\n", ts);
	if(!(b = json_clean(ts))) {
		err_error("cannot create json buffer");
		if (tf) free(tf);
		return 1;	
	}
	printf("json buffer = \n%s\n", b);
	printf("\n");

	if (json_is_valid(b)) {
		free(b);
		printf("Test string is a valid json string\n");
		if ((js = json_new(ts))) {
			json_tree_dump(js);	
		}
		json_destroy(js);
	} else 
	    printf("Test string is NOT a valid json string\n");

	if (tf) free(tf);
	return 0;
}
#endif
