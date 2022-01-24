#include <stdlib.h>
#include <string.h>

#include "storage.h"
#include "patr.h"
#include "err.h"
#include "mem.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	02/07/2016  1.0	 Creation
	04/08/2016  1.3  add separator in parameter & take text separator into account for counting fields.
*/   

char hds_MODULE[]  = "hierarchical data storage";
char hds_PURPOSE[] = "Hierarchical data storage/indexing (ex. foo.bar[0].object.value)";
char hds_VERSION[] = "1.3";
char hds_DATEVER[] = "04/08/2016";

#if 0
/* w   function support: */
#define hds_NAME_SPEC "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.[]()\""
#endif

/* w/o function support: */
#define hds_NAME_SPEC "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.[]"

static char *hds_error_text[] = {
	"ok", 
	"null hds",
	"bad hds",
	"empty hds",
	"no such data",
	"bad data type",
	"null path",
	"zero path",	
	"unsuitable char for path definition",	
	"not an object",
	"not an array",
	"cannot create node",
	"cannot store node",
	"path not found",
};

typedef enum {
	hdsOK = 0,
	hdsERR_NULL_HDS,
	hdsERR_BAD_HDS,
	hdsERR_EMPTY_HDS,
	hdsERR_NO_SUCH_DATA,
	hdsERR_BAD_DATA_TYPE,
	hdsERR_NULL_STRING,
	hdsERR_ZERO_STRING,
	hdsERR_UNSUITABLE_CHAR,
	hdsERR_NOT_AN_OBJECT,
	hdsERR_NOT_AN_ARRAY,
	hdsERR_NOT_IMPLEMENTED,
	hdsERR_CANNOT_CREATE_NODE,
	hdsERR_CANNOT_STORE_NODE,
	hdsERR_PATH_NOT_FOUND,

	hdsLAST_ERROR_CODE
} hds_error_code_t;

typedef struct {
	pstorage_t store;
	ppatr_t    ppatr;
} hds_t, *phds_t;

#define _hds_c_
#include "hds.h"
#undef  _hds_c_

typedef struct {
	hds_type_t	type;
	hds_any_t   data;
} hdsn_t, *phdsn_t;

char *
hds_error_string(hds_error_code_t errcode) {
	if (errcode <=hdsLAST_ERROR_CODE  || errcode >= hdsLAST_ERROR_CODE) return NULL;
	if (errcode < 0) errcode *= -1;
	return hds_error_text[errcode];	
}

size_t
hds_sizeof() {
	return sizeof(hds_t);
}

size_t
hdsn_sizeof() {
	return sizeof(hdsn_t);
}

phds_t
hds_new() {
	phds_t h;
	if (!(h = mem_zmalloc(hds_sizeof()))) return NULL;
	if (!(h->ppatr = patr_new(hds_NAME_SPEC))) {
		err_error("cannot create patr");
		return hds_destroy(h);
	}
	if (!(h->store = storage_new(hdsn_sizeof(), 100))) {
		err_error("cannot create node store");
		return hds_destroy(h);
	}
	return h;
}

#define hds_test_return_null(h) { if (!h || !h->ppatr || !h->store) return (hds_any_t) NULL; }
#define hds_test_return_err(h)  { if (!h) return hdsERR_NULL_HDS; if (!h->ppatr) return hdsERR_BAD_HDS; if (!h->store) return hdsERR_EMPTY_HDS; }
#define hds_test_return_errn(h) { if (!h) return -hdsERR_NULL_HDS; if (!h->ppatr) return -hdsERR_BAD_HDS; if (!h->store) return -hdsERR_EMPTY_HDS; }

phds_t
hds_destroy(phds_t h) {
	if (h) {
		if (h->ppatr) h->ppatr = patr_destroy(h->ppatr);
		if (h->store) h->store = storage_destroy(h->store);
		free(h);
	}
	return NULL;
}

int 
hds_node_add(phds_t h, char *path, hds_type_t type, hds_any_t data) {
	hdsn_t n, *hn;

	hds_test_return_err(h);

	if (!path || !*path) return hdsERR_NULL_STRING;

	n.type = type;
	n.data = data;

	if (!(hn = (phdsn_t) storage_add(h->store, (char *) &n))) {
		err_error("cannot create node for path %s", path);
		return hdsERR_CANNOT_CREATE_NODE;
	}
	if (!(patr_store(h->ppatr, path, hn))) {
		err_error("cannot store note at %s", path);
		return hdsERR_CANNOT_STORE_NODE;
	}
	return hdsOK;
}

int hds_check(phds_t h, char *path) {
	char *p, *q, *f; 
	phdsn_t hn;

	hds_test_return_err(h);

	for (f = q = p = strdup(path); *p; p++) {
		if (!strchr(hds_NAME_SPEC, *p)) {
			err_error("%c is not a suitable character for a data path", *p);
			free(f);
			return hdsERR_UNSUITABLE_CHAR;
		} 
		if (*p == '.') {
			*p = 0;
			if ((hn = patr_retrieve(h->ppatr, q))) { 
				/* node exists, check its type (must be an hdsTYPE_OBJECT): */
				if (hn->type != hdsTYPE_OBJECT) {
					err_error("%s is not an object", q);
					*p = '.';
					free(f);
					return hdsERR_NOT_AN_OBJECT;
				}
			} else {
				/* node doesn't exist, create it: */
				hds_node_add(h, q, hdsTYPE_OBJECT, (hds_any_t) NULL);
			}
			*p = '.';
		} else if (*p == '[') {
			*p = 0;
			if ((hn = patr_retrieve(h->ppatr, q))) { 
				/* node exists, check its type (must be an hdsTYPE_OBJECT): */
				if (hn->type != hdsTYPE_ARRAY) {
					err_error("%s is not an array", q);
					*p = '[';
					free(f);
					return hdsERR_NOT_AN_ARRAY;
				}
			} else {
				/* node doesn't exist, create it: */
				hds_node_add(h, q, hdsTYPE_ARRAY, (hds_any_t) NULL);
			}
			*p = '[';
		} else if (*p == '(') {
			err_error("methods are not implemented *yet*");
			free(f);
			return hdsERR_NOT_IMPLEMENTED;
 		}
	}	
	free(f);
	return hdsOK;
}

int
hds_put(phds_t h, char *path, hds_type_t type, hds_any_t data) {
	int r;

	hds_test_return_err(h);
	if ((r = hds_check(h, path)) != hdsOK) {	
		return r;
	}
	return hds_node_add(h, path, type, data);
}

hds_type_t
hds_type_get(phds_t h, char *path) {
	phdsn_t hn;

	hds_test_return_errn(h);
	if (!(hn = (phdsn_t) patr_retrieve(h->ppatr, path))) return hdsTYPE_NULL;
	return hn->type;
}

hds_any_t
hds_data_get(phds_t h, char *path) {
	phdsn_t hn;
	hds_test_return_null(h);
	if (!(hn = (phdsn_t) patr_retrieve(h->ppatr, path))) return (hds_any_t) NULL;
	return hn->data;
}

int
hds_get(phds_t h, char *path, hds_type_t *type, hds_any_t *data) {
	phdsn_t hn;
	hds_test_return_err(h);
	if (!(hn = (phdsn_t) patr_retrieve(h->ppatr, path))) return hdsERR_PATH_NOT_FOUND;
	*data = hn->data;
	*type = hn->type;
	return hdsOK;
}

#ifdef _test_hds_
#include <stdio.h>

static char *hds_type_text[] = {
	"null",
	"ptr",
	"char",
	"uchar",
	"int",
	"uint",
	"long",
	"ulong",
	"longlong",
	"ulonglong",
	"float",
	"double",	
	"object",
	"array"
};

void
hds_node_dump(void *p) {
	phdsn_t hn;
	hn = p;
	switch (hn->type) {
	case hdsTYPE_NULL:
		printf("null");	break;
	case hdsTYPE_PTR:
		printf("%p", hn->data.hds_ptr); break;
	case hdsTYPE_CHAR:
		printf("%d (%c)", hn->data.hds_char, hn->data.hds_char); break;
	case hdsTYPE_UCHAR:
		printf("%u (%u)", hn->data.hds_uchar, hn->data.hds_uchar); break;
	case hdsTYPE_INT:
		printf("%d", hn->data.hds_int); break;
	case hdsTYPE_UINT:
		printf("%u", hn->data.hds_uint); break;
	case hdsTYPE_LONG:
		printf("%ld", hn->data.hds_long); break;
	case hdsTYPE_ULONG:
		printf("%lu", hn->data.hds_ulong); break;
	case hdsTYPE_LONGLONG:
		printf("%lld", hn->data.hds_longlong); break;
	case hdsTYPE_ULONGLONG:
		printf("%llu", hn->data.hds_ulonglong); break;
	case hdsTYPE_FLOAT:
		printf("%f", hn->data.hds_float); break;
	case hdsTYPE_DOUBLE:
		printf("%f", hn->data.hds_double); break;
	case hdsTYPE_OBJECT:
		printf("object"); break;
	case hdsTYPE_ARRAY:
		printf("array");  break;
			
	default:
		printf("unknown data type");
		return;
	} 
	printf(" (%s)" , hds_type_text[hn->type]);
}

void
hds_dump(phds_t h) {
	if (h)
	patr_dump(h->ppatr, hds_node_dump);
}

int main(void) {
	char t_str[] = "orangered";

	hds_any_t any;
	
	phds_t h;
	if (!(h = hds_new())) {
		return 1;
	}
	any.hds_int    = 1;
	hds_put(h, "base.int",     	 hdsTYPE_INT,    any);

	any.hds_float  = 1;
	hds_put(h, "base.obj.float", hdsTYPE_FLOAT,  any);

	any.hds_double = 1;
	hds_put(h, "base.arr[0]",    hdsTYPE_DOUBLE, any);

	any.hds_double = 4;
	hds_put(h, "base.arr[1]",    hdsTYPE_DOUBLE, any);


	any.hds_ulong  = 1;
	hds_put(h, "base.arr[toot]", hdsTYPE_ULONG,  any);

	any.hds_ptr    = t_str;
	hds_put(h, "base.str",       hdsTYPE_PTR,    any);

	hds_dump(h);

	hds_destroy(h);

	return 0;
}

#endif
