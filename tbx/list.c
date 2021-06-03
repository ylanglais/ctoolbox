/*
	This code is released under GPL terms. 
	Please read license terms and conditions at http://www.gnu.org/

	Yann LANGLAIS

	Changelog:
	04/09/2001  2.0 fixes, use of the new memory allocator, ...
	11/03/2002	2.5 Make code reentrant w/ pthread_mutexes 	
	08/04/2004	3.0 reentrant fixes, interface cleaning & commenting.
*/

#include <stdlib.h>
#include <stdio.h>
#ifdef __REENTRANT__
#include <pthread.h> 
#define _list_lock_(x)   pthread_mutex_lock((x)->mp)
#define _list_unlock_(x) pthread_mutex_unlock((x)->mp)
#else 
#define _list_lock_(x) 
#define _list_unlock_(x) 
#endif
#include "mem.h"

char list_MODULE[]  = "Doubly linked list management";
char list_PURPOSE[] = "Doubly linked list management";
char list_VERSION[] = "3.0";
char list_DATEVER[] = "08/04/2004";

/* atom structure: */
typedef struct _atom_t {
	struct _atom_t *next;
	struct _atom_t *prev;
	void *data;
} atom_t, *patom_t;

/* list structure: */
typedef struct {
	int count;
	patom_t first;
	patom_t curr;
	patom_t last;
	int (*compare)(void *, void *);
	#ifdef __REENTRANT__
	pthread_mutex_t  *mp;
	#endif
} list_t, *plist_t;

/* include prototypes but avoid redeclaration of plist_t: */
#define _list_c_ 
#include "list.h"
#undef  _list_c_


/*************************************************************************************************
 * 
 *	Some special undocumented stuff for "friend" modules :
 *  ATTENTION: DO NOT REMOVE THE SAVED ATOM BEFORE RESTORING IT !!!!
 *  If possible the save/restore should be done in a single atomic function or under a lock
 *  to prevent alteration of the list between save and restore calls !!!!
 *
 *  ------------------------> BE VERY CAREFULL!
 */
/* Save current position: */
void *
list_current_save(plist_t list) {
	if (!list) return NULL;
	return (void *) list->curr;
}
/* Restore current position: */
void *
list_current_restore(plist_t list, void *p) {
	if (!list) return NULL;
	list->curr = (patom_t) p;
	return p;
}
/*************************************************************************************************/

size_t list_sizeof() { return sizeof(list_t); }

/* constructor: */
plist_t
list_new() {
	plist_t pl;

	/* try allocate memory for list: */
	if (!(pl = (plist_t) mem_zmalloc(sizeof(list_t)))) return NULL;

	/* initialize; */
	pl->first = pl->curr = pl->last = NULL;
	pl->count = 0;

	#ifdef __REENTRANT__
	pl->mp = (pthread_mutex_t *) mem_zmalloc(sizeof(pthread_mutex_t));
	#endif

	/* return pointer to list */
	return pl;
}

/* destructor: */
plist_t
list_destroy(plist_t pl) {
	patom_t pa, pf;

	/* sanity check */
	if (!pl) return NULL;

	/* prevent clients to access list: */
	_list_lock_(pl);

	/* loop on atoms: */
	for (pa = pl->first; pa;) {
		/* save pa */
		pf = pa;
		
		/* next: */
		pa = pa->next;
		
		/* clean memory: */
		pf->prev = pf->data = pf->next = NULL;

		/* free memory */
		free(pf);
		pf = 0;
	}

	/* restore access */
	_list_unlock_(pl);
	#ifdef __REENTRANT__
	pthread_mutex_destroy(pl->mp);
	mem_clean(pl->mp);
	#endif
	mem_clean(pl);
	return NULL;
}

/* private atom deletion function (non locking) for interal use in higher level functions: */
static void *
_list_del_(plist_t pl) {
	void *	data;
	patom_t pa = NULL;

	/* sanity check: */
	if (!pl || !pl->curr) return NULL;

	pl->count--;
	pa   = pl->curr;
	data = pa->data;
	
	if (pa->prev) pa->prev->next = pa->next;
	else pl->first = pa->next;

	if (pa->next) pa->next->prev = pa->prev;
	else pl->last = pa->prev;

	if (!pa->next && pa->prev) pl->curr = pa->prev;
	else pl->curr = pa->next;

	pa->data = pa->prev = pa->next = NULL;

	free(pa); pa = NULL;
	return data;
}

/* private atom push function (non locking) for interal use in higher level functions: */
static void *
_list_push_(plist_t pl, void *data) {
	patom_t pa;

	/* sanity check */
	if (!pl) return NULL;

	if (!(pa = (patom_t) mem_zmalloc(sizeof(atom_t)))) return NULL;

	/* set atom data: */
	pa->data = data;
	pa->next = NULL;
	pa->prev = pl->last;

	/* link new atom to last list atom: */
	if (pl->last) pl->last->next = pa;

	/* fix list: */
	pl->last = pa;
	pl->curr = pa;
	pl->count++;

	/* check if the new atom is the 1st element: */
	if (pl->first == NULL) pl->first = pa;

	return data;
}


/* set a comparison function: */
int
list_compare_set(plist_t pl, list_cmp_f compare) {
	/* sanity checks: */
	if (!pl)      return -1;
	if (!compare) return -2;
	
	_list_lock_(pl);
	pl->compare = compare;
	_list_unlock_(pl);

	return 0;
}

/*
 *	Navigation:
 */

void *
list_first(plist_t pl) {
	/* sanity check */
	if (!pl || !pl->curr) return NULL;
	_list_lock_(pl);
	pl->curr = pl->first;
	_list_unlock_(pl);
	return pl->first->data;
}

void *
list_prev(plist_t pl) {
	/* sanity check */
	if (!pl || !pl->curr || !pl->curr->prev) return NULL;
	_list_lock_(pl);
	pl->curr = pl->curr->prev;
	_list_unlock_(pl);
	return pl->curr->data;
}

void *
list_next(plist_t pl) {
	/* sanity check */
	if (!pl || !pl->curr ||! pl->curr->next) return NULL;
	_list_lock_(pl);
	pl->curr = pl->curr->next;
	_list_unlock_(pl);
	return pl->curr->data;
}

void *
list_last(plist_t pl) {
	/* sanity check */
	if (!pl || !pl->curr) return NULL;
	_list_lock_(pl);
	pl->curr = pl->last;
	_list_unlock_(pl);
	return pl->curr->data;
}

/* slow moves: */
void *
list_moveto_i(plist_t pl, int index) {
	patom_t pa;
	int i;

	/* sanity check: */
	if (!pl) return NULL;

	_list_lock_(pl);
	
	/* sanity check / corrections: */
	if (index > pl->count) index = pl->count - 1;
	if (index < 0) index = 0;

	/* loop until index or end of list: */
	for (i = 0, pa = pl->first; pa && i < index; i++, pa = pa->next);

	if (!pa || i < index) {
		/* a problem occured: */
		_list_unlock_(pl);
		return NULL;
	}

	/* set current to index-ieth atom: */
	pl->curr = pa;

	_list_unlock_(pl);
	return pa->data;
}

void *
list_moveto_p(plist_t pl, void *data) {
	patom_t pa;

	/* sanity check: */
	if (!pl) return NULL;

	_list_lock_(pl);	

	/* loop until data found or end of list: */
	for (pa = pl->first; pa && pa->data != data; pa = pa->next);
	
	if (!pa || pa->data != data) { 
		/* data not found: */
		_list_unlock_(pl);
		return NULL;
	}
	
	/* set current atom: */
	pl->curr = pa;

	_list_unlock_(pl);

	return pa->data;
}

/*
 *	get count, current data, current index:
 */

int 
list_count(plist_t pl) {
	/* sanity check: */
	if (pl == NULL) return -1;

	return pl->count;
}

void *
list_current(plist_t pl) {
	/* sanity check: */
	if (!pl || !pl->curr) return NULL;

	return pl->curr->data;
}

int
list_current_index(plist_t pl) {
	int i = 0;
	patom_t pa;
	
	/* sanity check */
	if (!pl || !pl->first) return -1;

	_list_lock_(pl);
	for (pa = pl->first; pa && pa != pl->curr; pa = pa->next, i++); 
	_list_unlock_(pl);

	return i;
}

/*
 *	stack / queue simulators:
 */

void *
list_push(plist_t pl, void *data) {

	/* sanity check */
	if (!pl) return NULL;

	_list_lock_(pl);

	/* call lower level private non blocking push function: */
	_list_push_(pl, data);

	_list_unlock_(pl);
	
	return data;
}

/* queue pop simulation (FIFO model): */
void *
list_pop_first(plist_t pl) {
	void *data;

	/* sanity check */
	if (!pl) return NULL;

	/* check if data availible: */
	if (!pl->first) return NULL;

	_list_lock_(pl);

	/* set current to first: */
	pl->curr = pl->first;
	
	/* call non blocking deletion primitive: */
	data = _list_del_(pl);

	_list_unlock_(pl);
	return data;
}	

/* stack pop simulation (LIFO model) */
void *
list_pop_last(plist_t pl) {
	void *data;

	/* sanity check: */
	if (!pl || !pl->curr) return NULL;

	_list_lock_(pl);

	/* set current to last: */
	pl->curr = pl->last;

	/* call non blocking deletion primitive: */
	data = _list_del_(pl);

	_list_unlock_(pl);
	return data;
}

/* add / insert / del / change */
void *
list_add(plist_t pl, void *data) {
	patom_t pa;

	/* sanity check: */
	if (!pl) return NULL;

	/* check if we cannot simply call list_push: */
	if (pl->curr == pl->last) return list_push(pl, data);

	if (!(pa = (patom_t) mem_zmalloc(sizeof(atom_t)))) return NULL;

	_list_lock_(pl);

	/* set atom data */
	pa->data = data;
	pa->prev = pl->curr;
	pa->next = pl->curr->next;

	pl->curr->next->prev = pa;
	pl->curr->next = pa;

	pl->curr = pa;
	pl->count++;

	_list_unlock_(pl);
	return data;
}
	
void *
list_insert(plist_t pl, void *data) {
	patom_t pa = NULL;
	
	/* sanity check: */
	if (pl == NULL) return NULL;

	/* check if we cannot simply call list_add: */
	if (pl->curr != pl->first) { 
		list_prev(pl); 
		return list_add(pl, data); 
	}
	if (!(pa = (patom_t) mem_zmalloc(sizeof(atom_t)))) return NULL;

	_list_lock_(pl);

	/* set atom data: */
	pa->data = data;
	pa->prev = NULL;
	pa->next = pl->first;
	
		/* update list data: */
	if (pl->first) pl->first->prev = pa; 
	pl->first = pa;
	pl->curr  = pl->first;
	pl->count++;

	_list_unlock_(pl);

	return data;	
}

void *
list_del(plist_t pl) {
	void *data;

	/* sanity check: */
	if (!pl || !pl->curr) return NULL;

	_list_lock_(pl);

	/* call internal non blocking deletion primitive: */
	data = _list_del_(pl);

	_list_unlock_(pl);
	return data;
}

void *
list_change(plist_t pl, void *pnew) {
	void *pold;
	
	/* sanity check: */
	if (!pl || !pl->curr) return NULL;

	_list_lock_(pl);

	pold = pl->curr->data;
	pl->curr->data = pnew;

	_list_unlock_(pl);

	return pold;
}

/* 
 * 	Loop function:
 */

void 
list_foreach(plist_t pl, list_cbk_f callback) {
	patom_t pa;
	/* sanity check: */
	if (!pl || !pl->curr || !callback) return;
	_list_lock_(pl);
	for (pa = pl->first; pa; pa = pa->next) callback(pa->data);
	_list_unlock_(pl);
}
	

/* 
 *	comparision and sorting: 
 */

void *
list_min(plist_t pl) {
	void *pmin;
	patom_t pa;

	/* sanity checks: */
	if (!pl || !pl->curr || !pl->compare) return NULL;

	_list_lock_(pl);

	/* 
	 * initialize pmin to 1st data, 
     * loop from second atom to end of list to check if "smaller" data found: 
	 */
	for (pmin = pl->first->data, pa = pl->first->next; pa; pa = pa->next) 
		if (pl->compare(pmin, pa->data) > 0) 
			/* keep this data: */
			pmin = pa->data;

	_list_unlock_(pl); 

	return pmin;
}	

void *
list_max(plist_t pl) {
	void *pmax;
	patom_t pa;

	/* sanity checks: */
	if (!pl || !pl->curr || !pl->compare) return NULL;

	_list_lock_(pl);

	/* 
	 * initialize pmax to 1st data, 
     * loop from second atom to end of list to check if "bigger" data found: 
	 */
	for (pmax = pl->first->data, pa = pl->first->next; pa; pa = pa->next) 
		if (pl->compare(pa->data, pmax) > 0) 
			/* keep this data: */
			pmax = pa->data;

	_list_unlock_(pl); 

	return pmax;
}	

int
list_sort(plist_t pl) {
	void **array, *curdata;
	patom_t pa;
	int i, count;
	
	/* sanity checks: */
	if (!pl || !pl->curr || !pl->compare) return 1;

	/* try to be lazy: */
	if (pl->count < 2) return 0;

	if (!(array = (void **) mem_zmalloc(pl->count * sizeof(void *)))) return 2;

	_list_lock_(pl);

	/* save pointer to current atom data: */
	curdata = pl->curr->data;

	/* starting at 1st atom, delete all atoms while storing data in an array: */
	for (pa = pl->first, i = 0; pl->first; pa = pl->first, i++) 
		/* store data of delete atom (non locking deletion) */
		array[i] = _list_del_(pl);

	count = i;

	/* sort the array: */
	qsort((void *) array, count, sizeof(void *), (int(*)(const void *, const void *)) pl->compare);

	/* reinsert data in new atoms using array order: */
	for (i = 0; i < count; i++) {
		_list_push_(pl, array[i]);
		if (array[i] == curdata) pa = pl->curr;
	}

	/* clean the array */	
	mem_clean(array);

	/* set back original current atom! */
	pl->curr = pa;

	_list_unlock_(pl);
	return 0;
}

#include <stdio.h>

/* Debug functions: */

void
list_dump(plist_t pl) {
#if defined(__DEBUG__)
	int i = 0;
	patom_t pa;

	/* sanity check */
	if (!pl) return;

	_list_lock_(pl);

	printf("list 0x%lx:\n", (unsigned long) pl);
	printf("count = %d\tfirst = %p\tcurr = %p\tlast = %p\n", pl->count, pl->first, pl->curr, pl->last);
	for (pa = pl->first; pa; pa = pa->next) 
		printf("%04d: prev = %05p\tptr = %05p\tnext = %05p\n", i++, pa->prev, pa, pa->next);
	_list_unlock_(pl);
#endif
}

void
list_dump_callback(plist_t pl, list_cbk_f callback) {
#if defined(__DEBUG__)
	int i = 0;
	patom_t pa;

	/* sanity check */
	if (!pl) return;

	_list_lock_(pl);

	printf("list 0x%lx:\n", (unsigned long) pl);
	printf("count = %d\tfirst = %p\tcurr = %p\tlast = %p\n", pl->count, pl->first, pl->curr, pl->last);
	for (pa = pl->first; pa; pa = pa->next) {
		printf("%04d: prev = %05p\tptr = %05p\tnext = %05p\t", i++, pa->prev, pa, pa->next);
		if (callback) callback(pa->data);
		printf("\n");
	}
	_list_unlock_(pl);
#endif
}

#if defined(_test_list_)
int comp(int *a, int *b) {
	int v1, v2;
	v1 = *a;
	v2 = *b;
	if (v1 < v2) return -1;
	if (v1 == v2) return 0;
	return 1;
}

void
dump2(plist_t l) {
	patom_t pa;
	
	_list_lock_(l);

	for (pa = l->last; pa; pa = pa->prev) printf("\t%d", *(int *) pa->data); 
	printf("\n");

	_list_unlock_(l);
	list_dump(l);
}

void
dump(plist_t l) {
	patom_t pa;

	_list_lock_(l);

	for (pa = l->first; pa; pa = pa->next) printf("\t%d", *(int *) pa->data); 

	printf("\n");
	_list_unlock_(l);
	dump2(l);
}

#define RGBFILE "rgb.txt"
#define NAMESIZE 64
#include "storage.h"

typedef struct {
    char name[NAMESIZE];
    int r, g, b;
} rgb_t, *prgb_t;

int rgb_cmp(void *a, void *b) {
	prgb_t aa, bb;
    if (a && b) {
		aa = (prgb_t) *(void **)a;
		bb = (prgb_t) *(void **)b;
        return strcmp(aa->name, bb->name);
	}
    return 0;
}    

void rgb_print(void *rgb) { 
    if (rgb)
        printf("%-20s (%3d, %3d, %3d)", 
			((prgb_t) rgb)->name, 
			((prgb_t) rgb)->r, 
			((prgb_t) rgb)->g, 
			((prgb_t) rgb)->b);
}    
void rgb_println(void *rgb) { 
    if (rgb)
        printf("%-20s (%3d, %3d, %3d)\n", 
			((prgb_t) rgb)->name, 
			((prgb_t) rgb)->r, 
			((prgb_t) rgb)->g, 
			((prgb_t) rgb)->b);
}    

#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }

#if defined(_test_list_)
int main(void) {
#else
int list_test(void) {
#endif
	plist_t l;
	pstorage_t s;
	int j, i[10];
	FILE *f;
	void *p; 

	printf("*********************\n");
    printf("testing line module: \n");

	
	for (j = 0; j < 10; j++) i[j] = j;
	echo_cmd(l = list_new());
	printf("populating with ints:\n");
	echo_cmd(list_compare_set(l, (list_cmp_f) comp));
	echo_cmd(list_push(l, i+2));
	dump(l);
	echo_cmd(list_push(l, i+1));
	dump(l);
	echo_cmd(list_push(l, i+6));
	dump(l);
	echo_cmd(list_push(l, i+7));
	dump(l);
	echo_cmd(list_push(l, i+9));
	dump(l);
	echo_cmd(list_push(l, i+8));
	dump(l);
	printf("sorting:\n");
	echo_cmd(list_sort(l));	
	dump(l);
	printf("last:\n");
	echo_cmd(list_last(l));
	printf("append 5:\n");
	echo_cmd(list_add(l, i+5));
	dump(l);
	printf("previous:\n");
	echo_cmd(list_prev(l));
	printf("insert 4:\n");
	echo_cmd(list_insert(l, i+4));
	dump(l);
	printf("delete last:\n");
	echo_cmd(list_last(l));
	echo_cmd(list_del(l));
	dump(l);
	printf("add 3:\n");
	echo_cmd(list_add(l, i+3));
	dump(l);
	printf("pop first:\n");
	echo_cmd(list_pop_first(l));
	dump(l);
	printf("pop last:\n");
	echo_cmd(list_pop_last(l));
	dump(l);
	printf("push 8 and 1:\n");
	echo_cmd(list_push(l, i+8));
	echo_cmd(list_push(l, i+1));
	dump(l);
	printf("insert 5:\n");
	echo_cmd(list_insert(l, i+5));
	dump(l);
	printf("insert 1:\n");
	echo_cmd(list_insert(l, i+1));
	dump(l);
	printf("insert 0:\n");
	echo_cmd(list_insert(l, i));
	dump(l);
	printf("sorting:\n");
	echo_cmd(list_sort(l));	
	dump(l);
	printf("list_moveto_i:\n");
	echo_cmd(list_moveto_i(l, 5));
	dump(l);
	printf("list_current_index:\n");
	printf("current index (should be 5) = %d\n", list_current_index(l));
	printf("list_moveto_p:\n");
	echo_cmd(list_moveto_p(l, i+1));
	dump(l);
	printf("list_current:\n");
	printf("current index (should be 0x%x) = %x\n", i+1, list_current(l));
	echo_cmd(list_destroy(l));


	l = list_new();
	s = storage_new(sizeof(rgb_t), 50);

	if (!(f = fopen(RGBFILE, "r"))) {
		list_destroy(l);
		printf("cannot open \"%s\" file, exiting\n", RGBFILE);
		return 1;
	}

	while (!feof(f)) {
		rgb_t rgb;
		
		memset(rgb.name, 0, NAMESIZE);
        if (fscanf(f, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, rgb.name) == 4) {
            p = storage_add(s, (char *) &rgb);
			list_push(l, p);
		}	
	}
   	fclose(f); 
	printf("\n\nrgb initial list:\n-----------------\n");
	for (p = list_first(l); p; p = list_next(l)) rgb_println((prgb_t) p);
	list_compare_set(l, (list_cmp_f) rgb_cmp);
	printf("sorting...");	
	list_sort(l);
	printf("done\n");
	printf("\n\nrgb final list (list_foreach):\n-----------------\n");
	list_foreach(l, (list_cbk_f) rgb_println);
	printf("\n\nrgb final (list_dump_callback):\n-----------------\n");
	list_dump_callback(l, (list_cbk_f) rgb_print);

	list_destroy(l);
	storage_destroy(s);
	
	return 0;	
}
#endif
