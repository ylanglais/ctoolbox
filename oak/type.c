
#include <stdlib.h>
#include <string.h>
#include <strings.h>


#include <err.h>
#include <storage.h>
#include <hash.h>

#define typeBAD ((unsigned long) (~0));

typedef void * (*from_string_f)(void *, char *);
typedef char * (*to_string_f)(char *, void *);
typedef int    (*is_valid_f)(void *);

typedef unsigned long typeid_t;
typedef char          typename_t[64];
typedef char          typedesc_t[256];
typedef char          typefmt_t[64];

typedef struct {
	typeid_t      type;
	typename_t    name;
	size_t        size;
	typedesc_t    description;
	typefmt_t     format;
	is_valid_f	  is_valid;
	to_string_f   to_string;
	from_string_f from_string;
} type_t, *ptype_t;

static pstorage_t _typestore_ = NULL;
static phash_t	  _typehash_  = NULL;

void
_type_init_() {
	if (!_typestore_) {
		if (!(_typestore_ = storage_new(sizeof(type_t), 20))) {
			err_error("cannot create type storage");
			return;
		}
	}
	if (!_typehash_) {
		if (!(_typehash_  = hash_new())) {
			err_error("cannot create type hash table");
			return;
		}
	}
}

#ifdef SOLARIS
#pragma init (type_init)
#endif
#ifdef LINUX
void type_init() __attribute__((constructor));
#endif

void 
_type_fini_() {
	if (_typestore_) storage_destroy(_typestore_);
	if (_typehash_)  hash_destroy(_typehash_);
}

#ifdef SOLARIS
#pragma fini (type_fini)
#endif
#ifdef LINUX
void type_fini() __attribute__((destructor));
#endif

typeid_t 
type_add(char *name, size_t size, char *description, char *format, is_valid_f is_valid, to_string_f to_string, from_string_f from_string) {
	void *p;
	type_t t;

	if (!_typestore_ || ! _typehash_) _type_init_();
	
	t.type = storage_used(_typestore_);
	strncpy(t.name, name, sizeof(typename_t));
	t.size = size;
	strncpy(t.description, description, sizeof(typedesc_t));
	strncpy(t.format, format, sizeof(typefmt_t));
	t.is_valid = is_valid;
	t.to_string = to_string;
	t.from_string = from_string;
	
	if (hash_retrieve(_typehash_, name)) {
		err_error("type %s already exist", name); 
		return typeBAD;
	}
	
	p = storage_add(_typestore_, (char *) &t);
	if (hash_insert(_typehash_, name, p)) {
		err_error("type %s cannot be hashed", name);		
		return typeBAD;
	}
	
	return t.type;
}

ptype_t 
type_get(typeid_t type) {
	ptype_t t;
	if (type > (typeid_t) storage_used(_typestore_)) {
		err_warning("bad type");
		return NULL;
	} 
	if (!(t = (ptype_t) storage_get(_typestore_, type))) {
		err_warning("type hash not been completely registerd");	
		return NULL;
	}
	return t;
}

char *
type_name(typeid_t type) {
	ptype_t t;
	if (t = type_get(type)) {
		err_warning("type hash not been completely registerd");	
		return NULL;
	}
	return t->name;
}

typeid_t 
type_id(char *name) {
	ptype_t t;
	
	if (!(t = (ptype_t) hash_retrieve(_typehash_, name))) {
		err_warning("%s is a bad type name", name);	
		return typeBAD;
	}
	return t->type;
}

char *
type_description(typeid_t type) {
	ptype_t t;
	if (t = type_get(type)) {
		err_warning("type hash not been completely registerd");	
		return NULL;
	}
	return t->description;
}

char *
type_format(typeid_t type) {
	ptype_t t;
	if (t = type_get(type)) {
		err_warning("type hash not been completely registerd");	
		return NULL;
	}
	return t->format;
}

char *
type_to_string(typeid_t type, char *string, void *data) {
	ptype_t t;
	if (!data) {
		err_warning("no data");
		return NULL;
	}
	if (!(t = type_get(type))) {
		err_warning("type hash not been completely registerd");	
		return NULL;
	}
	if (!(t->to_string)) {
		err_warning("type %s has no \"to_string\" method", type_name(type));
		return NULL;
	}	
	return t->to_string(string, data);	
}

void *
type_from_string(typeid_t type, void *data, char *string) {
	ptype_t t;
	if (!string || !*string) {
		err_warning("no string");
		return NULL;
	}
	if (!(t = type_get(type))) {
		err_warning("type hash not been completely registerd");	
		return NULL;
	}
	if (!t->from_string) {
		err_warning("type %s has no \"from_string\" method", type_name(type));
		return NULL;
	}
	return t->from_string(data, string);
}

int
type_is_valid(typeid_t type, void *data) {
	ptype_t t;
	if (!data) {
		err_warning("no data");
		return -1;
	}
	if (!(t = type_get(type))) {
		err_warning("type hash not been completely registerd");	
		return -2;
	}
	if (!t->is_valid) {
		err_warning("type %s has no \"is_valid\" method", type_name(type));
		return -3;
	}
	return t->is_valid(data);
}


int main(void) {
	err_level_set(err_WARNING);
	type_add("int", sizeof(int), "basic integer", "%d", NULL, NULL, NULL);
	type_is_valid(type_id("int"), NULL);
	return 0;	
}

