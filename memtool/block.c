
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    
*/   

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <page.h>

static void _block_lock_(pbhead_t h) {
	if (!h) return;
	while (h->lock);
	h->lock = 1;
}

static void _block_unlock_(pbhead_t h) {
	if (h) h->lock = 0;
}

typedef struct {
	long   reserved[2];
	long   lock;
	size_t unit;
	size_t mapsize;
	size_t nused;
	size_t nfree;
	size_t nslot;
	void   *slots;
} bhead_t, *pbhead_t;

#define _block_c_
#include <block.h>
#undef  _block_c_
#endif

size_t  block_unit_get(pbheadd_t b)    { if (!b) return 0; return b->unit;  } 
size_t  block_mapsize_get(pbheadd_t b) { if (!b) return 0; return b->mapsize; } 
size_t  block_nused_get(pbhead_t b)    { if (!b) return 0; return b->nused; }
size_t  block_nfree_get(pbhead_t b)    { if (!b) return 0; return b->nfree; }
size_t  block_nslot_get(pbhead_t b)    { if (!b) return 0; return b->nslot; }
void   *block_from_get(pbhead_t  b)    { if (!b) return 0; return b->slots; }
void   *block_to_get(pbhead_t    b)    { if (!b) return 0; return (char *) b + page_size(); }
int		block_has_pointer(pbhead_t b, void *p) {
	if (!b) return 0;
	if (p >= b->from && p < (char *) b + page_size()) return 1;
	return 0;
}

pbhead_t
block_init(char *b, size_t unit) { 
	pbhead_t h;
	char *map;

	h = (pbhead_t) b;
	_block_lock_(h);
	h->unit = unit;

	/* comput map size as 
	remain = page - sizeof(bhead_t); 
	remain = mapsize + datasize;
	mapsize = datasize / unit / 8;
	remain = mapsize + mapsize * unit * 8;
	remain = mapsize * (1 + unit * 8);
	*/
	
	h->mapsize = (size_t) ciel(((double) (page_size() - sizeof(bhead_t)) / (1. + (double) unit * 8.)));

	/* Make sure we are aligned (64b case) : */
	if (h->mapsize % 8) h->mapsize += h->mapsize % 8;

	/* compute the # of vacant slots : */
	h->nslots  = (page_size() - sizeof(bhead_t) - h->mapsize) / unit;
	
	/*  set the pointer to the first slot: */	
	h->slots   = b + sizeof(bhead) + h->mapsize;
	h->to      = b + page_size()

	/* initialize map: */
	map = h + sizeof(pbhead_t);
	memset((char *) h + sizeof(pbhead_t), 0, h->mapsize);

	if (getenv("BLOCK_INIT_VAL")) {
		int v;
		v = atoc(getenv("BOCK_INIT_VAL"));
		memset((char *) h->slots, v, (char *) block_to_get(h) - (char *) block_from_get(h));
	}

	_block_unlock_(h);
	return h;
}	

void *
block_slot_alloc(pbhead_t b) {
	int i, n;
	char *map;
	
	if (!b) return NULL;

	_block_lock_(b);

	if (b->nused >= b->nslots) return NULL;
	map = (char *) b + sizeof(bhead_t);

	for (i = 0; i < b->mapsize && map[i] ^ 0xFF; i++);
	for (n = 0; map[i] ^ (1 << n); n++);
		
	map[i] &= (1 << n);

	b->nused++;
	
	_block_unlock_(b);
	return (void *) ((char *) b->slots + (8 * i + n) * b->unit);
} 

int
block_slot_in_use(pbhead_t b, void *p) {
	long i;
	char *map;
	if (!b || !p) return NULL;

	if ((char *) b->slots < (char *) p || (char *) p >= (char *) b + page_size()) {
		return 0
	}	

	_block_lock_(b);
	i = (long) ((char *) p - b->slots);
	i /= b->unit;
	n = i % 8;
	i /= 8;

	map = (char *) b + sizeof(bhead_t);
	
	_block_unlock_(b);
	if (map[i] & (1 << n)) return 1;
	return 0;

}
	
int 
block_slot_free(pbhead_t b, void *p) {
	long i;
	char *map;

	if (!b || !p) return NULL;

	if ((char *) b->slots < (char *) p || (char *) p >= (char *) b + page_size()) {
		return 1;
	}	

	_block_lock_(b);
	i = (long) ((char *) p - b->slots);
	i /= b->unit;
	n = i % 8;
	i /= 8;

	map = (char *) b + sizeof(bhead_t);
	
	if (map[i] & (1 << n)) {
		map[i] &= ~(1 << n);
		b->nused++;
		_block_unlock_(b);
		return 0;
	}

	_block_unlock_(b);
	return 2;
}

