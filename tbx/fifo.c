/*
	This code is released under GPL terms. 
	Please read license terms and conditions at http://www.gnu.org/

	Yann LANGLAIS

	Changelog:
	15/10/2022  1.0 initial version

	For test-fifo compilation : 
	gcc -g fifo.c -o test-fifo -D_test_fifo_

*/

#include <stdlib.h>
#include <stdio.h>

char fifo_MODULE[]  = "Simple fifo management";
char fifo_PURPOSE[] = "Simple fifo management";
char fifo_VERSION[] = "1.0";
char fifo_DATEVER[] = "15/10/2022";

typedef struct _elmn_t {
	void 	*data;
	struct _elmn_t *next;
} elmn_t, *pelmn_t;

typedef struct _fifo_t {
	 pelmn_t first;
	 pelmn_t last;
} fifo_t, *pfifo_t;

#define _fifo_c_
#include "fifo.h"
#undef  _fifo_c_

static pelmn_t _elmn_new(void *data) {
	pelmn_t e;
	if (!(e = (pelmn_t) malloc(sizeof(elmn_t)))) return NULL;
	e->data = data;
	e->next = NULL;
	return e;
}

static pelmn_t _elmn_destroy(pelmn_t e) {
	if (e) {
		e->data = NULL;
		e->next = NULL;
		free(e);
	}
	return NULL;
}

pfifo_t 
fifo_new() {
	pfifo_t f;
	if (!(f = malloc(sizeof(fifo_t)))) return NULL;
	f->first = f->last = NULL;
	return f;
}

pfifo_t 
fifo_destroy(pfifo_t f) {
	if (f) {
		while (f->first != NULL) fifo_pop(f);
		f->first = f->last = NULL;
		free(f);
	}
	return NULL;
}

void *
fifo_push(pfifo_t f, void *data) {
	pelmn_t e;
	if (!f)    return NULL;
	if (!(e = _elmn_new(data))) return NULL;
	if (f->last == NULL) f->first = e;
	else f->last->next = e;
	f->last = e;	
	
	return data;
}

void *
fifo_pop(pfifo_t f) {
	if (!f)        return NULL;
	if (!f->first) return NULL;
	void *p;
	pelmn_t e; 
	e = f->first;
	p = e->data;
	f->first = e->next;
	if (f->first == NULL) f->last = NULL;
	e = _elmn_destroy(e);
	return p;
}

int
fifo_is_empty(pfifo_t f) {
	if (!f || !f->first) return 1;
	return 0;
}

void
fifo_dump(pfifo_t f, elmn2str_f elmn2str) {
	printf("\nFifo dump %p:\n", f);
	if (!f) {
		printf("null fifo ptr\n");
		return;
	}
	pelmn_t e;

	printf("first: %x, last: %x\n", f->first, f->last);
	
	int i;
	for (i = 0, e = f->first; e != NULL; e = e->next, i++) {
		printf("% 3d: %x data = %x, next = % 9x", i, e, e->data, e->next);
		if (elmn2str != NULL) elmn2str(e->data);
		printf("\n"); 
	}	
}

#if defined(_test_fifo_)

void data_dump(void *p) {
	printf(" (data =	% 3d)", *(int *)p);
}

#include <stdio.h>
int main(void) {
	pfifo_t f;

	int i;
	int ints[10];
	for (i = 0; i < 10; i++) ints[i] = i + 1;

	if (!(f = fifo_new())) return 1;
 
	fifo_push(f, (void *) (ints + 2));
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 5));
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 7));
fifo_dump(f, data_dump);
	fifo_pop(f);
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 9));
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 1));
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 3));
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 4));
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 8));
fifo_dump(f, data_dump);
	fifo_push(f, (void *) (ints + 0));
fifo_dump(f, data_dump);

	printf("expecting: 6 8 10 2 4 5\n");
	printf("having   : ");
	while (!(fifo_is_empty(f))) {
		int *p;
		p = fifo_pop(f);
		printf("%d ", *p);
		if (*p == 5) {
			printf("\n");
			break;
		}
	}	
	printf("\n");

	fifo_destroy(f);
	return 0;
}
#endif
