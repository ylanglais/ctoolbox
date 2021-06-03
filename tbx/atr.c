
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    12/03/2002  1.0 Creation
	30/05/2016	3.1	Corrections
*/   

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "mem.h"

char atr_MODULE[]  = "Alphanumeric atree";
char atr_PURPOSE[] = "Ascii tree limited to [A-Z][a-z][0-9] strings";
char atr_VERSION[] = "3.1";
char atr_DATEVER[] = "30/05/2016";

/* */
/*
	[A-z0-9_];
	==> 26 * 2 + 10 + 1 = 63;
	
	0    |    1    |    2    |    3    |    4    |    5    |    6    |    7
	01234567890123456789012345678901234567890123456789012345678901234567890
	0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_
              ^                         ^                         ^
*/

 static int
atr_char2index(char c) {
	static char _atr_tt_[] =  {
		/*      0   1   2   3   4   5   6   7  */
		/*0*/  63, 63, 63, 63, 63, 63, 63, 63,
		/*1*/  63, 63, 63, 63, 63, 63, 63, 63,
		/*2*/  63, 63, 63, 63, 63, 63, 63, 63,
		/*3*/  63, 63, 63, 63, 63, 63, 63, 63,
		/*4*/  63, 63, 63, 63, 63, 63, 63, 63,
		/*5*/  63, 63, 63, 63, 63, 63, 63, 63,
		/*6*/   0,  1,  2,  3,  4,  5,  6,  7,
		/*7*/   8,  9, 63, 63, 63, 63, 63, 63,
		/*8*/  63, 10, 11, 12, 13, 14, 15, 16,
		/*9*/  17, 18, 19, 20, 21, 22, 23, 24,
		/*A*/  25, 26, 27, 28, 29, 30, 31, 32,
		/*B*/  33, 34, 35, 63, 63, 63, 63, 62, /** <--- '_' = 62!!! **/
		/*C*/  63, 36, 37, 38, 39, 40, 41, 42,
		/*D*/  43, 44, 45, 46, 47, 48, 49, 50,
		/*E*/  51, 52, 53, 54, 55, 56, 57, 58,
		/*F*/  59, 60, 61, 63, 63, 63, 63, 63
	};
	return _atr_tt_[(int) c];
}

 static char 
atr_index2char(int i) {
	static char _atr_tt_[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_ ";
	return 	_atr_tt_[i];
}
	
typedef struct _atr_t {
	struct _atr_t *a[64];
	void          *data;
} atr_t, *patr_t;


#define _atr_c_
#include "atr.h"
#undef  _atr_c_

size_t atr_sizeof() { return sizeof(atr_t); }

patr_t 
atr_new() {
	return (patr_t) mem_zmalloc(sizeof(atr_t));
}

patr_t 
atr_destroy(patr_t p) {
	register int i;
	if (!p) return NULL;
	/* destroy subtree */
	for (i = 0; i < 64; i++) atr_destroy(p->a[i]);
	/* free memory */
	mem_free(p);
	return NULL;
}

void *
atr_retrieve(register patr_t p, register char *key) {
	register int i;	
	if (!p || !key) return NULL;
	while (*key && p) {
		i = atr_char2index(*key);
		if (i >= 0 && i < 63) p = p->a[i];
		key++;
	}
	if (!*key && p) return p->data;
	return NULL;
}

void *
atr_store(register patr_t p, register char *key, register void *data) {
	register int i;
	if (!p | !key | !data) return NULL;
	while (*key) {
		i = atr_char2index(*key);
		if (i >= 0 && i < 63) {
			if (!p->a[i] && !(p->a[i] = (patr_t) mem_zmalloc(sizeof(atr_t)))) return NULL;
			p = p->a[i];
		}
		key++;
	}
	return p->data = data;
}

static void 
_atr_foreach_(patr_t p, f_hook_t f_hook, int level, char *string) {
	int i;
	if (!p) return;
	if (p->data) { 
		string[level] = 0;
		f_hook(level, string, p->data);
	}
	for (i = 0; i < 64; i++) {
		if (p->a[i]) { 
			string[level] = atr_index2char(i);
			_atr_foreach_(p->a[i] , f_hook, level + 1, string);
		}
	}
}

void
atr_foreach(patr_t p, f_hook_t f_hook) {
	char str[1024];
	_atr_foreach_(p, f_hook, 0, str);
}

static void 
_atr_dump_(patr_t p, f_dumpdata_t f_dumpdata, int level, char *string) {
	int i;
	if (p) { 
		if (p->data) { 
			string[level] = 0;
			printf("%4d: %-20s --> ", level, string); 
			f_dumpdata(p->data); printf("\n");
		}
		for (i = 0; i < 64; i++) {
			if (p->a[i]) { 
				string[level] = atr_index2char(i);
				_atr_dump_(p->a[i], f_dumpdata, level + 1, string);
			}
		}
	}
}

void
atr_dump(patr_t p, f_dumpdata_t f_dumpdata) {
	char str[1024];
	_atr_dump_(p, f_dumpdata, 0, str);
}

#if defined(_test_atr_)
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
	patr_t pa;
	pstorage_t ps;
	char *cols[752];
	int i;
	char name[128];
	FILE *pf;

	printf("atr test\n");

	ps = storage_new(sizeof(rgb_t), 100);
	pa = atr_new();
	
	pf = fopen(RGBFILE, "r");
	printf("store rgb data from %s\n", RGBFILE);
	
	i = 0;
	while (!feof(pf)) {
		rgb_t rgb;
		memset(name, 0, 128);
		if (fscanf(pf, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, name) == 4) { 
			cols[i] = mem_strdup(name);
			atr_store(pa, cols[i], (void *) storage_add(ps, (char *) &rgb)); 
			i++;
		}
		/* printf("*********\n%d\n", i); */
		/* atr_dump(pa, rgb_print); */
	}
	fclose(pf);

	printf("\n*********\natr dump:\n");
	atr_dump(pa, rgb_print);

	/* final chech: */
	printf("\n*********\nretrieval\n");
	for (i = 0; i < storage_used(ps); i++) {
		prgb_t prgb0, prgb1;
		prgb0 = (prgb_t) storage_get(ps, i);
		prgb1 = (prgb_t) atr_retrieve(pa, cols[i]);
		printf("%d --> %s: (%d, %d, %d) /// (%d, %d, %d)\n", 
				   i, cols[i], 
				   prgb0->r, prgb0->g, prgb0->b,
				   prgb1->r, prgb1->g, prgb1->b);
		mem_free(cols[i]);
	}
	storage_destroy(ps);
	atr_destroy(pa);
	return 0;
}
		
#endif
