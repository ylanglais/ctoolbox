
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		10/05/2006	1.0	Creation
    
*/   

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#ifdef __REENTRANT__
#include <pthread.h>
#define dyna_lock(x)   pthread_mutex_lock((x)->mp)
#define dyna_unlock(x) pthread_mutex_unlock((x)->mp)
#else
#define dyna_lock(x)
#define dyna_unlock(x)
#endif
#include "mem.h"

char dyna_MODULE[]  = "Pointers management (dynamic dynamic)";
char dyna_PURPOSE[] = "Dynamic object arrays with automatic memory allocation/deallocation";
char dyna_VERSION[] = "1.0";
char dyna_DATEVER[] = "10/05/2006";

typedef struct _slot_t {
	struct _slot_t *next;
	char           *slot;
} slot_t, *pslot_t;

/*
 	zones de stockage;
	zone index;
 */

typedef struct {
	size_t unit;
	size_t grain;
	int    used;
	int    allocated;
	int    pages;
	char **index;
	char **data;
	pslot_t free;
    #ifdef __REENTRANT__
    pthread_mutex_t  *mp;
    #endif  
} dyna_t, *pdyna_t;

#define _dyna_c_
#include "dyna.h"
#undef  _dyna_c_

static pslot_t
slot_new() {
	return (pslot_t) mem_zmalloc(sizeof(slot_t));
}

static pslot_t 
slot_destroy(pslot_t f) {
	if (f) {
		//if (f->slot) free(f->slot);
		free(f);
	}
	return NULL;
}

#define __dyna_cc__
#include "dyna.h"
#undef  __dyna_cc__

pdyna_t 
dyna_destroy(pdyna_t da) {
	int i;
	pslot_t f;
	if (da) { 
		dyna_lock(da);
		/* free all pages: */
		for (i = 0; i < da->pages; i++) if (da->data[i]) mem_free(da->data[i]);
		f = da->free;

		while (f) {
			da->free = f->next;
			slot_destroy(f);
			f = da->free;
		}
		
		/* free index: */
		mem_free(da->index);

		/* free page dyna: */
		mem_free(da->data);
		dyna_unlock(da);
		#ifdef __REENTRANT__
		pthread_mutex_destroy(da->mp);
		mem_free(da->mp);
		#endif
		
		mem_free(da);
	}
	return NULL;
}

size_t dyna_sizeof() { return sizeof(dyna_t); }

pdyna_t 
dyna_new(size_t unit, size_t grain) {
	pdyna_t da;
	if (!(da =          (pdyna_t) mem_zmalloc(sizeof(dyna_t))))           return NULL;
	if (!(da->data    = (char **) mem_zmalloc(sizeof(char *))))           return dyna_destroy(da);
	if (!(da->data[0] = (char *)  mem_zmalloc(unit * grain)))             return dyna_destroy(da);
	if (!(da->index   = (char **) mem_zmalloc(sizeof(char *) * grain)))   return dyna_destroy(da);

	da->grain = grain;
	da->unit  = unit;
	da->used  = 0;
	da->pages = 1;
	da->allocated = grain;
	#ifdef __REENTRANT__
	da->mp = (pthread_mutex_t *) mem_zmalloc(sizeof(pthread_mutex_t));
	#endif
	return da;
}

static int
dyna_alloc_more(pdyna_t da) {
	char *p, **pi;

	/* Need more room! 
	 * Room extension CANNOT be done by resizing since realloc may change dyna location.
	 * Thus, in this case,  pointers refering to stored data within the dyna area
	 * would point to anywhere but expected data!!!
	 * Instead, room extension can be done by resizing a page dyna and allocate a 
	 * new page of grain * unit bytes. This way, we are sure that allocated areas will
	 * not be moved.
	 */
	dyna_lock(da);

	/* resize page dyna: */
	if (!(p = (char *) mem_realloc((char *) da->data, (da->pages + 1) * sizeof(char *)))) {
		dyna_unlock(da);
		return 1;
	}
	if (!(pi = (char **) mem_realloc((char **) da->index, (da->allocated + da->grain) * sizeof(char *)))) {
		/* try to come back to previous allocated area: */
		p = (char *) mem_realloc((char *) da->data, da->pages * sizeof(char *));

		dyna_unlock(da);
		return 2;
	}
	da->pages++;
	da->data  = (char **) p;
	da->index = (char **) pi;

	/* allocate a new page at the end of the page dyna: */
	if (!(da->data[da->pages - 1] = (char *) mem_zmalloc(da->grain * da->unit))) {
		da->pages--;
		p  = (char *)  mem_realloc((char *)  da->data,  da->pages     * sizeof(char *));
		pi = (char **) mem_realloc((char **) da->index, da->allocated * sizeof(char *)); 
		da->data  = (char **) p;
		da->index = (char **) pi;

		dyna_unlock(da);
		return 3;
	}
	da->allocated += da->grain;
	dyna_unlock(da);
	
	return 0;
}

static char *
dyna_next_free_slot(pdyna_t da) {
	char *p;

	/* see if we have a free slot: */
	if (da->free) {
		pslot_t f;

		/* yes, then use it: */
		f = da->free;
		p = f->slot; 
		
		/* update the free slot dyna: */
		da->free = da->free->next;

		/* destroy the slot: */
		slot_destroy(f);	

	} else {
		/* no more free slots, add at the end: */

		/* check if we need more memory: */
		if (da->used >= da->allocated - 1) if (dyna_alloc_more(da)) return NULL;

		/* retrieve last page: */
		p = da->data[da->used / da->grain];

		/* retrieve offset within the page: */
		p += (da->used % da->grain) * da->unit; 
	}
	return p;
}

char *
dyna_alloc(pdyna_t da) {
	char *p;
	
	if (!da) return NULL;

	/* Get next free slot : */
	if (!(p = dyna_next_free_slot(da))) {
		return NULL;
	}
	dyna_lock(da);	

	/* update index: */
	da->index[da->used] = p;

	/* update usage count: */
	da->used++;

	dyna_unlock(da);
	return p;
}

char *
dyna_add(pdyna_t da, char *pdata) {
	char *p;
	
	if (!da) return NULL;
	if (!(p = dyna_alloc(da))) return NULL;

	/* copy data to slot: */
	memcpy((void *) p, (void *) pdata, da->unit);
	
	return p;
}

char *
dyna_insert(pdyna_t da, int index, char *pdata) {
	char *p;

	if (!da) return NULL;
	if (index == -1 || index >= da->used) return dyna_add(da, pdata);
	
	/* Get next free slot : */
	if (!(p = dyna_next_free_slot(da))) {
		dyna_unlock(da);
		return NULL;
	}

	/* update the index dyna: */
	memmove(da->index + index + 1, da->index + index, (da->used - index) * sizeof(char *));

	/* copy data: */
	memcpy((void *) p, (void *) pdata, da->unit);

	da->index[index] = p;
		
	/* update usage count: */

	da->used++;
	dyna_unlock(da);
	return p;
}

char *
dyna_push(pdyna_t da, char *pdata) {
	return dyna_add(da, pdata);
}

char *
dyna_pop_first(pdyna_t da) {
	char *data;

	if (!dyna_get(da, 0))                         return NULL;
	if (!(data = (char *) malloc(dyna_unit(da)))) return NULL;
	memcpy(data, dyna_get(da, 0), dyna_unit(da)); 
	dyna_delete(da, 0);
		
	return data; 
}

char *
dyna_pop_last(pdyna_t da) {
	char *data;

	if (!dyna_get(da, dyna_count(da) - 1))        return NULL;
	if (!(data = (char *) malloc(dyna_unit(da)))) return NULL;
	memcpy(data, dyna_get(da, dyna_count(da) - 1), dyna_unit(da)); 
	dyna_delete(da, dyna_count(da) - 1);
	return data; 
}

int
dyna_delete_ptr(pdyna_t da, void *ptr) {
	int i;
	if (!da) return 1;
	for (i = 0; i < da->used; i++) if (da->index[i] == ptr) return dyna_delete(da, i);
	return 2;
}

int
dyna_delete(pdyna_t da, int index) {
	pslot_t ns;

	if (!da) 							return 1;
	if (index < 0 || index >= da->used) return 2;

	dyna_lock(da);
	
	/* cleanup slot: */ 
	
	memset(da->index[index], 0, da->unit);

	/* create a new freeslot.*/
	if (!(ns = slot_new())) {	
		dyna_unlock(da);
		return 3;	
	}

	/* push slot in free slot list : */
	ns->slot = da->index[index]; 								 
	ns->next = da->free;

//xXXXXXXx
	da->free = ns;
	
	/* update the index dyna: */
	memmove(da->index + index, da->index + index + 1, (da->used - index - 1) * sizeof(char *));

	da->used--;

	dyna_unlock(da);
	return 0;	
}
		
char * 
dyna_get(pdyna_t da, int i) {
	if (!da) return NULL;
	if (i < 0 || i >= da->used) return NULL;

	return (da->index[i]);
}

char *
dyna_set(pdyna_t da, int i, char *pdata) {
	if (!da) return NULL;
	if (i < 0 || i >= da->used) return NULL;
	memcpy((void *) da->index[i], (void *) pdata, da->unit);
	return  da->index[i];
} 

int dyna_count(pdyna_t da) {
	return dyna_used(da);
}

int 
dyna_used(pdyna_t da) {
	if (da) return da->used;
	return -1;
}

int 
dyna_allocated(pdyna_t da) {
	if (da) return da->allocated;
	return -1;
}

int 
dyna_available(pdyna_t da) {
	if (da) return da->allocated - da->used;
	return -1;
}

int 
dyna_grain(pdyna_t da) {
	if (da) return da->grain;
	return -1;
}

int 
dyna_unit(pdyna_t da) {
	if (da) return da->unit;
	return -1;
}

int 
dyna_pages(pdyna_t da) {
	if (da) return da->pages;
	return -1;
}

void *
dyna_data_ptr(pdyna_t da, int i) {
	if (da) return da->data[i];
	return NULL;
}

#ifdef _test_dyna_
typedef void (*print_cb)(char *);

void
dyna_dump_idx_seq(pdyna_t da, print_cb cb) {
	int i;
	for (i = 0; i < dyna_used(da); i++) {
		printf("%4d: ", i);
		cb(dyna_get(da,  i));
		printf("\n");
	}
} 

void
dyna_dump_ptr_seq(pdyna_t da, print_cb cb) {
	int ip, ig;
	for (ip = 0; ip < da->pages; ip++) for (ig = 0; ig < da->grain; ig++) {
		printf("page %2d offset %2d: ", ip, ig);
		cb(da->data[ip] + ig * da->unit);
		printf("\n");
	}
}

void
print_int(char *p) {
	int *pi;
	pi = (int *) p;
	printf("%d", *pi);
}

#define printadd(da, action, val)   {printf("%s %d\n", #action, val); action(da, (char *) &val); } 
#define printdel(da, action, index) {printf("%s %d\n", #action, index); action(da, index); }
#define printpop(da, action) {char *a = action(da); printf("%s %d\n", #action, *(int *)a); if (a) free(a); }
#define printins(da, action, index, val) {printf("%s %d at %d\n", #action, val, index); action(da, index, (char *) &val); } 
	
int
main() {
	int i;
	pdyna_t da;
	
	if (!(da = dyna_new(sizeof(int), 3))) return 1;
	
	i = 1; printadd(da, dyna_add, i);
	i = 2; printadd(da, dyna_add, i);
	i = 3; printadd(da, dyna_add, i);
	i = 4; printadd(da, dyna_add, i);
	i = 5; printadd(da, dyna_add, i);
	i = 6; printadd(da, dyna_add, i);
	i = 7; printadd(da, dyna_add, i);
	i = 8; printadd(da, dyna_add, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	printdel(da, dyna_delete, 3);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	printdel(da, dyna_delete, 6);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	i = 9;  printadd(da, dyna_add,  i);
	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	i = 10; printins(da, dyna_insert, 5, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	i = 11; printins(da, dyna_insert, 1, i);
	i = 12; printins(da, dyna_insert, 1, i);
	i = 13; printins(da, dyna_insert, 1, i);
	i = 14; printins(da, dyna_insert, 1, i);
	i = 15; printins(da, dyna_insert, 1, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	i = 16; printins(da, dyna_insert, 0, i);
	i = 17; printins(da, dyna_insert, 0, i);
	i = 18; printins(da, dyna_insert, 0, i);
	i = 19; printins(da, dyna_insert, 0, i);
	i = 20; printins(da, dyna_insert, 0, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	i = 7; printins(da, dyna_set, 15, i);
	i = 8; printins(da, dyna_set, 16, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	printdel(da, dyna_delete, 0);
	printdel(da, dyna_delete, 0);
	printdel(da, dyna_delete, 0);
	printdel(da, dyna_delete, 0);
	printdel(da, dyna_delete, 0);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	printdel(da, dyna_delete, 1);
	printdel(da, dyna_delete, 1);
	printdel(da, dyna_delete, 1);
	printdel(da, dyna_delete, 1);
	printdel(da, dyna_delete, 1);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	i = 10; printadd(da, dyna_add, i);
	i = 11; printadd(da, dyna_add, i);
	i = 12; printadd(da, dyna_add, i);
	i = 13; printadd(da, dyna_add, i);
	i = 14; printadd(da, dyna_add, i);
	i = 15; printadd(da, dyna_add, i);
	i = 16; printadd(da, dyna_add, i);
	i = 17; printadd(da, dyna_add, i);
	i = 18; printadd(da, dyna_add, i);
	i = 19; printadd(da, dyna_add, i);
	i = 20; printadd(da, dyna_add, i);

	i = 4; printins(da, dyna_insert, 3, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	dyna_delete_ptr(da, dyna_get(da, 5)); 
	
	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	dyna_destroy(da);

	if (!(da = dyna_new(sizeof(int), 3))) return 2;

	i = 1; printadd(da, dyna_push, i);
	i = 2; printadd(da, dyna_push, i);
	i = 3; printadd(da, dyna_push, i);
	i = 4; printadd(da, dyna_push, i);
	i = 5; printadd(da, dyna_push, i);
	
	printpop(da, dyna_pop_first);
	printpop(da, dyna_pop_first);
	printpop(da, dyna_pop_first);
	printpop(da, dyna_pop_first);
	printpop(da, dyna_pop_first);

	i = 1; printadd(da, dyna_push, i);
	i = 2; printadd(da, dyna_push, i);
	i = 3; printadd(da, dyna_push, i);
	i = 4; printadd(da, dyna_push, i);
	i = 5; printadd(da, dyna_push, i);
	
	printpop(da, dyna_pop_last);
	printpop(da, dyna_pop_last);
	printpop(da, dyna_pop_last);
	printpop(da, dyna_pop_last);
	printpop(da, dyna_pop_last);


	dyna_destroy(da);
	
	return 0;	
}

#endif
