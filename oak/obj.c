#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <atr.h>
#include <dyna.h>
#include <storage.h>

#define objNAMESIZE 64
#define objFMTSIZE 64

enum {
	typUNDEF = 0,
	typINT,
	typFLOAT,
	typDOUBLE,
	typSTRING,
	typARRAY,
	typOBJ,
	typOOB
};

char *type_names[] = {
	"undefined",
	"int",
	"float",
	"double",
	"string",
	"array",
	"object",
	"out of bounds"
};

char *types_default_formats[] = {
	"ERROR-undef",
	"%d",
	"%f",
	"%f",
	"%s",
	"",
	"",
	"ERROR-out-of-bounds"
};

char *
type_default_format(int type) {
	if (type < typUNDEF || type >= typOBJ) 
		return types_default_formats[typOOB];
	return types_default_formats[type];
}

typedef union {
	int    i;
	float  f;
	double d;
	char   s[100];
	pdyna_t	array;
	void   *ptr;
} any_t, *pany_t;

int
type_string_to_any(int type, char *fmt, char *string, pany_t a) {
	if (!fmt) if (!(fmt = type_default_format(type))) return 1;
	switch (type) {
	case typINT:
		sscanf(string, fmt, a->i);
		return 0;
	case typFLOAT:
		sscanf(string, fmt, a->f);
		return 0;
	case typDOUBLE:
		sscanf(string, fmt, a->d);
		return 0;
	case typSTRING:
		strcpy(a->s, string);
		return 0;
	case typUNDEF:
		fprintf(stderr, "undefined type\n");
		return 2;
	default:
		fprintf(stderr, "type out of bonds\n");
	}
	return 3;
}

typedef struct _member_t {
	struct _member_t *parent;
	char   name[objNAMESIZE + 1];
	int	   type;
	size_t size;
	size_t offset;
	char   format[objFMTSIZE + 1];
	int	   null;
	any_t  def_val;
} member_t, *pmember_t;

typedef struct _obj_t {
	char   name[objNAMESIZE + 1];
	patr_t atr;
	int	   nmembers;
	pmember_t members;
	size_t size;
} obj_t, *pobj_t;

typedef struct {
	patr_t atr;
	pstorage_t store;
} objlist_t, *pobjlist_t;

#define _obj_c_
#include <obj.h>
#undef  _obj_c_

pobj_t 
obj_destroy(pobj_t o) {
	if (o) {
		if (o->members) {
			free(o->members);
			o->members = NULL;
		} 
		if (o->atr) o->atr = atr_destroy(o->atr);
		o->nmembers = 0;
		*o->name = 0;
	}
	return NULL;
}	

pobjlist_t
objlist_destroy(pobjlist_t ol) {
	if (ol) {
		if (ol->store) ol->store = storage_destroy(ol->store);
		if (ol->atr)   ol->atr   = atr_destroy(ol->atr);
		free(ol);
	}
	return NULL;
}

pobjlist_t 
objlist_new() {
	pobjlist_t ol;
	if (!(ol = (pobjlist_t) malloc(sizeof(objlist_t)))) return NULL;
	if (!(ol->store = storage_new(sizeof(obj_t), 50)))  return objlist_destroy(ol);
	if (!(ol->atr   = atr_new())) return objlist_destroy(ol); 
	return ol;
}

pobj_t
objlist_obj_add(pobjlist_t ol, char *name) {
	obj_t o;
	if (!ol) return NULL;
	strncpy(o.name, name, objNAMESIZE);
	//obj_store();
	return NULL;
}

pobjlist_t
objlist_read(char *filename) {
	return NULL;
}


pobj_t 
obj_new(char *name) {
	pobj_t o;
	if (!(o = (pobj_t) malloc(sizeof(obj_t)))) return NULL;
	o->nmembers = 0;
	o->members  = NULL;
	if (!(o->atr = atr_new())) return obj_destroy(o);
	strncpy(o->name, name, objNAMESIZE);
	return o;
}

void
obj_dump(pobj_t o) {
	int i;
	pmember_t m;

	if (!o) {
		printf("Null object\n");
		return;
	}

	printf("Object (%x);\n", (unsigned int) o);
	printf("- name: %s\n", o->name);
	printf("- size: %u\n", o->size);
	printf("- nb members: %d\n", o->nmembers);
	printf("- members:\n");
		
	for (i = 0; i < o->nmembers; i++) {
		m = o->members + i;
		printf("\tMember # %d\n", i);
	    printf("\t- name:   %s\n",  m->name);
		printf("\t- type:   %s\n",  type_names[m->type]);
		printf("\t- size:   %u\n",  m->size);
		printf("\t- offset: %u\n", m->offset);	
		printf("\t- format: \"%s\"\n", m->format);
		printf("\t- null:   %d\n", m->null);
		printf("\t- def_val: ");
		switch (m->type) {
		case typINT:
			printf("%d\n", m->def_val.i);
			break;
		case typFLOAT:
			printf("%f\n", m->def_val.f);
			break;
		case typDOUBLE:
			printf("%f\n", m->def_val.d);
			break;
		case typSTRING:
			printf("\"%s\"\n", m->def_val.s);
			break;
		case typUNDEF:
			printf("undefined type\n");
			break;
		default:
			printf("type out of bonds\n");
		}
	}
}

int
obj_member_add(pobj_t o, char *name, int type, size_t size, char *format, int null, pany_t def_val) {
	size_t s;

	if (!o) return 1;
	
	if (!o->nmembers) {
		if (!(o->members = (pmember_t) malloc(sizeof(member_t)))) return 2;
	} else {
		void *p;

		if (!(p = realloc(o->members, (o->nmembers + 1) * sizeof(member_t)))) 
			return 3;
		o->members = (pmember_t) p;
	}
	strncpy(o->members[o->nmembers].name, name, objNAMESIZE);
	o->members[o->nmembers].type   = type;
	o->members[o->nmembers].size   = size; 
	o->members[o->nmembers].offset = o->size;
	o->members[o->nmembers].null   = null;
	
	if (def_val) 
		memcpy(&o->members[o->nmembers].def_val, def_val, sizeof(any_t));
	else 
		memset(&o->members[o->nmembers].def_val, 0,       sizeof(any_t));

	/* 4 bytes alignment: */
	if ((s = size) % 4) s += 4 - size % 4;
	o->size += s;

	o->nmembers++; 
	atr_store(o->atr, name, (char *) (o->members + o->nmembers));

	return 0; 
} 

void *
obj_instance_free(void *p) {
	if (p) free(p);
	return NULL;
}

void *
obj_instance_alloc(pobj_t o) {
	if (!o) return NULL;
	return malloc(o->size);
}

void *
obj_instance_reset(pobj_t o, void *instance) {
	if (!o || !instance) return NULL;
	memset((char *) instance, 0, o->size);
	return instance;
}

void
obj_instance_dump(pobj_t o, void *instance) {
	/* TODO */
	obj_serialize_csv(o, instance);
}

int 
obj_member_value_set(pobj_t o, void *instance, char *name, pany_t value) {
	char *p;
	pmember_t m;

	if (!o) return 1;
	if (!(m = atr_retrieve(o->atr, name))) return 2;
	p = (char *) instance;
	memcpy(p + m->offset, value, sizeof(any_t));
	return 0;
}

int
obj_member_value_from_string_set(pobj_t o, void *instance, char *name, char *value) {
	pmember_t m;
	char *p, *fmt;

	if (!o) return 1;
	if (!(m = atr_retrieve(o->atr,  name))) return 2;
	p = (char *) instance;
	if (!(fmt = m->format)) if (!(fmt = type_default_format(m->type))) return 3;
	type_string_to_any(m->type, fmt, value, (pany_t) (p + m->offset));
	return 0;
}

pany_t
obj_member_value_get(pobj_t o, void *instance, char *name) {
	pany_t value;
	pmember_t m;
	char *p;
	if (!o || !instance) return NULL;
	if (!(m = atr_retrieve(o->atr, name))) return NULL;	
	if (!(value = (pany_t) malloc(sizeof(any_t)))) return NULL;
	p = (char *) instance;
	memcpy(value, p + m->offset, sizeof(any_t));	
	return value;
}

char *
obj_member_value_to_string_get(pobj_t o, void *instance, char *name, char *value) {
	/* TODO */
	return NULL;
}

int 
is_null(int type, pany_t value) {
	switch (type) {
	case typINT: 
		if (!value->i) return 1;
	case typFLOAT:
		if (value->f == 0.) return 1;
	case typDOUBLE:
		if (value->d == 0.) return 1;
	case typSTRING:
		if (!*value->s) return 1;
	default:
		return -1;
	}
	return 0;
}

int
obj_invalid(pobj_t o, void *instance) {
	pmember_t m;
	int i;
	char *p;

	if (!o || !instance) return -1;
		
	p = (char *) instance;

	for (i = 0; i < o->nmembers; i++) {
		m = o->members + i;
		if (!m->null && is_null(m->type, (pany_t) (p + m->offset))) return 1; 	
	}	
	return 0;
} 

char *
obj_strncat_at(char *p, char *q, size_t *size) {
	while (*size-- && (*++p = *++q));
	if (!*size) *(p - 1) = 0; 
	return p;
}

char *
obj_serialize_csv(pobj_t o, void *instance) {
	int i;
	size_t size = 4096;
	char *p, buf[100], outbuf[4096], *pout, *fmt;
	pany_t a;
	pmember_t m;

	if (!o || !instance) return NULL;
	pout = outbuf;
	p = (char *) instance;

	for (i = 0; i < o->nmembers; i++) {
		m = o->members + i;
		if (!(a = obj_member_value_get(o, instance, m->name))) {
			if (!m->null) {
				fprintf(stderr, "ERROR getting value of member \"%s\" on a specified non null value... skipping current entry\n", m->name); 
				return NULL;
			}
			fprintf(stderr, "ERROR getting value of member \"%s\" => setting empty value...\n", m->name); 
		}
		if (!m->null && !is_null(m->type, a)) {
			fprintf(stderr, "ERROR field \"%s\" is null on a specified non null value... skipping current entry\n", m->name);
			return NULL;
		}
	
		if (m->format && *m->format) fmt = m->format;
		else fmt = type_default_format(m->type);

		sprintf(buf, fmt, a);
		pout = obj_strncat_at(pout, buf, &size);
		pout = obj_strncat_at(pout, ";", &size); 
	}
	pout[-1] = 0;
 	return strdup(outbuf);
}

char *
obj_serialize_xml(pobj_t o, void *instance) {
/* TODO */
	return NULL;
}

#ifdef _test_obj_
int main() {
	pobj_t o; 
	int    intgr;
	double doubl;

	o = obj_new("test");
	
	intgr =  1;
	doubl  = 47;
	obj_member_add(o, "number", typINT,    sizeof(int),    NULL, 0, (pany_t) &intgr);
	obj_member_add(o, "double", typDOUBLE, sizeof(double), NULL, 0, (pany_t) &doubl);
	
	void *i1, *i2;

	i1 = obj_instance_alloc(o);
	i2 = obj_instance_alloc(o);

	printf("dump i1:\n");
	obj_instance_dump(o, i1);

	intgr = 17; doubl = 24.3;	
	obj_member_value_set(o, i1, "number", (pany_t) &intgr); 
	obj_member_value_set(o, i1, "double", (pany_t) &doubl); 

	obj_instance_dump(o, i1);

	obj_instance_free(i1);
	obj_instance_free(i2);
	obj_destroy(o);
	return 0;
}
#endif

