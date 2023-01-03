

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    23/04/2010  1.0 initial version
	26/10/2017  1.1 add pstore_set
	30/10/2017  1.2 bugfix
	03/01/2023  1.3 bugfix (messy reentrance)
*/   

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#ifdef __REENTRANT__
#include <pthread.h>
#define pstore_lock(x)   {if (x && x->mp) pthread_mutex_lock((x)->mp);}
#define pstore_unlock(x) {if (x && x->mp) pthread_mutex_unlock((x)->mp);}
#else
#define pstore_lock(x)
#define pstore_unlock(x)
#endif
#include "pal.h"
#include "err.h"
#include "fmt.h"
#include "mem.h"

char pstore_MODULE[]  = "Simple persistant storage management";
char pstore_PURPOSE[] = "Simple persistant storage management";
char pstore_VERSION[] = "1.3";
char pstore_DATEVER[] = "03/01/2023";

typedef struct {
	void   *base;
	size_t unit;
	size_t grain;
	int    used;
	int    allocated;
	int    pages;
    #ifdef __REENTRANT__
    pthread_mutex_t  *mp;
    #endif  
	char  *data;
} pstore_t, *ppstore_t;

#define _pstore_c_
#include "pstore.h"
#undef  _pstore_c_

ppstore_t pstore_destroy(ppstore_t ps) {
	if (ps) {
		#ifdef __REENTRANT__
		if (ps->mp) {
			pthread_mutex_destroy(ps->mp);
			free(ps->mp);
		}
		#endif  
		pfree(ps);
	}
	return NULL;
}

size_t pstore_sizeof() { return sizeof(pstore_t); }

void pstore_dump(ppstore_t ps) {
	if (!ps) return;
	err_debug("Persistant storage:");
	err_debug("base     : %x", ps->base);
	err_debug("unit     : %u", ps->unit);
	err_debug("grain    : %u", ps->grain);
	err_debug("used     : %d", ps->used);
	err_debug("allocated: %d", ps->allocated);
	err_debug("pages    : %d", ps->pages);
    #ifdef __REENTRANT__
    err_debug("mutex    : %x", ps->mp);
    #endif  
	err_debug("data     : %x", ps->data);

	char *bin = NULL;
	if (!(bin = fmt_bin((char *) &(ps->data), ps->allocated * ps->unit))) {
		err_debug("Cannot display data");
	} else {
		err_debug("Data content\n%s", bin);
		if (bin) free(bin);
	} 
}

ppstore_t pstore_new(char *filename, size_t unit, size_t grain) {
	ppstore_t ps;

	if (!filename) return NULL;
	if (!unit || !grain) return NULL;

	/* try load data first: */
	if (!(ps = (ppstore_t) pmalloc(filename,  pstore_sizeof() + (unit * grain)))) 
		return NULL;

    #ifdef __REENTRANT__
	if (!(ps->mp = (pthread_mutex_t *) mem_zmalloc(sizeof(pthread_mutex_t)))) {
		return pstore_destroy(ps);
	}
	#endif

	if (!ps->unit) {
		ps->unit  = unit;
		ps->grain = grain;
		ps->pages = 1;
		ps->used  = 0;
		ps->allocated = grain;
	}
	ps->base = &(ps->data);
	
	return ps;
}

int pstore_morecore(ppstore_t ps) {
	char *p;

	if (!ps) return 1;
	if (ps->used < ps->allocated - 1) return 0;

	/* Need more room! 
	 * => resize file:
	 */
	
	/* resize page array: */
	if (!(p = (char *) prealloc((void *) ps, pstore_sizeof() + (ps->pages + 1) * ps->unit * ps->grain))) {
		pstore_unlock(ps);
		return 2;
	}
	ps->allocated += ps->grain;
	ps->pages++;
	return 0;
}

char *pstore_alloc(ppstore_t ps) {
	char *p;

	if (!ps) return NULL;
	pstore_lock(ps);	
	if (pstore_morecore(ps)) {
		pstore_unlock(ps);
		return NULL;
	}

	p = (char *) &(ps->data) + (ps->used++ * ps->unit);
	pstore_unlock(ps);
	return p;
}

char *pstore_add(ppstore_t ps, char *pdata) {
	char *p;
	if (!ps) return NULL;
	if (!(p = pstore_alloc(ps))) return NULL;
	memcpy((void *) p, (void *) pdata, ps->unit);
	return p;
}

char * pstore_get(ppstore_t ps, int i) {
	if (!ps) return NULL;
	if (i < 0 || i > ps->used) return NULL;
	return (char *) &(ps->data) + (i * ps->unit);
}

char *pstore_set(ppstore_t ps, int i, char *pdata) {
	char *p;
	if (!ps) return NULL;
	if (i < 0 || i > ps->used) return NULL;
	p =  (char *) (&(ps->data)) + (i * ps->unit);
	memcpy(p, pdata, ps->unit);
	return p;
}

int pstore_used(ppstore_t ps) {
	if (ps) return ps->used;
	return -1;
}

int pstore_allocated(ppstore_t ps) {
	if (ps) return ps->allocated;
	return -1;
}

int pstore_available(ppstore_t ps) {
	if (ps) return ps->allocated - ps->used;
	return -1;
}

int pstore_grain(ppstore_t ps) {
	if (ps) return ps->grain;
	return -1;
}

int pstore_unit(ppstore_t ps) {
	if (ps) return ps->unit;
	return -1;
}

int pstore_pages(ppstore_t ps) {
	if (ps) return ps->pages;
	return -1;
}

#ifdef _test_pstore_
#include <stdio.h>

typedef struct {
	char name[30];
	int  amount;
	char code[6];
} tstpst_t, *ptstpst_t;

void
tstpst_print(ptstpst_t p) {
	printf("name  : %s\namount: %d\ncode  : %5s\n", p->name, p->amount, p->code);
}

void
tstpst_dmp(ppstore_t ps) {
	int i;
	for (i = 0; i < pstore_used(ps); i++) tstpst_print((ptstpst_t) pstore_get(ps, i));
}
extern void pal_dump(void *p);
int main(void) {
	int i;
	ppstore_t ps;

	err_init(NULL, err_DEBUG);
	//err_init(NULL, err_WARNING);

	for (i = 0; i < 2; i++) {

		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>\nrun %d\n", i);

		if (!(ps = pstore_new("test-pstore.data", sizeof(tstpst_t), 1))) {
			printf("cannot create a peristant storage\n");
			return 1;
		}

		if (ps->used == 0) {
			tstpst_t p, *q;
			printf("--> storage is empty, populate it\n");

	pal_dump(ps);		
	pstore_dump(ps);		
			memset((char *) &p, 0, sizeof(tstpst_t));

			strcpy(p.name, "one");
			p.amount = 1;
			strcpy(p.code, "id1");
			if (!(q = (ptstpst_t) pstore_add(ps, (void *) &p))) printf("cannot add 1\n");
	pal_dump(ps);		
	pstore_dump(ps);		

			strcpy(p.name, "two");
			p.amount = 2;
			strcpy(p.code, "id2");
			if (!(q = (ptstpst_t) pstore_add(ps, (void *) &p))) printf("cannot add 2\n");
	pal_dump(ps);		
	pstore_dump(ps);		
		
			strcpy(p.name, "three");
			p.amount = 3;
			strcpy(p.code, "id3");
			if (!(q = (ptstpst_t) pstore_add(ps, (void *) &p))) printf("cannot add 3\n");
	pal_dump(ps);		

			printf("--> first dump:\n");
			tstpst_dmp(ps);
			pstore_dump(ps);

			printf("--> get slot #1:\n");
			tstpst_print((ptstpst_t) pstore_get(ps, 1));

			printf("--> Change second slot of data\n");
			strcpy(p.name, "deux");
			p.amount = 2;
			strcpy(p.code, "deux");
			pstore_set(ps, 1, (void *) &p);

			printf("--> second dump:\n");
			tstpst_dmp(ps);
			pstore_dump(ps);

		} else {
			printf("--> storage is already populated\n");	
			printf("--> check pstore content:\n");
			tstpst_dmp(ps);
			pstore_dump(ps);
		}

		printf("--> free pstore\n"); 

		pstore_destroy(ps);
	}
	return 0;	
}

#endif
