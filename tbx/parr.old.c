
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    08/12/2017  0.9 	Initial version
*/   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pal.h"
#include "err.h"
#include "futl.h"

char parr_MODULE[]  = "Persystant DYNAmic allocator ";
char parr_PURPOSE[] = "Allocate persistant arrays of standarized data";
char parr_VERSION[] = "1.0";
char parr_DATEVER[] = "01/12/2017";

typedef char parr_filename_t[256];

/* Define free slots relatively */
typedef struct {
	int         page;
	int         offset;
} slot_t, *pslot_t;

typedef struct _page_t {
	parr_filename_t pname;
	int	            num;
	size_t          unit;
	size_t          grain;
	size_t          used;
	struct _page_t *prev;	
	struct _page_t *next;	 
	char *	        data;
	slot_t	        freeslots[];
} page_t, *ppage_t;

typedef struct {
	int 		lock;
	char 		path[256];
	char 		filespec[256];
	size_t 		unit;
	size_t 		grain;
	int	   		used;
	int    		npages;
	slot_t 	    freeslot; /* linked list of free slot pointing to 1st page w/ free slots */
	ppage_t	    pages;    /* linked list of npages */
	pslot_t     idx;      /* => in a separate file !!! Absolute order index of used slots: */
} parr_t, *pparr_t;

#define _parr_c_
#include "parr.h"
#undef  _parr_c_

typedef void (*dump_data_f)(char *data);

static char *
_parr_file_main(char *path, char *filespec, parr_filename_t fname) {
	sprintf(fname, "%s/%s.parr", path, filespec);
	return (char *) fname;
}

static char *
_parr_file_idx(char *path, char *filespec, parr_filename_t iname) {
	sprintf(iname, "%s/%s.idx", path, filespec);
	return (char *) iname;
}

static char *
_parr_file_page(char *path, char *filespec, int page, parr_filename_t fname) {
	sprintf(fname, "%s/%s.%d", path, filespec, page);
	return (char *) fname;
}

static size_t
_parr_page_size(pparr_t arr) {
	if (!arr) return 0;
	return arr->grain * (arr->unit + sizeof(slot_t)) + sizeof(page_t);
}

static ppage_t
_parr_page_get(pparr_t arr, int pnum) {
	ppage_t page;
	int i;
	if (!arr) return NULL;
	if (pnum > arr->npages) pnum = arr->npages - 1;
	for (page = arr->pages, i = 0; page && page->next && i < pnum; page = page->next, i++);
	if (page && i == pnum) return page;
	return NULL;
}

static pslot_t
_parr_page_slot(ppage_t page, int offset) {
	if (!page) return NULL;
	if (offset > page->grain) return NULL;
	return &(page->freeslots[offset]);
}

static pslot_t 
_parr_slot_next(pparr_t arr, pslot_t slot) {
	ppage_t p;
	if (!arr)  return NULL;
	if (!slot) return NULL;
	if (slot->page == -1) return NULL;
	p = _parr_page_get(arr, slot->page);
	return  _parr_page_slot(p, slot->offset);
}

static void
_parr_page_dump(ppage_t p, dump_data_f dumpdata) {
	if (!p) {
		printf("null page\n");
		return;
	}
	printf("  page:  %x\n",  p);
	printf("  pname: %s\n",  p->pname);
	printf("  num:   %d\n",  p->num);
	printf("  unit:  %lu\n", p->unit);
	printf("  grain: %lu\n", p->grain);
	printf("  used:  %d\n",  p->used);
	printf("  prev:  %x\n",  p->prev);
	printf("  next:  %x\n",  p->next);
	printf("  data:	 %x\n",  p->data);
	printf("  slots:\n");
	int i;
	for (i = 0; i < p->grain; i++) {
		printf("  --> % 2d: page %d, offset %d ", i, p->freeslots[i].page, p->freeslots[i].offset);
		if (dumpdata) {
			printf(" --> "); dumpdata(p->data + i * p->unit);
	 	}
		printf("\n");
	}
}

static void 
_parr_dump(pparr_t arr, dump_data_f dumpdata) {
	pslot_t s;
	int i = 0;
	
	if (!arr) {
		printf("null persistant array\n");
		return;
	}
	printf("lock:     %s\n",  arr->lock ? "Locked" : "Unlocked");
	printf("path:     %s\n",  arr->path);
	printf("filespec: %s\n",  arr->filespec);
	printf("unit:     %lu\n", arr->unit);
	printf("grain:    %lu\n", arr->grain);
	printf("used:     %d\n",  arr->used);
	printf("npages:   %d\n",  arr->npages);
	printf("freeslot: \n");
	for (s = &arr->freeslot; s && s->page != -1; s = _parr_slot_next(arr, s), i++) {
		printf("--> % 2d: page %d, offset %d\n", i, s->page, s->offset);
	}
	printf("data:\n");
	for (i = 0; i < arr->used; i++) {
		s = arr->idx + i;
		if (!s) printf("--> % 2d: null slot\n");
		else {
			printf("--> % 2d: page %d, offset %d", i, s->page, s->offset);
			if (dumpdata) {
				printf(" --> "); 
				dumpdata(parr_get(arr, i));
			}
			printf("\n");
		}
	}
	ppage_t p;
	for (p = arr->pages; p ; p = p->next) {
		_parr_page_dump(p, dumpdata);
	}
}

#if 0
static ppage_t
_parr_page_last(pparr_t arr) {
	if (!arr) return NULL;
	return _parr_page_get(arr, arr->npages - 1);
}
	
static ppage_t 
_parr_page_from_slot(pparr_t arr, pslot_t slot) {
	if (!arr) return NULL;
	return _parr_page_get(arr, slot->page);
}

#endif

static ppage_t
_parr_page_new(pparr_t arr) {
	ppage_t page;
	parr_filename_t pname;

	if (!arr) return NULL;
	
	_parr_file_page(arr->path, arr->filespec, arr->npages, pname);
	if (!(page = pmalloc(pname, _parr_page_size(arr)))) {
		return NULL;
	}

	page->next = page->prev = NULL;

	return page;
}

static ppage_t
_parr_page_destroy(ppage_t page) {
	parr_filename_t pname;
	if (page) {
		strcpy(page->pname, pname);
		pfree(page);
		futl_rm(pname);
	}
	return NULL;
}

static ppage_t
_parr_page_add(pparr_t arr) {
	pslot_t idx;
	if (!arr) return NULL;
	ppage_t page, p;
	if (!(page = _parr_page_new(arr))) {
		return NULL;
	}
	/* resize index */
	if (!arr->idx) {
		parr_filename_t iname;
		_parr_file_idx(arr->path, arr->filespec, iname);
		if (!(idx = pmalloc(iname, arr->grain * sizeof(slot_t)))) {
			memset(idx, 0, arr->grain * sizeof(slot_t));
			return NULL;
		}
	} else if (!(idx = (pslot_t) prealloc(arr->idx, (arr->npages + 1) * arr->grain * sizeof(slot_t)))) {
		memset((idx + (arr->npages - 1)), 0, arr->grain * sizeof(slot_t));
		return NULL;
	}

	arr->idx = idx;

	/* Initialize page header: */
	_parr_file_page(arr->path, arr->filespec, arr->npages, page->pname);
	page->unit  = arr->unit;
	page->grain = arr->grain;
	page->used  = 0;
	page->num   = arr->npages;
	page->next  = NULL;
	if (arr->pages == NULL) {
		arr->pages = page;
	} else {
		for (p = arr->pages; p->next; p = p->next);
		page->prev  = p;	
		p->next     = page;
	}
	page->data 	= ((char *) page) + page->grain * sizeof(slot_t);

	/* initialize page freeslots: */
	int i;
	for (i = 0; i < arr->grain - 1; i++) {
		page->freeslots[i].page   = page->num;
		page->freeslots[i].offset = i + 1;
	}
	page->freeslots[i].page     = -1;
	page->freeslots[i].offset   = -1;

	/* set 1st free slot: */
	arr->freeslot.page   = page->num;
	arr->freeslot.offset = 0;
	
	arr->npages++;


	return page;
}

static int 
_parr_page_remove(pparr_t arr, int page) {
	if (!arr) return 1;
	if (page < 0 || page > arr->npages) return 2;	

	/* Look at all free slots, check if more than one other than current pasge is present */ 
	pslot_t s;
	ppage_t p;
	for (s = &arr->freeslot; s && s->page > -1 && s->page < page; s = _parr_slot_next(arr, s));
	if (s->page == page) {
		pslot_t s2;
		for (s2 = s; s2 && s->page > -1 && s->page == page; s2 = _parr_slot_next(arr, s2));
		s->page   = s2->page;	
		s->offset = s2->offset;	
	} else {
		s->page   = -1;
		s->offset = -1;
	}

	/* Now decreese npages num in free slot ll where npages > num */
	for (s = &arr->freeslot; s && s->page > -1 && s->page < page; s = _parr_slot_next(arr, s));
	slot_t s2;
	s2.page = s->page; s2.offset = s2.offset;
	for (; s && s->page > -1; s = _parr_slot_next(arr, &s2)) {
		s2.page   = s->page;
		s2.offset = s->offset;
		s->page--;
	}

	/* Clean up page references in page linked list */
	for (p = arr->pages; p && p->num < page && p->next; p = p->next);
	if (p->prev) p->prev->next = p->next; else arr->pages = p->next;
	if (p->next) p->next->prev = p->prev; 
	for (p = p->next; p && p->next; p = p->next) {
		p->num--;
	}

	/* last, but not least, decrease arr page count: */
	arr->npages--;

	/* now destroy page: */
	_parr_page_destroy(p);	
	
	return 0;
}

pparr_t
parr_new(char *path, char *filespec, size_t unit, size_t grain) {
	pparr_t arr;
	ppage_t  p = NULL;
	parr_filename_t fname;
	_parr_file_main(path, filespec, fname);
	if (futl_is_dir(path) && futl_is_file(fname)) {
		if ((arr = (pparr_t)  pmalloc(fname, sizeof(parr_t)))) {
			memset(arr, 0, sizeof(parr_t));
			arr->freeslot.page   = -1;
			arr->freeslot.offset = -1;
	
			int i; 
			parr_filename_t pname;
			for (i = 0; i < arr->npages; i++) {
				ppage_t page;
				_parr_file_page(arr->path, arr->filespec, i, pname);	
				if (!(page = (ppage_t) pmalloc(pname, 0))) {
					/** Error handling **/	
					err_error("That's a mess!!!");
					return parr_destroy(arr);
				} else {
					/* Attach arr to 1st page: */
					if (i == 0) arr->pages = page;
					/* Attach new page to previous */
					else p->next = page; 
					p = page;
				}
			}	
			_parr_file_idx(arr->path, arr->filespec, pname);
			if (!(arr->idx = pmalloc(pname, 0))) {
				/** Error handling **/	
				err_error("That's a mess!!!");
				return parr_destroy(arr);
			}
		} else {
			/** Error handling **/	
			err_error("That's a mess!!!");
			return parr_destroy(arr);
		}
	} else {
		if (!futl_is_dir(path)) futl_mkdir(path);
		arr = pmalloc(fname, sizeof(parr_t));
		arr->unit  = unit;
		arr->grain = grain;
		arr->used  = 0;
		strcpy(arr->path, 	 path);
		strcpy(arr->filespec, filespec);
		ppage_t page;

		if (!(page = _parr_page_add(arr))) {	
			/** Error handling **/	
			err_error("That's a mess!!!");
			return parr_destroy(arr);
		} else  {
			arr->npages = 1;
			arr->pages  = page;
		}
	}
	return arr;
}

pparr_t  
parr_destroy(pparr_t arr) {
	if (arr) {
		while (arr->npages > 0 && arr->pages) {
			ppage_t page;
			page = arr->pages;
			arr->pages = page->next;
			if (page->next) page->next->prev = NULL;
			page->next = NULL;
			_parr_page_destroy(page);
			arr->npages--;
		}
		parr_filename_t iname;
		_parr_file_idx(arr->path, arr->filespec, iname);
		if (arr->idx) arr->idx = pfree(iname);
		if (arr->npages == 0 && arr->pages) {
			err_warning("apparently parr has npages left");
		} else if (arr->npages > 0 && !arr->pages) {
			err_warning("apparently parr has no mor npages left whereas page count is %d", arr->npages);
		}
	}
	return NULL;
}


/* Allocate a new entry: */
char *
parr_alloc(pparr_t arr) {
	if (!arr) return NULL;

	if (arr->freeslot.page == -1) {
		if (!(_parr_page_add(arr))) {
			return NULL;
		}
	} 
	ppage_t page = NULL;
	for (page = arr->pages; page->num < arr->freeslot.page && page->next; page = page->next);
	if (!page || page->num < arr->freeslot.page) {
		err_error("A big mess occured, blame the programmer");
		return NULL;
	}
	pslot_t slot;
	char *data = NULL;

	/* save slot data: */
	slot = &page->freeslots[arr->freeslot.offset];
	
	/* get data pointer: */
	data = page->data + page->unit * arr->freeslot.offset;
	page->used++;

	/* update index & count: */
	arr->idx[arr->used].page   = arr->freeslot.page;
	arr->idx[arr->used].offset = arr->freeslot.offset;
	arr->used++;
	
	/* Mage arr->freeslot point to next free slot: */
	arr->freeslot.page   = slot->page;
	arr->freeslot.offset = slot->offset;
	
	/* Mark slot as used: */
	slot->page   = -1;
	slot->offset = -1;	

	return data;
}

/* Add a new enbry w/ data */
char *
parr_add(pparr_t arr, char *data) {
	/* check if room alvaml*/
	char *ldat = NULL;
	if (!(ldat = parr_alloc(arr))) 
		return NULL;
	memcpy(ldat, data, arr->unit);	
	return ldat;
}

/* Insert a new entry w/ data */
char *
parr_insert(pparr_t arr, int index, char *data) {
	if (!arr) return NULL;
	if (index > arr->used) return parr_add(arr, data);
	parr_add(arr, data);		
	pslot_t s;
	s = arr->idx + index * sizeof(slot_t);
	memmove(s, s + 1, arr->used - 1 - index);
	return data;
}

int      
parr_delete(pparr_t arr, int index) {
	pslot_t s, f, i;
	ppage_t page, p;
	if (!arr) return 1;
	if (index < 0 || index >= arr->used) {
		err_error("Index %d is out of bounds [0:%d]", index, arr->used - 1);
		return 2;
	}
	s = arr->idx + index;
	page = _parr_page_get(arr, s->page);

	/* Clean slot */
	memset(page->data + page->unit * s->offset, 0, page->unit);

	/* Add slot to freeslot list */

	/* Search for the last free slot BEFORE new free slot 
		=> free slot page num    <= slot page 
		=> free slot page offset <  slot offset
  	 */
	
	f = &arr->freeslot;

	while ((f->page > -1 && f->page < page->num) || (f->page == page->num && f->offset < s->offset)) {
		for (p = arr->pages; p->num < f->page; p = p->next);
		f = p->freeslots + f->offset;
	}

	page->freeslots[s->offset].page    = f->page;
	page->freeslots[s->offset].offset  = f->offset;
	f->page    = s->page;
	f->offset  = s->offset;

	i = arr->idx + index + 1;

	/* squeeze index: */
	memmove((char *) (i - 1), (char *) i, (arr->used - index - 1) * sizeof(slot_t));
	memset((char *) (&arr->idx[arr->used]), 0, sizeof(slot_t));
	arr->used--;

	if (page->used == 0) 
	_parr_page_remove(arr, page->num);

	return 0;
}

char    *parr_push(pparr_t arr, char *data) {
	return parr_add(arr, data);
}

char *
parr_pop_first(pparr_t arr) { 
	char *data;
	data = parr_get(arr, 0);
	parr_delete(arr, 0);
	return data;
}
char *
parr_pop_last(pparr_t arr) {
	char *data;
	data = parr_get(arr, arr->used - 1);
	parr_delete(arr, arr->used - 1);
	return data;
}

char *
parr_get(pparr_t arr, int i) {
	pslot_t s;
	ppage_t p;
	if (!arr) return NULL;
	if (i < 0 || i > arr->used) return NULL;
	s = arr->idx + i;
	p = _parr_page_get(arr, s->page);
	return p->data + s->offset * p->unit; 
}

char *
parr_set(pparr_t arr, int i, char *data) {
		pslot_t s;
	ppage_t p;
	if (!arr) return NULL;
	if (i < 0 || i > arr->used) return NULL;
	s = arr->idx + i;
	p = _parr_page_get(arr, s->page);

	memcpy(p->data + s->offset * p->unit, data, p->unit);
	return p->data + s->offset;
}

int 
parr_count(pparr_t arr) {
	if (!arr) return 0;
	return arr->used;
}

#ifdef _test_parr_

void dump_int(char *data) {
	int i;
	i = *(int *) data;
	printf("%d", i);
}

#define printadd(arr, action, val)   {printf("%s %d\n", #action, val); action(arr, (char *) &val); } 
#define printdel(arr, action, index) {printf("%s %d\n", #action, index); action(arr, index); }
#define printpop(arr, action) {char *a = action(arr); printf("%s %d\n", #action, *(int *)a); if (a) free(a); }
#define printins(arr, action, index, val) {printf("%s %d at %d\n", #action, val, index); action(arr, index, (char *) &val); } 
	
int
main() {
	int i;
	pparr_t arr;
	
	if (!(arr = parr_new(".partest", "partest", sizeof(int), 3))) return 1;
	
	i = 1; printadd(arr, parr_add, i);
	_parr_dump(arr, dump_int);
	i = 2; printadd(arr, parr_add, i);
	_parr_dump(arr, dump_int);
	i = 3; printadd(arr, parr_add, i);
	_parr_dump(arr, dump_int);
	i = 4; printadd(arr, parr_add, i);
	_parr_dump(arr, dump_int);
	i = 5; printadd(arr, parr_add, i);
	_parr_dump(arr, dump_int);
	i = 6; printadd(arr, parr_add, i);
	_parr_dump(arr, dump_int);
	i = 7; printadd(arr, parr_add, i);
	_parr_dump(arr, dump_int);
	i = 8; printadd(arr, parr_add, i);

	_parr_dump(arr, dump_int);
	
	printdel(arr, parr_delete, 3);

	_parr_dump(arr, dump_int);

	printdel(arr, parr_delete, 6);

	_parr_dump(arr, dump_int);
	
	i = 9;  printadd(arr, parr_add,  i);
	_parr_dump(arr, dump_int);

	i = 10; printins(arr, parr_insert, 5, i);

	_parr_dump(arr, dump_int);

	i = 11; printins(arr, parr_insert, 1, i);
	i = 12; printins(arr, parr_insert, 1, i);
	i = 13; printins(arr, parr_insert, 1, i);
	i = 14; printins(arr, parr_insert, 1, i);
	i = 15; printins(arr, parr_insert, 1, i);

	_parr_dump(arr, dump_int);

	i = 16; printins(arr, parr_insert, 0, i);
	i = 17; printins(arr, parr_insert, 0, i);
	i = 18; printins(arr, parr_insert, 0, i);
	i = 19; printins(arr, parr_insert, 0, i);
	i = 20; printins(arr, parr_insert, 0, i);


	i = 7; printins(arr, parr_set, 15, i);
	i = 8; printins(arr, parr_set, 16, i);

	_parr_dump(arr, dump_int);
	
	printdel(arr, parr_delete, 0);
	printdel(arr, parr_delete, 0);
	printdel(arr, parr_delete, 0);
	printdel(arr, parr_delete, 0);
	printdel(arr, parr_delete, 0);

	_parr_dump(arr, dump_int);
	
	printdel(arr, parr_delete, 1);
	printdel(arr, parr_delete, 1);
	printdel(arr, parr_delete, 1);
	printdel(arr, parr_delete, 1);
	printdel(arr, parr_delete, 1);

	_parr_dump(arr, dump_int);

	i = 10; printadd(arr, parr_add, i);
	i = 11; printadd(arr, parr_add, i);
	i = 12; printadd(arr, parr_add, i);
	i = 13; printadd(arr, parr_add, i);
	i = 14; printadd(arr, parr_add, i);
	i = 15; printadd(arr, parr_add, i);
	i = 16; printadd(arr, parr_add, i);
	i = 17; printadd(arr, parr_add, i);
	i = 18; printadd(arr, parr_add, i);
	i = 19; printadd(arr, parr_add, i);
	i = 20; printadd(arr, parr_add, i);

	i = 4; printins(arr, parr_insert, 3, i);

	_parr_dump(arr, dump_int);
	
	parr_destroy(arr);

	if (!(arr = parr_new(".partest", "partest", sizeof(int), 3))) return 2;

	_parr_dump(arr, dump_int);

	i = 1; printadd(arr, parr_push, i);
	i = 2; printadd(arr, parr_push, i);
	i = 3; printadd(arr, parr_push, i);
	i = 4; printadd(arr, parr_push, i);
	i = 5; printadd(arr, parr_push, i);
	
	printpop(arr, parr_pop_first);
	printpop(arr, parr_pop_first);
	printpop(arr, parr_pop_first);
	printpop(arr, parr_pop_first);
	printpop(arr, parr_pop_first);

	i = 1; printadd(arr, parr_push, i);
	i = 2; printadd(arr, parr_push, i);
	i = 3; printadd(arr, parr_push, i);
	i = 4; printadd(arr, parr_push, i);
	i = 5; printadd(arr, parr_push, i);
	
	_parr_dump(arr, dump_int);

	printpop(arr, parr_pop_last);
	printpop(arr, parr_pop_last);
	printpop(arr, parr_pop_last);
	printpop(arr, parr_pop_last);
	printpop(arr, parr_pop_last);

	_parr_dump(arr, dump_int);

	parr_destroy(arr);


	return 0;
}

#endif

