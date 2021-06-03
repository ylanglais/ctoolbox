
typedef struct {
	size_t size;
	int    amount;
}


typedef struct {
	
} alprof_t, *palprof_t;


#include <block.h>
#include <page.h>

typedef struct {
	int nspecial;
	pblocks_t specials;
} 


self_malloc() {
}


void
alloc_init(page_type_t type, int param, char *profile, int profcode) {
}

void
alloc_new() {
}

void
alloc_destroy() {
}

void *
alloc_malloc(size_t size) {

}

void *
alloc_realloc(void *p, size_t newsize) {
}

void *
alloc_calloc(size_t nelem, size_t elsize) {
}

void 
alloc_free(void *p) {
}

void 
alloc_stats_init() {
}

alloc_stats_print() {
}

void 
alloc_profile_save(char *filename, palprof_t p) {
}

palprof_t
alloc_profile_read(char *filename, palprof_t p) {
}


