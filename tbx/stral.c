
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    14/09/2017  1.0 Initial version
*/   

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#ifdef __reentrant__
#include <pthread.h>
#define stral_lock(x)   pthread_mutex_lock((x)->mp)
#define stral_unlock(x) pthread_mutex_unlock((x)->mp)
#else
#define stral_lock(x)
#define stral_unlock(x)
#endif
#include "mem.h"

char stral_MODULE[]  = "STRing ALlocator";
char stral_PURPOSE[] = "Allocate lots of strings for a single purpose (no free) & free all at once";
char stral_VERSION[] = "1.0";
char stral_DATEVER[] = "14/09/2017";

		
#ifdef _test_stral_
#define stral_PAGE 64
#else
#define stral_PAGE 4032
#endif

typedef struct {
	size_t	size;
	size_t  reserved;
} page_t, *ppage_t;

typedef struct _slot_t {
	char *p;
	size_t size;
	struct _slot_t *prev;
	struct _slot_t *next;
} slot_t, *pslot_t;

typedef struct {
	int     pages;
	ppage_t *data;
	pslot_t free;
    #ifdef __reentrant__
    pthread_mutex_t  *mp;
    #endif  
} stral_t, *pstral_t;

#define _stral_c_
#include "stral.h"
#undef  _stral_c_

static ppage_t
_stral_page_new(size_t size) {
	ppage_t p;
	if (!(p = (ppage_t) mem_zmalloc(sizeof(page_t) + size))) return NULL;
	p->size = size + sizeof(page_t);
	return p;
}

static pslot_t
_stral_slot_new(pstral_t s, char *room, size_t size) {
	pslot_t sl, f;
	
	if (!(sl = (pslot_t) mem_zmalloc(sizeof(slot_t))))
		return NULL;

	sl->p    = room;
	sl->size = size;

	if (!s->free) {
		s->free = sl;
		sl->prev = sl->next = NULL;
		return sl;
	}

	for (f = s->free; f->size <= sl->size && f->next; f = f->next);

	if (f->size > sl->size) {
		/* insert before */
		sl->prev = f->prev;
		if (sl->prev) 
			/* if not first: */
			sl->prev->next = sl;
		else 
			/* first: */
			s->free = sl;
		f->prev  = sl;
		sl->next = f;
	} else {
		/* insert after: */	
		sl->next = f->next;
		if (sl->next) 
			sl->next->prev = sl;
		sl->prev = f;
		f->next = sl;
	}
	return sl;
}

static pslot_t
_stral_slot_destroy(pstral_t s, pslot_t sl) {
	if (sl) {
		if (sl->prev)
			sl->prev->next = sl->next;
		else 
			s->free = sl->next;

		if (sl->next) 
			sl->next->prev = sl->prev;
	
		free(sl);
	}
	return NULL;
}

size_t
stral_sizeof() {
	return sizeof(stral_t);
}

pstral_t 
stral_destroy(pstral_t s) {
	if (s) {
		int i;
		while (s->free) _stral_slot_destroy(s, s->free);
		if (s->data) {
			for (i = 0; i < s->pages; i++) 
				if (s->data[i]) free(s->data[i]);
			free(s->data);
 		}
#ifdef __REENTRANT__
		if (s->mp) free(s->mp);
#endif
		free(s);
	}
	return NULL;
}
pstral_t
stral_new() {
	pstral_t s;

	if (!(s          = (pstral_t) mem_zmalloc(stral_sizeof())))    return NULL;
	if (!(s->data    = (ppage_t *)  mem_zmalloc(sizeof(ppage_t)))) return stral_destroy(s);
	if (!(s->data[0] = _stral_page_new(stral_PAGE)))               return stral_destroy(s);

	s->pages = 1;
	s->free  = NULL;

	_stral_slot_new(s, ((char *) s->data[0]) + sizeof(page_t), stral_PAGE);
	
	#ifdef __REENTRANT__
	s->mp = (pthread_mutex_t *) mem_zmalloc(sizeof(pthread_mutex_t));
	#endif
	return s;
}

static pslot_t
_stral_page_add(pstral_t s, size_t size) {
	pslot_t sl;
	ppage_t *p;
	ppage_t q;

	if (!(p = (ppage_t *) realloc((char *) s->data,  (s->pages + 1) * sizeof(ppage_t *))))
		return NULL;
	if (!(q = _stral_page_new(size))) {
		p = (ppage_t *) realloc((char *) s->data,  s->pages * sizeof(ppage_t *));
		s->data = p;
		return NULL;
	}
	s->data           = p;
	s->data[s->pages] = q;
	s->pages++;
	sl = _stral_slot_new(s, ((char *) q) + sizeof(page_t), size);
	return sl;
}

char *
stral_alloc(pstral_t s, size_t size) {
	char *p = NULL ;
	if (!s) return NULL;
	/* find 1st free slot with size >= size */	
	pslot_t sl;
	size_t si;
	if (size > stral_PAGE) si = size * 3;
	else si = stral_PAGE;

	if (!s->free) if (!(sl = _stral_page_add(s, si))) return NULL;
	for (sl = s->free; sl->size < size && sl->next; sl = sl->next);
	if (sl->size < size) {
		if (!(sl = _stral_page_add(s, si))) return NULL;
	}
	if (sl->size >= size) {
		p = sl->p;
		if (sl->size > size) {
			_stral_slot_new(s, p + size, sl->size - size);
		}
		_stral_slot_destroy(s, sl);
	}
	return p;	
}

char *
stral_dup(pstral_t s, char *str) {
	char *p;
	p = stral_alloc(s, strlen(str) + 1);
	strcpy(p, str);
	return p;
} 

#ifdef _test_stral_
#include <stdio.h>
#include "fmt.h"

void
stral_slot_print(pslot_t sl) {
	printf("slot %p: prev = %p; next = %p; ptr = %p; size = %d\n", sl, sl->prev, sl->next, sl->p, sl->size); fflush(stdout);
}

void
stral_page_print(int i, ppage_t p) {
	ppage_t pa;
	pa = p;
	printf("page % 3d: size: %d\n", i, pa->size - sizeof(page_t)); fflush(stdout);
	fmt_dump_bin_data((char *) p, pa->size);
}

void
stral_freelist_print(pstral_t s) {
	pslot_t sl;
	printf("freelist:\n"); fflush(stdout);
	for (sl = s->free; sl; sl = sl->next) stral_slot_print(sl);
}

void
stral_dump(pstral_t s) {
	int i;
	stral_freelist_print(s);
	for (i = 0; i < s->pages; i++) {
		stral_page_print(i, s->data[i]);
	}
}

int main(void) {
	char *test1[] = {
		"0azerty",
		"1azerty",
		"2azerty",
		"3azerty",
		"4azerty",
		"5azerty",
		"6azerty",
		"7azerty",
		"8azerty",
		"9azerty",
		"10azert",
		"11azert",
		"12azert",
		"13azert",
		"14azert",
		"15azert",
		NULL
	};
	char *test2[] = {
		"0azerty",
		"1azerty",
		"***012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789***",
		"2azerty",
		"3azerty",
		"4azerty",
		"5azerty",
		"6azerty",
		"7azerty",
		"8azerty",
		"9azerty",
		"10azert",
		"11azert",
		"12azert",
		"13azert",
		"14azert",
		"15azert",
		"+++012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789+++",
		"16azert",
		"17azert",
		NULL
	};
	pstral_t s;
	char *p;
	int i;
	s = stral_new();
	for (i = 0; test1[i]; i++) {
		p = stral_alloc(s, strlen(test1[i]) + 1);
		strcpy(p, test1[i]);
	//	stral_freelist_print(s);	
	} 
//	stral_dump(s);
	stral_destroy(s);
	s = stral_new();
	//stral_dump(s);
	for (i = 0; test2[i]; i++) {
		p = stral_dup(s, test2[i]);
	//	stral_dump(s);
	//	stral_freelist_print(s);	
	}
	//stral_dump(s);
	stral_destroy(s);
}

#endif
