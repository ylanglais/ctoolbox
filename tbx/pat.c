
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    12/03/2002  1.0 Creation
	03/08/2016  1.1 Bugfix if lenght de spec > 63
*/   

#include <stdio.h>
#include <strings.h>
#include <string.h>

#include "mem.h"

char pat_MODULE[]  = "Programmable ascii tree";
char pat_PURPOSE[] = "Dictionny based on a user defined ascii subset entries";
char pat_VERSION[] = "1.1";
char pat_DATEVER[] = "03/08/2016";

/* */
typedef struct _pat_t {
	struct _pat_t *root;
	int		len;
	char   *c2i;
	char   *i2c;
	void   *data;
	struct _pat_t **a;
} pat_t, *ppat_t;

#define _pat_c_
#include "pat.h"
#undef  _pat_c_

int
pat_char2index(ppat_t p, char c) {
	if (!p || !p->c2i) return 0;
	return p->c2i[(int) c];
}

char 
pat_index2char(ppat_t p, int i) {
	if (!p || !p->i2c || i < 0 || i > 255 ) return 0; 
	return p->i2c[i];
}
	
size_t pat_sizeof() { return sizeof(pat_t); }

ppat_t 
pat_new(char *spec) {
	ppat_t p;

	int i;
	if (!(p = (ppat_t) mem_zmalloc(sizeof(pat_t) + strlen(spec)))) return NULL;

	if (!(p->i2c = strdup(spec))) return pat_destroy(p);
	p->len = strlen(spec);
	if (!(p->c2i = mem_zmalloc(256 * sizeof(int)))) return pat_destroy(p);;
	//printf("c2i:\n");
	for (i = 0; i < p->len; i++) {
		p->c2i[(int) p->i2c[i]] = i;
		//printf("%c -> %d\n", p->i2c[i], i);
	} 
	/* allocate 1st level array: */
	if (!(p->a = (ppat_t *) mem_zmalloc(p->len * sizeof(ppat_t)))) return pat_destroy(p);

	return p;
}

ppat_t 
pat_destroy(ppat_t p) {
	register int i;
	if (!p) return NULL;
	if (!p->root) {
		if (p->c2i) {
			free(p->c2i);
			p->c2i = NULL;
		}
		if (p->i2c) {
			free(p->i2c);
			p->i2c = NULL;
		}
		
	}
	/* destroy subtree */
	for (i = 0; i < p->len; i++) pat_destroy(p->a[i]);
	/* free memory */
	mem_free(p->a);
	mem_free(p);
	return NULL;
}

void *
pat_retrieve(register ppat_t p, register char *key) {
	register int i;	
	if (!p || !key) return NULL;
	while (*key && p) {
		i = pat_char2index(p, *key);
		if (i >= 0 && i < p->len) p = p->a[i];
		key++;
	}
	if (!*key && p) return p->data;
	return NULL;
}

void *
pat_store(register ppat_t p, register char *key, register void *data) {
	register int i;
	if (!p | !key | !data) return NULL;
	while (*key) {
		i = pat_char2index(p, *key);
		if (i >= 0 && i < p->len) {
			if (!p->a[i]) {
				if (!(p->a[i]    = (ppat_t) mem_zmalloc(sizeof(pat_t))))
					return NULL;
				if (!(p->a[i]->a = (ppat_t *) mem_zmalloc(p->len * sizeof(ppat_t)))) {
					p->a[i] = mem_free(p->a[i]);
					return NULL;
				}
			}
			p->a[i]->len = p->len;
			p->a[i]->c2i = p->c2i; 
			p->a[i]->i2c = p->i2c; 
			p->a[i]->root = p;
			p = p->a[i];
		}
		key++;
	}
	return p->data = data;
}

static void 
_pat_foreach_(ppat_t p, f_hook_t f_hook, int level, char *string) {
	int i;
	if (!p) return;
	if (p->data) { 
		string[level] = 0;
		f_hook(level, string, p->data);
	}
	for (i = 0; i < p->len; i++) {
		if (p->a[i]) { 
			string[level] = pat_index2char(p, i);
			_pat_foreach_(p->a[i] , f_hook, level + 1, string);
		}
	}
}

void
pat_foreach(ppat_t p, f_hook_t f_hook) {
	char str[1024];
	_pat_foreach_(p, f_hook, 0, str);
}

static void 
_pat_dump_(ppat_t p, f_dumpdata_t f_dumpdata, int level, char *string) {
	int i;
	if (p) { 
		if (p->data) { 
			string[level] = 0;
			printf("%4d: %-20s --> ", level, string); 
			f_dumpdata(p->data); printf("\n");
		}
		for (i = 0; i < p->len; i++) {
			if (p->a[i]) { 
				string[level] = pat_index2char(p, i);
				_pat_dump_(p->a[i], f_dumpdata, level + 1, string);
			}
		}
	}
}

void
pat_dump(ppat_t p, f_dumpdata_t f_dumpdata) {
	char str[1024];
	_pat_dump_(p, f_dumpdata, 0, str);
}

#if defined(_test_pat_)
#include <stdio.h>
#include "storage.h"
#define RGBFILE "rgb.txt"

typedef struct {
	int r, g, b;
} rgb_t, *prgb_t;

void rgb_print(void *p) {
	prgb_t pr;
	pr = (prgb_t) p;
	printf("(%03d, %03d, %03d)", pr->r, pr->g, pr->b);
}

int main(void) {
	ppat_t pa;
	pstorage_t ps;
	char *cols[752];
	int i;
	char name[128];
	FILE *pf;

	printf("pat test\n");

	ps = storage_new(sizeof(rgb_t), 100);
	pa = pat_new("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_ ");
	
	pf = fopen(RGBFILE, "r");
	printf("store rgb data from %s\n", RGBFILE);
	
	i = 0;
	while (!feof(pf)) {
		rgb_t rgb;
		memset(name, 0, 128);
		if (fscanf(pf, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, name) == 4) { 
			cols[i] = mem_strdup(name);
			pat_store(pa, cols[i], (void *) storage_add(ps, (char *) &rgb)); 
			i++;
		}
		/* printf("*********\n%d\n", i); */
		/* pat_dump(pa, rgb_print); */
	}
	fclose(pf);

	printf("\n*********\npat dump:\n");
	pat_dump(pa, rgb_print);

	/* final chech: */
	printf("\n*********\nretrieval\n");
	for (i = 0; i < storage_used(ps); i++) {
		prgb_t prgb0, prgb1;
		prgb0 = (prgb_t) storage_get(ps, i);
		prgb1 = (prgb_t) pat_retrieve(pa, cols[i]);
		if (!prgb0) {
			printf("cannot retreive color %d from storage", i);
		} else if (!prgb1) {
			printf("cannot retreive color '%s' from atr", cols[i]);
		} else {
			printf("%d --> %s: (%d, %d, %d) /// (%d, %d, %d)\n", 
				   i, cols[i], 
				   prgb0->r, prgb0->g, prgb0->b,
				   prgb1->r, prgb1->g, prgb1->b);
		}
		mem_free(cols[i]);
	}
	storage_destroy(ps);
	pat_destroy(pa);
	return 0;
}
		
#endif
