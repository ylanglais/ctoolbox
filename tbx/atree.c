
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    12/03/2002  1.0	Creation
    30/05/2016  4.1	adding revision log
*/   

#include <stdio.h>
#include <strings.h>
#include <string.h>

#include "mem.h"

char atree_MODULE[]  = "Ascii tree";
char atree_PURPOSE[] = "Ascii tree";
char atree_VERSION[] = "4.1";
char atree_DATEVER[] = "30/05/2016";

typedef struct _atree_t {
	struct _atree_t *a[256];
	void             *data;
} atree_t, *patree_t;

#define _atree_c_
#include "atree.h"
#undef  _atree_c_

size_t atree_sizeof() { return sizeof(atree_t); }

patree_t 
atree_new() {
	return (patree_t) mem_zmalloc(sizeof(atree_t));
}

patree_t 
atree_destroy(patree_t p) {
	int i;
	if (!p) return NULL;
	/* destroy subtree */
	for (i = 0; i < 256; i++) atree_destroy(p->a[i]);
	/* free memory */
	mem_free(p);
	return NULL;
}

void *
atree_retrieve(patree_t p, char *key) {
	if (!p || !key) return NULL;
	while (*key && p) {
		p = p->a[(int) *key];
		key++;
	}
	if (!*key && p) return p->data;
	return NULL;
}

void *
atree_store(patree_t p, char *key, void *data) {
	if (!p | !key | !data) return NULL;
	while (*key) {
		if (!p->a[(int) *key] && !(p->a[(int) *key] = (patree_t) mem_zmalloc(sizeof(atree_t)))) return NULL;
		p = p->a[(int) *key];
		key++;
	}
	return p->data = data;
}

static void 
_atree_foreach_(patree_t p, f_hook_t f_hook, int level, char *string) {
	int i;
	if (!p) return;
	if (p->data) { 
		string[level] = 0;
		f_hook(level, string, p->data);
	}
	for (i = 0; i < 256; i++) {
		if (p->a[i]) { 
			string[level] = i;
			_atree_foreach_(p->a[i] , f_hook, level + 1, string);
		}
	}
}

void
atree_foreach(patree_t p, f_hook_t f_hook) {
	char str[1024];
	_atree_foreach_(p, f_hook, 0, str);
}

static void 
_atree_dump_(patree_t p, void (*f_dumpdata)(void *), int level, char *string) {
	int i;
	if (p) { 
		if (p->data) { 
			string[level] = 0;
			printf("%4d: %-20s --> ", level, string); 
			f_dumpdata(p->data); printf("\n");
		}
		for (i = 0; i < 256; i++) {
			if (p->a[i]) { 
				string[level] = i;
				_atree_dump_(p->a[i], f_dumpdata, level + 1, string);
			}
		}
	}
}

void
atree_dump(patree_t p, void (*f_dumpdata)(void *)) {
	char str[1024];
	_atree_dump_(p, f_dumpdata, 0, str);
}

#if defined(_test_atree_)
#include <stdio.h>
#include "storage.h"
#define RGBFILE "rgb.txt"


typedef struct {
	int r, g, b;
}rgb_t, *prgb_t;

void rgb_print(void *p) {
	prgb_t pr;
	pr = (prgb_t) p;
	printf("(%03d, %03d, %03d)", pr->r, pr->g, pr->b);
}

int main(void) {
	patree_t pa;
	pstorage_t ps;
	char *cols[752];
	int i;
	char name[128];
	FILE *pf;

	printf("atree test\n");

	ps = storage_new(sizeof(rgb_t), 100);
	pa = atree_new();
	
	pf = fopen(RGBFILE, "r");
	printf("store rgb data from %s\n", RGBFILE);
	
	i = 0;
	while (!feof(pf)) {
		rgb_t rgb;
		memset(name, 0, 128);
		if (fscanf(pf, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, name) == 4) { 
			cols[i] = strdup(name);
			atree_store(pa, cols[i], (void *) storage_add(ps, (char *) &rgb)); 
			i++;
		}
		/* printf("*********\n%d\n", i); */
		/* atree_dump(pa, rgb_print); */
	}
	fclose(pf);

	printf("\n*********\natree dump:\n");
	atree_dump(pa, rgb_print);

	/* final chech: */
	printf("\n*********\nretrieval\n");
	for (i = 0; i < storage_used(ps); i++) {
		prgb_t prgb0, prgb1;
		prgb0 = (prgb_t) storage_get(ps, i);
		prgb1 = (prgb_t) atree_retrieve(pa, cols[i]);
		/* if (prgb0->r != prgb1->r || prgb0->g != prgb1->g || prgb0->b != prgb1->b) { */
			printf("%d --> %s: (%d, %d, %d) /// (%d, %d, %d)\n", 
				   i, cols[i], 
				   prgb0->r, prgb0->g, prgb0->b,
				   prgb1->r, prgb1->g, prgb1->b);
		/* } */
		mem_free(cols[i]);
	}
	storage_destroy(ps);
	atree_destroy(pa);
	return 0;
}
		
#endif
