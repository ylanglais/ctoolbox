/*    
    This code is released under GPL terms. 
	Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    05/09/2001  1.0 update code to v1.0 (fixes, use of the new memory allocator, ...)
    11/03/2002  2.0 update code to v2.0, bug fixes and make code reentrant w/ pthread_mutexes
    30/05/2018  3.0 fix lock problems 
*/   

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

#ifdef __REENTRANT__
#include <pthread.h>
#define hash_lock(x)   pthread_mutex_lock((x)->mp)
#define hash_unlock(x) pthread_mutex_unlock((x)->mp)
#else
#define hash_lock(x)
#define hash_unlock(x)
#endif

#include "mem.h"
#include "err.h"
 
char hash_MODULE[]	= "Hash table manager";
char hash_PURPOSE[]	= "Hash table manager";
char hash_VERSION[] = "3.0";
char hash_DATEVER[] = "30/05/2018";

int  hash_BLOCKSIZE = 257;

char *hash_errmsgs[] = {
	"hash ok",
	"null hash pointer",
	"null atom pointer",
	"null or invalid key",
	"cannot insert duplicate key",
	"cannot insert atom",
	"incorrect hash index"
};

typedef enum {
	hash_OK = 0,
	hash_NULL_HASH = 1,
	hash_NULL_ATOM,
	hash_NULL_KEY,
	hash_DUP_KEY,
	hash_CANNOT_INSERT_ATOM,
	hash_BAD_HASH_INDEX
} hash_errcode_t;

typedef struct _atom_t {
	struct _atom_t *next;
	struct _atom_t *prev;
	void *pdata;
	char *name;
} atom_t, *patom_t;

#ifdef __HASH_STATISTICS__
typedef struct {
	int nfound;
	int notfound;
	int nretrieval;
	int ninsert;
	int nisteps;
	int ndelete;
	int nsteps;
} stats_t, *pstats_t;
#endif

typedef int (f_hashcode_t)(void *, char *);

typedef struct _hash_t {
	#ifdef __HASH_STATISTICS__
	stats_t stats;
	#endif
	size_t  size;
	size_t  used;
	f_hashcode_t *f_hashcode;
	atom_t  **array;
	#ifdef __REENTRANT__
	pthread_mutex_t  *mp;
	#endif
} hash_t, *phash_t;

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define hash_debug_return(errcode) {err_error("%s", hash_errmsgs[ABS(errcode)]); return errcode;}

/* include public prototypes: */
#define _hash_c_
#include "hash.h"
#undef _hash_c_ 

	
/* Private prototypes: */
static void *   _hash_atom_destroy(phash_t ph, patom_t pa);
static patom_t  _hash_atom_new(phash_t ph, char *name, void *pdata);
static patom_t  _hash_retrieve(phash_t ph, char *name);
static void *   _hash_delete(phash_t ph, char *name);

/* Prime number calculator for hash size optimization */

unsigned long prime_first_after(unsigned long seed) {
	unsigned long i;
	if (seed == 0 || seed == 2) return seed;
	if (seed % 2 == 0) ++seed;
	for (;;) {
		for (i = 2; i * i < seed && seed % i; i++); 
		if (seed % i) return seed;
		seed += 2;
	}
}

/********************* Source *********************/

size_t hash_sizeof() { return sizeof(hash_t); }

/* hash table maintenance: */
phash_t hash_new_set(size_t size, f_hashcode_t f_hashcode) {
	phash_t ph = NULL;

	if (size <= 0) size = hash_BLOCKSIZE;
	/* optimize w/ a prime number: */
 	else size = prime_first_after(size);  

	if (f_hashcode == NULL) 
		f_hashcode = (f_hashcode_t *) &hash_classic_code;

	if (!(ph = (phash_t) mem_zmalloc(sizeof(hash_t)))) return NULL;
	/*memset((char *) ph, 0, sizeof(hash_t));*/

	ph->used = 0;
	ph->size = size;
	ph->f_hashcode = f_hashcode;
	if (!(ph->array = (atom_t **) mem_zmalloc(sizeof(atom_t *) * ph->size))) return hash_destroy(ph);

	#ifdef __REENTRANT__
	ph->mp = (pthread_mutex_t *) mem_zmalloc(sizeof(pthread_mutex_t));
	#endif
	return ph;
}

phash_t hash_new() {
	return hash_new_set(0, NULL);
}

phash_t hash_new_sized(size_t size) {
	return hash_new_set(size, NULL);
}

phash_t hash_new_fcode(f_hashcode_t f_hashcode) {
	return hash_new_set(0, f_hashcode);
}

int	hash_used(phash_t ph) {
	if (!ph) return -1;
	return ph->used;
}

int	hash_size(phash_t ph) {
	if (!ph) return -1;
	return ph->size;
}

char
hash_strcmp(register char *a, register char *b) {
	for (; *a == *b; a++, b++) if (!*a) return 0;
	return *a - *b;
}

phash_t hash_destroy(phash_t ph) {
	int i;
	patom_t pa, pa2;
	
	if (!ph) return NULL;

	hash_lock(ph);
	for (i = 0; i < ph->size; i++) {
		pa = ph->array[i];
		while (pa) {
			pa2 = pa;
			pa = pa->next;
			_hash_delete(ph, pa2->name);
		}
	}
	if (ph->array) mem_free(ph->array);
	ph->used = ph->size = 0;
	hash_unlock(ph);
	#ifdef __REENTRANT__
	pthread_mutex_destroy(ph->mp);
	mem_free(ph->mp);
	#endif

	mem_free(ph);
	return NULL;
}

phash_t hash_resize(phash_t ph, size_t size) {
	int i;
	phash_t pn = NULL;
	patom_t pa = NULL;
	void *pt   = NULL;
	size_t  tmp;

	if (!ph) return NULL;
	hash_lock(ph);
	if (size == 0) size = ph->used;
	else /* optimize w/ a prime number: */ 
		size = prime_first_after(size); 

	if (!(pn = hash_new_sized(size))) return ph;
	for (i = 0; i < ph->size; i++) {
		while ((pa = ph->array[i])) {
			hash_insert(pn, pa->name, pa->pdata); 
			hash_delete(ph, pa->name);
		}
	}
	pt = ph->array; 
	ph->array = pn->array;
	pn->array = pt;

	tmp = ph->size;
	ph->size = pn->size;
	pn->size = tmp;
	
	tmp = ph->used;
	ph->used = pn->used;
	pn->used = tmp;
	
	hash_destroy(pn);

	hash_unlock(ph);
	return ph; 
}

phash_t hash_set_code_function(phash_t ph, f_hashcode_t f_hashcode) {
	int i;
	phash_t pn = NULL;
	patom_t pa = NULL;
	void *pt   = NULL;
	size_t  tmp;

	if (!ph) return ph;
    if (!f_hashcode) return ph;

	if (!(pn = hash_new_sized(ph->size))) return ph;

	hash_lock(ph);
	pn->f_hashcode = f_hashcode;

	for (i = 0; i < ph->size; i++) {
		while ((pa = ph->array[i])) {
			hash_insert(pn, pa->name, pa->pdata); 
			hash_delete(ph, pa->name);
		}
	}

	/* exchange ph & pn containts: */

    pt = ph->array;
    ph->array = pn->array;
    pn->array = pt;
    
    tmp = ph->size;
    ph->size = pn->size;
    pn->size = tmp;
    
    tmp = ph->used;
    ph->used = pn->used;
    pn->used = tmp;
    
    hash_destroy(pn);
    
    hash_unlock(ph);
    return ph;
}

int hash_decimal_code(register phash_t ph, register char *name) {
	register char *p;
	register unsigned int index = 0;

	if (!ph) hash_debug_return(-hash_NULL_HASH);
	if (!name || (strlen(name)) < 1) hash_debug_return(-hash_NULL_KEY);

	p = name;
	while (*p) {
		if (*p <= '9' && *p >= '0') {
			index *= 10;
			index += *p - '0';
		}
		++p;
	}
	return index % ((phash_t) ph)->size;
}

int hash_classic_code(register phash_t ph, register char *name) {
	register unsigned accum;
	register unsigned len;

	if (!ph) hash_debug_return(-hash_NULL_HASH);
	if (!name || (len = strlen(name)) < 1) hash_debug_return(-hash_NULL_KEY);
	accum = 0;
	for (;len > 0; len--) {
		accum <<= 1;
		accum += (unsigned) (*name++ & 0xFF);        
	}
	return accum % ((phash_t) ph)->size;
}

/* private atom manipulation */
static patom_t _hash_atom_new(phash_t ph, char *name, void *pdata) {
	patom_t pa = NULL;
	if (!(pa = (patom_t) mem_zmalloc(sizeof(atom_t)))) return NULL;
	/*memset((char *) pa, 0, sizeof(atom_t));*/

	if (!(pa->name = mem_strdup(name))) {
		mem_free(pa);
	}
	pa->pdata = pdata;	
	ph->used++;
	return pa;
}

static void *_hash_atom_destroy(phash_t ph, patom_t pa) {
	void *p = NULL;
	if (!ph) {
		err_error("%s", hash_errmsgs[hash_NULL_HASH]);
		return NULL;
	}
 	if (!pa) {
		err_error("%s", hash_errmsgs[hash_NULL_ATOM]);
		return NULL;
	}

	if (pa->name) mem_free(pa->name);
	p = pa->pdata; 
	mem_free(pa);
	ph->used--;
	return p;
}

static patom_t _hash_retrieve(register phash_t ph, char *name) {
	register int index, cmp = 0, nsteps;
	register patom_t pa = NULL;
 
	if (!ph) {
		err_error("%s", hash_errmsgs[hash_NULL_HASH]);
		return NULL;
	}
		
	#ifdef __HASH_STATISTICS__
	ph->stats.nretrieval++;
	#endif

	index = ph->f_hashcode(ph, name);
	if (index < 0) return NULL;
	if (!(pa = ph->array[index])) return NULL;
	nsteps = 0; 
	//hash_lock(ph);

	while ((cmp = strcmp(name, pa->name)) && pa->next) {
		nsteps++;
		pa = pa->next;
	}

	#ifdef __HASH_STATISTICS__
	ph->stats.nsteps += nsteps;
	#endif

	if (cmp) {
		#ifdef __HASH_STATISTICS__
		ph->stats.notfound++;
		#endif
		//hash_unlock(ph);
		return NULL;
	}
	#ifdef __HASH_STATISTICS__
	ph->stats.nfound++;
	#endif

	//hash_unlock(ph);
	return pa;
}

/* public atom manipulation */
int hash_insert(phash_t ph, char *name, void *pdata) {
	int index, nsteps;
	patom_t pp, pa;

	if (!ph) hash_debug_return(hash_NULL_HASH);
	if (!name || strlen(name) < 1) hash_debug_return(hash_NULL_KEY);
	if (hash_retrieve(ph, name)) hash_debug_return(hash_DUP_KEY);

	index = ph->f_hashcode(ph, name); 
	if (index < 0) return hash_BAD_HASH_INDEX;

	#ifdef __HASH_STATISTICS__
	ph->stats.ninsert++;
	#endif

	nsteps = 0;

	hash_lock(ph);
	if (!(pp = ph->array[index])) {
		if (!(pa = ph->array[index] = _hash_atom_new(ph, name, pdata))) 
			hash_debug_return(hash_CANNOT_INSERT_ATOM);
		pa->prev = NULL;
		pa->next = NULL;
	} else {
		while (pp->next) {
			nsteps++; 
			pp = pp->next;
		}
		if (!(pa = _hash_atom_new(ph, name, pdata))) hash_debug_return(hash_CANNOT_INSERT_ATOM);
		pp->next = pa;
		pa->prev = pp;
	}
	#ifdef __HASH_STATISTICS__
	ph->stats.nisteps += nsteps;
	#endif

	hash_unlock(ph);

	return hash_OK;
}

void *hash_retrieve(phash_t ph, char *name) {
	patom_t pa = NULL;

	hash_lock(ph);
	pa = _hash_retrieve(ph, name);
	hash_unlock(ph);
	if (pa) return pa->pdata;

	return NULL;
}

static void *_hash_delete(phash_t ph, char *name) {
	void *p = NULL;
	patom_t pa = NULL;
	pa = _hash_retrieve(ph, name);
	if (pa) {
		if (pa->prev) pa->prev->next = pa->next;
		else if (pa->next) ph->array[ph->f_hashcode(ph, pa->name)] = pa->next;
		else ph->array[ph->f_hashcode(ph, pa->name)] = NULL;
		if (pa->next) pa->next->prev = pa->prev;
			
		p = _hash_atom_destroy(ph, pa);
	}
	return p;
}

void *hash_delete(phash_t ph, char *name) {
	void *p = NULL;
	hash_lock(ph);
	p = _hash_delete(ph, name);
	hash_unlock(ph);
	return p;
}

phash_t 
hash_rehash(phash_t ph, int size) {
	patom_t a;
	phash_t h;
	int i;

	if (!ph) return ph;
	
	hash_lock(ph);

	/* count the required numbre of slots: */
	if (!(h = hash_new_set(size, ph->f_hashcode))) {
		err_error("cannot rehash");
		return ph;
	}

	for (i = 0; i < ph->size; i++) 
		for (a = ph->array[i]; a; a = a->next) 
			hash_insert(h, a->name, a->pdata);
		
	hash_unlock(ph);
	hash_destroy(ph);
	
	return h;
}

/* debug */

#ifdef __HASH_STATISTICS__

void hash_stats_reset(phash_t ph) {
	if (!ph) return;
	hash_lock(ph);
	memset((char *) &ph->stats, 0, sizeof(stats_t));
	hash_unlock(ph);
}

void hash_stats(phash_t ph) {
	int i, j, fslot = 0, maxj = 0, totj = 0;
	patom_t pa = NULL; 
	if (!ph) return;

	hash_lock(ph);
	for (i = 0; i < ph->size; i++) {
		pa = ph->array[i];
		j = 0;
		if (pa == 0) fslot++;
		while (pa && pa->next) {
			j++;
			pa = pa->next;
		}
		totj += j;
		if (j > maxj) maxj = j;
	}
	err_info("Hash statistics:");
	err_info("----------------");
	err_info("hash size     = %8d", ph->size);
	err_info("hash used     = %8d", ph->used);
	err_info("Free Slots    = %8d", ph->size - (ph->used < ph->size ? ph->used : ph->size));
	err_info("%% occupation  = % 11.2f", (double) ph->used / (double) ph->size * 100.);
	err_info("unsused slots = %8d", fslot);
	err_info("max slot load = %8d", maxj);
	err_info("tot slot load = %8d", totj);
	err_info("avg slot load = % 11.2f", (double) totj / (double) ph->size);
	err_info("n found       = %8d", ph->stats.nfound);
	err_info("n not found   = %8d", ph->stats.notfound);
	err_info("nretrieval    = %8d", ph->stats.nretrieval);
	err_info("n ret steps   = %8d", ph->stats.nsteps);
	err_info("avg steps     = % 11.2f", (double) ph->stats.nsteps / (double) ph->stats.nretrieval);
	err_info("ninsert       = %8d", ph->stats.ninsert);
	err_info("ninsert steps = %8d", ph->stats.nisteps);
	err_info("avg ins steps = % 11.2f", ph->stats.ninsert != 0 ? 
		   (double) ph->stats.nisteps / (double) ph->stats.ninsert : 0.);
	hash_unlock(ph);
}

void hash_table_stats(phash_t ph) {
	int i, j, fslot = 0, maxj = 0, totj = 0;
	patom_t pa = NULL; 

	if (!ph) return;
	
	hash_lock(ph);

	for (i = 0; i < ph->size; i++) {
		pa = ph->array[i];
		j = 0;
		if (pa == 0) fslot++;
		while (pa && pa->next) {
			j++;
			pa = pa->next;
		}
		totj += j;
		if (j > maxj) maxj = j;
	}
	err_info("%d %d %d %f %d %d %d %f %d %d %d %d %f %d %d %f", 
		   ph->size, 
		   ph->used, 
		   ph->size - (ph->used < ph->size ? ph->used : ph->size), 
		   (double) ph->used / (double) ph->size * 100., 
		   fslot, 
		   maxj,
		   totj, 
		   (double) totj / (double) ph->size, 
		   ph->stats.nfound, 
		   ph->stats.notfound, 
		   ph->stats.nretrieval, 
		   ph->stats.nsteps, 
		   (double) ph->stats.nsteps / (double) ph->stats.nretrieval, 
		   ph->stats.ninsert, 
		   ph->stats.nisteps, 
		   ph->stats.ninsert != 0 ? (double) ph->stats.nisteps / (double) ph->stats.ninsert : 0.);

	hash_unlock(ph);
}
#endif

void hash_dump(phash_t ph) {
	int i, j, fslot = 0, maxj = 0, totj = 0;
	patom_t pa = NULL; 

	if (!ph)  return;
	
	hash_lock(ph);
	
	err_debug("hash size  = %d", ph->size);
	err_debug("hash used  = %d", ph->used);
	for (i = 0; i < ph->size; i++) {
		pa = ph->array[i];
		j = 0;
		if (pa == 0) fslot++;
		err_debug("hash dump(%08p) %4d.%03d: \t%p -> \"%s\" (prev=%p,next=%p)", 
			   ph, i, j, pa, pa ? pa->name : "", pa ? pa->prev : NULL, pa ? pa->next : NULL);
		while (pa && pa->next) {
			j++;
			pa = pa->next;
			err_debug("hash dump(%08p) %4d.%03d: \t%p -> \"%s\" (prev=%p,next=%p)", ph, i, j, 
				   pa, pa ? pa->name : "", pa ? pa->prev : NULL, pa ? pa->next : NULL);
		}
		totj += j;
		if (j > maxj) maxj = j;
	}
	err_debug("Hash statistics:");
	err_debug("----------------");
	err_debug("hash size     = %8d", ph->size);
	err_debug("hash used     = %8d", ph->used);
	err_debug("unsused slots = %8d", fslot);
	err_debug("max slot load = %8d", maxj);
	err_debug("tot slot load = %8d", totj);
	err_debug("avg slot load = % 11.2f", (double) totj / (double) ph->size);

	#ifdef __HASH_STATISTICS__
	err_debug("n found       = %8d", ph->stats.nfound);
	err_debug("n not found   = %8d", ph->stats.notfound);
	err_debug("nretrieval    = %8d", ph->stats.nretrieval);
	err_debug("n ret steps   = %8d", ph->stats.nsteps);
	err_debug("avg steps     = % 11.2f", (double) ph->stats.nsteps / (double) ph->stats.nretrieval);
	err_debug("ninsert       = %8d", ph->stats.ninsert);
	err_debug("ninsert steps = %8d", ph->stats.nisteps);
	err_debug("avg ins steps = % 11.2f", ph->stats.ninsert != 0 ? 
		   (double) ph->stats.nisteps / (double) ph->stats.ninsert : 0.);
	#endif

	hash_unlock(ph);
}

#ifdef _test_hash_

#define RGBFILE "rgb.txt"
#define NAMESIZE 64
#define loops   1000

typedef struct {
	char *name;
	int r, g, b;
} rgb_t, *prgb_t;

#include "storage.h"
#include "mem.h"
#include "coverage.h"

int main(void) {
	phash_t ph;
	pstorage_t s;
	int i, n, j;	
	FILE *f;
	char name[NAMESIZE], *cols[800], *t;

	err_init(NULL, err_INFO);

	struct timeval tv, tmp;

	if (!(ph  = hash_new_sized(1000))) {
		err_error("cannot create hash table");
		exit(1);
	}
	
	if (!(s = storage_new(sizeof(rgb_t), 100))) {
		err_error("cannot create storage");
		exit(2);
	}

	if (!(f = fopen(RGBFILE, "r"))) {
		err_error("cannot open %s file", RGBFILE);
		hash_destroy(ph);
		storage_destroy(s);
		exit(3);
	}

	printf("read rgb data from %s", RGBFILE);

	i = 0;
	while (!feof(f)) {
		rgb_t rgb;
		void *p;

		memset(name, 0, NAMESIZE);
		if (fscanf(f, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, name) == 4) { 
			cols[i] = rgb.name = mem_strdup(name);
			p = storage_add(s, (char *) &rgb);
			hash_insert(ph, cols[i], (char *) p);
			i++;
		}
	}	
	n = storage_used(s);

	printf("getting from hash with %d slots :", ph->size); fflush(stdout);
	tv = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		hash_retrieve(ph, cols[i]);
		tv = cov_time_add(tv, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done in %s seconds\n", t =  cov_time_fmt(tv)); fflush(stdout);
	
	
	hash_stats(ph); 

	printf("rehashing with %d slots...", ph->used);
	ph = hash_rehash(ph, ph->used);
	printf("done \n"); fflush(stdout);
	
	printf("getting from hash with %d slots : ", ph->size); fflush(stdout);
	tv = cov_time_zero();
	for (j = 1; j < loops; j++) 
	for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		hash_retrieve(ph, cols[i]);
		tv = cov_time_add(tv, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done in %s seconds\n", t =  cov_time_fmt(tv)); fflush(stdout);
	hash_stats(ph); fflush(stdout);

	hash_destroy(ph);
	storage_destroy(s);

	return 0;
}

#endif
