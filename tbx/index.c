/*
    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		26/09/2007 	1.0	Creation
    
*/   

#include <stdlib.h>

char index_MODULE[]  = "Index management";
char index_PURPOSE[] = "Data intexing";
char index_VERSION[] = "1.0";
char index_DATEVER[] = "26/09/2007";

#include "atr.h"

typedef struct {
	patr_t atr;
} index_t, *pindex_t;

#define _index_c_
#include "index.h"
#undef  _index_c_ 

void
_index_list_destroy_(int level, char *key, void *data) {
	if (data) list_destroy((plist_t) data);
}

size_t index_sizeof() { return sizeof(index_t); }

pindex_t 
index_destroy(pindex_t i) {
	if (!i) return NULL;
	if (i->atr) {
		atr_foreach(i->atr, _index_list_destroy_);
		i->atr = atr_destroy(i->atr);
	}
	free(i);
	return NULL;
 }	

pindex_t 
index_new() {
	pindex_t i;

	if (!(i = (pindex_t) malloc(sizeof(index_t)))) return NULL;
	if (!(i->atr = atr_new())) return index_destroy(i);
	
	return i;
}

int
index_add(pindex_t i, char *key, void *data) {
	plist_t l;

	if (!i || !i->atr) return 1;
	if (!(l = (plist_t) atr_retrieve(i->atr, key))) {
		if (!(l = list_new())) return 2;
	}
	if (!(atr_store(i->atr, key, l))) return 3;
	if (!(list_push(l, data))) return 4;
	return 0;
}

plist_t 
index_list_retrieve(pindex_t i, char *k) {
	if (!i || !i->atr) return NULL;
	return (plist_t) atr_retrieve(i->atr, k);
}

void *
index_retrieve_first(pindex_t i, char *key) {
	plist_t l;
	if (!i || !i->atr) return NULL;
	if (!(l = index_list_retrieve(i, key))) return NULL;
	return list_first(l);
}

void *
index_retrieve_next(pindex_t i, char *key) {
	plist_t l;
	if (!i || !i->atr) return NULL;
	if (!(l = index_list_retrieve(i, key))) return NULL;
	return list_next(l);
}

void
_index_list_foreach_(int level, char *key, void *data) {
	static f_index_hook_t hook = NULL;
	void *i;
	plist_t l;

	if (!hook || level == -1 || !key) {
		hook = (f_index_hook_t) data;
		return;
	}

	l = (plist_t) data;
	for (i = list_first(l); i; i = list_next(l)) hook(i);
}

void
index_foreach(pindex_t i, f_index_hook_t hook) {
	if (i && i->atr) {
		_index_list_foreach_(-1, NULL, (void *) hook);
		atr_foreach(i->atr, _index_list_foreach_);
		_index_list_foreach_(-1, NULL, NULL);
	}
}

#if defined(_test_index_)

#include <stdio.h>

void 
index_list_print(int level, char *key, void *data) {
	plist_t l;
	long i;
	l = (plist_t) data;
	for (i = (long) list_first(l); i; i = (long) list_next(l)) 
		printf("level %d, val[%s] = %ul\n", level, key, (long) i);
}

void 
index_dump(pindex_t i, f_hook_t f_index_data_print) {
	if (!i || !i->atr) return;
	atr_foreach(i->atr, index_list_print);
}

#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }

int main(void) {
	pindex_t i;
	long  l;

	if (!(i = index_new())) return 1;

	printf("Populate index:\n");
	echo_cmd(index_add(i, "a",   (void *) 55));
	echo_cmd(index_add(i, "aaa", (void *) 1));
	echo_cmd(index_add(i, "aaa", (void *) 2));
	echo_cmd(index_add(i, "aaa", (void *) 3));
	echo_cmd(index_add(i, "bbb", (void *) 1));
	echo_cmd(index_add(i, "c",   (void *) 66));
	
	printf("dump index content:\n");
	index_dump(i, index_list_print);

	printf("retrieval: \n");
	
	echo_cmd(l = (long) index_retrieve_first(i, "aaa"));
	while (l) {
		printf(" -> %d\n", l);
		l = (long) index_retrieve_next(i, "aaa");
	} 

	index_destroy(i);
	return 0;
}
#endif
