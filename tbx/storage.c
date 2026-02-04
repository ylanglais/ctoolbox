
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    11/03/2002  1.0 make code reentrant w/ pthread_mutexes
	18/07/2016	1.1 free mutex in destroy.
	26/10/2017	1.2 add storage_set
*/   

#include <stdlib.h>
#include <strings.h>
#include <string.h>

#ifdef __reentrant__
#include <pthread.h>
#define storage_lock(x)   pthread_mutex_lock((x)->mp)
#define storage_unlock(x) pthread_mutex_unlock((x)->mp)
#else
#define storage_lock(x)
#define storage_unlock(x)
#endif

#include "mem.h"

char storage_MODULE[]  = "Simple storage management (dynamic arrays w/o deletion)";
char storage_PURPOSE[] = "Simple storage management (dynamic arrays w/o deletion)";
char storage_VERSION[] = "1.2";
char storage_DATEVER[] = "27/10/2016";

typedef struct {
	size_t unit;
	size_t grain;
	int    used;
	int    allocated;
	int    pages;
	char **data;
    #ifdef __reentrant__
    pthread_mutex_t  *mp;
    #endif  
} storage_t, *pstorage_t;

#define __storage_c__
#include "storage.h"
#undef  __storage_c__

pstorage_t storage_destroy(pstorage_t ps) {
	int i;
	if (ps) { 
		storage_lock(ps);
		/* free all pages: */
		for (i = 0; i < ps->pages; i++) 
			if (ps->data[i]) mem_free(ps->data[i]);
		/* free page array: */
		mem_free(ps->data);
		storage_unlock(ps);
		#ifdef __reentrant__
		pthread_mutex_destroy(ps->mp);
		mem_free(ps->mp);
		#endif
		
		mem_free(ps);
	}
	return NULL;
}

size_t storage_sizeof() { return sizeof(storage_t); }

pstorage_t storage_new(size_t unit, size_t grain) {
	pstorage_t ps;
	if (!(ps          = (pstorage_t) mem_zmalloc(sizeof(storage_t)))) return NULL;
	if (!(ps->data    = (char **)    mem_zmalloc(sizeof(char *))))    return storage_destroy(ps);
	if (!(ps->data[0] = (char *)     mem_zmalloc(unit * grain)))      return storage_destroy(ps);
	ps->grain = grain;
	ps->unit  = unit;
	ps->used  = 0;
	ps->pages = 1;
	ps->allocated = grain;
	#ifdef __reentrant__
	ps->mp = (pthread_mutex_t *) mem_zmalloc(sizeof(pthread_mutex_t));
	#endif
	return ps;
}

int storage_morecore(pstorage_t ps) {
	char *p;

	if (!ps) return 1;
	if (ps->used < ps->allocated - 1) return 0;

	/* Need more room! 
	 * Room extension CANNOT be done by resizing since realloc may change storage location.
	 * Thus, in this case,  pointers refering to stored data within the storage area
	 * would point to anywhere but expected data!!!
	 * Instead, room extension can be done by resizing a page array and allocate a 
	 * new page of grain * unit bytes. This way, we are sure that allocated areas will
	 * not be moved.
	 */
	
	/* resize page array: */
	if (!(p = (char *) mem_realloc((char *) ps->data, (ps->pages + 1) * sizeof(char *)))) {
		storage_unlock(ps);
		return 2;
	}
	ps->pages++;
	ps->data = (char **) p;

	/* allocate a new page at the end of the page array: */
	if (!(ps->data[ps->pages - 1] = (char *) mem_zmalloc(ps->grain * ps->unit))) {
		storage_unlock(ps);
		return 3;
	}
	ps->allocated += ps->grain;
	return 0;
}

char *storage_alloc(pstorage_t ps) {
	char *p;

	if (!ps) return NULL;
	storage_lock(ps);	
	if (storage_morecore(ps)) {
		storage_unlock(ps);
		return NULL;
	}

	/* retrieve page: */
	p = ps->data[ps->used / ps->grain];

	/*retrieve offset with the page: */
	p += (ps->used % ps->grain) * ps->unit; 
	ps->used++;
	storage_unlock(ps);
	return p;
}

char *storage_add(pstorage_t ps, char *pdata) {
	char *p;
	if (!ps) return NULL;
	if (!(p = storage_alloc(ps))) return NULL;
	if (pdata != NULL) memcpy((void *) p, (void *) pdata, ps->unit);
	return p;
}

char * storage_get(pstorage_t ps, int i) {
	if (!ps) return NULL;
	if (i < 0 || i > ps->used) return NULL;
	return ps->data[i / ps->grain] + ((i % ps->grain) * ps->unit);
}

char * storage_set(pstorage_t ps, int i, char *pdata) {
	char *p;
	if (!ps) return NULL;
	if (i < 0 || i > ps->used) return NULL;
	p = ps->data[i / ps->grain] + ((i % ps->grain) * ps->unit);
	memcpy(p, pdata, ps->unit);
	return p;
}

int storage_used(pstorage_t ps) {
	if (ps) return ps->used;
	return -1;
}

int storage_allocated(pstorage_t ps) {
	if (ps) return ps->allocated;
	return -1;
}

int storage_available(pstorage_t ps) {
	if (ps) return ps->allocated - ps->used;
	return -1;
}

int storage_grain(pstorage_t ps) {
	if (ps) return ps->grain;
	return -1;
}

int storage_unit(pstorage_t ps) {
	if (ps) return ps->unit;
	return -1;
}

int storage_pages(pstorage_t ps) {
	if (ps) return ps->pages;
	return -1;
}
