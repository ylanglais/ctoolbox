/*
	This code is released under GPL terms. 
	Please read license terms and conditions at http://www.gnu.org/

	Yann LANGLAIS

	Changelog:
	15/10/2022  1.0 initial version

	For test-lifo compilation : 
	gcc -g lifo.c -o test-lifo -D_test_lifo_

*/

#include <stdlib.h>
#include <stdio.h>

char lifo_MODULE[]  = "Simple lifo management";
char lifo_PURPOSE[] = "Simple lifo management";
char lifo_VERSION[] = "1.0";
char lifo_DATEVER[] = "15/10/2022";

typedef struct _elmn_t {
	void 	*data;
	struct _elmn_t *prev;
} elmn_t, *pelmn_t;

typedef struct _lifo_t {
	 pelmn_t first;
	 pelmn_t last;
} lifo_t, *plifo_t;

#define _lifo_c_
#include "lifo.h"
#undef  _lifo_c_

static pelmn_t _elmn_new(void *data) {
	pelmn_t e;
	if (!(e = (pelmn_t) malloc(sizeof(elmn_t)))) return NULL;
	e->data = data;
	e->prev = NULL;
	return e;
}

static pelmn_t _elmn_destroy(pelmn_t e) {
	if (e) {
		e->data = NULL;
		e->prev = NULL;
		free(e);
	}
	return NULL;
}

plifo_t 
lifo_new() {
	plifo_t f;
	if (!(f = malloc(sizeof(lifo_t)))) return NULL;
	f->first = f->last = NULL;
	return f;
}

plifo_t 
lifo_destroy(plifo_t f) {
	if (f) {
		while (f->first != NULL) lifo_pop(f);
		f->last = f->first = NULL;
		free(f);
	}
	return NULL;
}

void *
lifo_push(plifo_t f, void *data) {
	pelmn_t e;
	if (!f)    return NULL;
	if (!(e = _elmn_new(data))) return NULL;
	if (f->last == NULL) f->first = e;
	else e->prev = f->last;
	f->last = e;	
	
	return data;
}

void *
lifo_pop(plifo_t f) {
	if (!f)        return NULL;
	if (!f->last)  return NULL;
	void *p;
	pelmn_t e; 
	e = f->last;
	p = e->data;
	f->last = e->prev;
	if (f->last == NULL) f->first = NULL;
	e = _elmn_destroy(e);
	return p;
}

int
lifo_is_empty(plifo_t f) {
	if (!f || !f->first) return 1;
	return 0;
}

void
lifo_dump(plifo_t f, elmn2str_f elmn2str) {
	printf("\nFifo dump %p:\n", f);
	if (!f) {
		printf("null lifo ptr\n");
		return;
	}
	pelmn_t e;

	printf("first: %x, last: %x\n", f->first, f->last);
	
	int i;
	for (i = 0, e = f->last; e != NULL; e = e->prev, i++) {
		printf("% 3d: %x data = %x, prev = % 9x", i, e, e->data, e->prev);
		if (elmn2str != NULL) elmn2str(e->data);
		printf("\n"); 
	}	
}

#if defined(_test_lifo_)

void data_dump(void *p) {
	printf(" (data =	% 3d)", *(int *)p);
}

#include <stdio.h>
int main(void) {
	plifo_t f;

	int i;
	int ints[10];
	for (i = 0; i < 10; i++) ints[i] = i + 1;

	if (!(f = lifo_new())) return 1;
 
	lifo_push(f, (void *) (ints + 2));
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 5));
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 7));
lifo_dump(f, data_dump);
	lifo_pop(f);
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 9));
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 1));
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 3));
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 4));
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 8));
lifo_dump(f, data_dump);
	lifo_push(f, (void *) (ints + 0));
lifo_dump(f, data_dump);

	printf("expecting: 1 9 5 4 2 10 6 3\n");
	printf("having   : ");
	while (!(lifo_is_empty(f))) {
		int *p;
		p = lifo_pop(f);
		printf("%d ", *p);
		if (*p == 3) {
			printf("\n");
			break;
		}
	}	
	printf("\n");

	lifo_destroy(f);
	return 0;
}
#endif
