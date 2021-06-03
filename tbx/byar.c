#include <stdlib.h>
#include <string.h>

#include "mem.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		30/05/2016 1.1 	Adding changelog

*/

char byar_MODULE[]  = "BinarY ARray";
char byar_PURPOSE[] = "Manage a binary array for implementing network protocols";
char byar_VERSION[] = "1.1";
char byar_DATEVER[] = "30/05/2016";

typedef struct {
	size_t size;
	char *pos;
	char *buffer;
} byar_t, *pbyar_t;

#define _byar_c_
#include "byar.h"
#undef  _byar_c_

static int
byar_buffer_resize(pbyar_t b, int len) {
	if (!b)   return 1;
	if (!len) return 2;
	if (!b->buffer) {
		if (!(b->buffer = malloc(len))) {
			return 3;
		} else {
			b->pos  = b->buffer;	
			b->size = len;
		}
	} else {
		char *p;
		if (!(p = realloc(b->buffer, b->size + len))) return 4;
		b->buffer  = p;
		b->size   += len;
	}
	return 0;
}

pbyar_t
byar_new() {
	pbyar_t b;

	if (!(b = mem_zmalloc(sizeof(byar_t)))) {
		return NULL;
	}
	b->size  = 0;
	b->pos   = b->buffer = NULL;
	return b;	
}

pbyar_t
byar_destroy(pbyar_t b) {
	if (b) {
		if (b->buffer) b->buffer = mem_free(b->buffer);
		free(b);
	}
	return b;
}

pbyar_t 
byar_new_from_data(char *data, size_t len) {
	pbyar_t b;
	if (!data || !len) 		return NULL;
	if (!(b = byar_new()))  return NULL;
	if (byar_buffer_resize(b, len)) {
		return b;
	}
	memcpy(data, b->buffer, len);
	b->pos += len;
	return b;
}

int
byar_data_get(pbyar_t b, char **data, size_t *size) {
	if (!b) {
		*data = NULL; *size = 0;
		return 1;
	} 
	*data = b->buffer;
	*size = b->size;
	return 0;
}

int
byar_push(pbyar_t b, int len, char *data) {
	if (!b || !len || !data) return 1;
	if (byar_buffer_resize(b, len)) return 2;
	memcpy(b->pos, data, len);
	b->pos += len;
	return 0;
}

char *
byar_pop(pbyar_t b, int len, char *data) {
	if (!b || !b->buffer) return NULL;

	if (b->pos >= (b->buffer + b->size + len))  return NULL;
	if (b->pos - len < b->buffer)  return NULL;

	b->pos -= len;
	memcpy(data, b->pos, len);
	return data;
}

int
byar_rewind(pbyar_t b) {
	if (!b) return 1;
	if (!b->buffer) return 2;
	b->pos = b->buffer;
	return 0;	
}

#ifdef _test_byar_
#include <stdio.h>
#include "fmt.h"

#define str_size(x)    (strlen(x)), (x)
void
byar_dump(pbyar_t b) {
	if (!b) {
		printf("Null byte array\n");		
		return;
	}
	if (b->size == 0) {
		printf("Empty byte array\n");
		return;
	} 
	printf("buffer: %p\nsize: %lu\n", b->size);
	printf("%s\n", fmt_bin(b->buffer, b->size));
}

int main(void) {
	pbyar_t ya;
	int           i = 11;
	double        d = 5.555e3;
	unsigned long u = 1024;
	//char          c[50];
	
	ya = byar_new();

	byar_push(ya, str_size("int"));
	byar_dump(ya);
	byar_push(ya, sizeof(int), (char *) &i);
	byar_dump(ya);
	byar_push(ya, str_size("double"));
	byar_dump(ya);
	byar_push(ya, sizeof(double), (char *) &d);
	byar_dump(ya);
	byar_push(ya, str_size("ulong"));
	byar_dump(ya);
	byar_push(ya, sizeof(unsigned long), (char *) &u);
	byar_dump(ya);
	byar_pop(ya, sizeof(unsigned long), (char *) &u);
	printf("pop %lu => %lu\n", sizeof(unsigned long), u);
	byar_dump(ya);
	
	byar_destroy(ya);
	return 0;

}

#endif
