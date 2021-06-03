#ifndef _obj_h_
#define _obj_h_

#include <stdlib.h>

#ifndef _obj_c_
enum {
	typUNDEF = 0,
	typINT,
	typFLOAT,
	typDOUBLE,
	typSTRING,
	typOOB
};

typedef union {
	int i;
	float f;
	double d;
	char   s[100];
} any_t, *pany_t;

//typedef void *pmember_t;
typedef void *pobj_t;
#endif

pobj_t obj_new(char *name);
pobj_t obj_destroy(pobj_t o);
void   obj_dump(pobj_t o);

int    obj_member_add(pobj_t o, char *name, int type, size_t size, char *format, int null, pany_t def_val);

void * obj_instance_free(void *p);
void * obj_instance_alloc(pobj_t o);
void * obj_instance_reset(pobj_t o, void *instance);
void   obj_instance_dump(pobj_t o, void *instance);

int    obj_member_value_set(pobj_t o, void *instance, char *name, pany_t value);
int    obj_member_value_from_string_set(pobj_t o, void *instance, char *name, char *value);
pany_t obj_member_value_get(pobj_t o, void *instance, char *name);
char * obj_member_value_to_string_get(pobj_t o, void *instance, char *name, char *value);

int    obj_invalid(pobj_t o, void *instance);
char * obj_serialize_csv(pobj_t o, void *instance);
char * obj_serialize_xml(pobj_t o, void *instance); 

#endif
