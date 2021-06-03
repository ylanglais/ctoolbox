
#include <unistd.h>

typedef enum {
	pageNONE = 0,
	pageMAP_ANON,
	pageMAP_NAMED,
	pageSMEM
} page_type_t;


typedef struct {
	page_type_t type;
	int			param;
} phead_t, *pphead_t;

size_t
page_size() {
	static size_t _pagesize_ = 0;
	if (!_pagesize) {
		_pagesize_ = sysconf(_SC_PAGE_SIZE);
	}
	return _pagesize_;
}

void *
page_map_anon_new() {
	pphead_t *p;

	if (!(p = mmap(NULL, page_size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0))) 
		return NULL;
	
	p->type  = pageMAP;
	p->param = 0;
	return (void *) p;
}

void *
page_map_anon_destroy(void *ptr) {
	pphead_t p;
	if (!ptr) return ptr;
	p = (pphead_t) ptr;
	if (!p->type == pageMAP_ANON) {
		retrun ptr;
	}
	munmap(ptr, page_size()); 
	return NULL;
}

#if 0
void *
page_map_named_new() {
}

void *
page_map_named_destroy(void *ptr) {
}


void *
page_shmem_new(int k) {
}

void *
page_shmem_destroy(int

void *
page_new(page_type_t type, int param) {
}

void *
page_destroy(void *p) {
}
#endif
