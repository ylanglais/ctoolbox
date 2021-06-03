
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>

typedef struct {
	size_t 	size;
	int 	amount;
	double	avg;
	double  sigma;
	double  variance;
	double	filling;
} amsize_t, *pamsize_t;

typedef struct {
	int	npolls;
	amsize_t  amsizes[17];
	long 	  nspecials;
	size_t	 *specials;
	size_t	  nmalloc;
	size_t 	  nfree;
	size_t	  nrealloc;
	size_t 	  ncalloc;
	size_t	  maxmem;
	size_t	  nonfree;
	size_t    totmem;
} mprof_t, *pmprof_t;

static mprof_t _mprof_;

typedef struct {
void *(*malloc_)(size_t);
void  (*free_)(void *);
void *(*realloc_)(void *, size_t);
void *(*calloc_)(size_t, size_t);
} sys_t;

static sys_t _sys = { NULL, NULL, NULL, NULL };

static int	_mprof_alloc_ = 1;
static int	_mprof_lock_  = 0;

static void mprof_lock()   { while (_mprof_lock_); _mprof_lock_ = 1; }
static void mprof_unlock() { _mprof_lock_ = 0; }
static void mprof_off()    { _mprof_alloc_ = 0; }
static void mprof_on()     { _mprof_alloc_ = 1; }

void
mprof_fprintf(FILE *f, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	mprof_off();	
	vfprintf(f, fmt, args);
	mprof_on();
}

#define shift  8

int
mprof_save(char *filename) {
	int i;
	FILE *f;

	if (!(f = fopen(filename, "w"))) {
		mprof_fprintf(stderr, "Cannot open file \"%s\"\n", filename);
		return 2;
	}
	
	mprof_fprintf(f, "npolls = %d\n", _mprof_.npolls);
	
	for (i = 0; i < 17; i++) {
		if (_mprof_.amsizes[i].amount == 0) continue;
		mprof_fprintf(f, "2^%2d      = %u :\n", i, 1 << i);
		mprof_fprintf(f, "\tsize     = %u\n", _mprof_.amsizes[i].size);
		mprof_fprintf(f, "\tamount   = %d\n",  _mprof_.amsizes[i].amount);
		mprof_fprintf(f, "\tavg      = %f\n",  _mprof_.amsizes[i].avg =  (double) _mprof_.amsizes[i].size / (double) _mprof_.amsizes[i].amount);
		mprof_fprintf(f, "\tsigma    = %f\n",  _mprof_.amsizes[i].sigma);
		mprof_fprintf(f, "\tvariance = %f\n",  _mprof_.amsizes[i].variance);
		if (_mprof_.amsizes[i].size) 
			mprof_fprintf(f, "\tfilling  = %f\n",  (double) _mprof_.amsizes[i].size * 100. / (double) (1 << i) / (double) _mprof_.amsizes[i].amount);
		else 
			mprof_fprintf(f, "\tfilling  = 100\n");
	}
	mprof_fprintf(f, "%d special blocks;\n", _mprof_.nspecials);
	for (i = 0; i < _mprof_.nspecials; i++) {
		mprof_fprintf(f, "\tsize = %u\n", _mprof_.specials[i]);
	}
	mprof_fprintf(f, "# of malloc  = %u\n", _mprof_.nmalloc);
	mprof_fprintf(f, "# of free    = %u\n", _mprof_.nfree);
	mprof_fprintf(f, "# of realloc = %u\n", _mprof_.nrealloc);
	mprof_fprintf(f, "# of calloc  = %u\n", _mprof_.ncalloc);

	mprof_fprintf(f, "max mem      = %u\n", _mprof_.maxmem);
	mprof_fprintf(f, "non free     = %u\n", _mprof_.nonfree);
	mprof_fprintf(f, "total mem    = %u\n", _mprof_.totmem);
	fclose(f);
	return 0;
}

void 
mprof_fini() {
	static char b[] = ".mprof.report";
	char  *p;
	mprof_fprintf(stdout, "in mprof_fini\n");
	mprof_off();
	if (!(p = getenv("MPROF_FILE"))) {
		p = b;
	} 
	mprof_save(p);
}

#ifdef SOLARIS
#pragma fini (mprof_fini)
#endif
#ifdef LINUX
void mprof_fini() __attribute__((destructor));
#endif

void
mprof_init() {
	if (_sys.malloc_) return;
	_sys.malloc_  = (void * (*)(size_t))         dlsym(RTLD_NEXT, "malloc");
	_sys.free_    = (void   (*)(void *))         dlsym(RTLD_NEXT, "free");
	_sys.realloc_ = (void * (*)(void *, size_t)) dlsym(RTLD_NEXT, "realloc");
	_sys.calloc_  = (void * (*)(size_t, size_t)) dlsym(RTLD_NEXT, "calloc");
	memset((char *) &_mprof_, 0, sizeof(mprof_t));
	_mprof_.specials  = NULL;
	_mprof_.nspecials = 0;
	
	atexit(mprof_fini);
	mprof_fprintf(stdout, "in mprof_init\n");
}

#ifdef SOLARIS
#pragma init (mprof_init)
#endif
#ifdef LINUX
void mprof_init() __attribute__((constructor));
#endif

#if 0
pmprof_t
mprof_load(char *filename) {
}
#endif

size_t
mprof_closest_upper_power(size_t val) {
	size_t n;
	for (n = 1; n < val ; n <<= 1) ; /* printf("\t%d\n", n); */
	return n;
}

int
mprof_power_of_2(size_t val) {
	int i;
	for (i = 1; val > (1 << i); i++);
	return i;
}

void
mprof_upper_closest_pow_of_2(size_t val, int *pwr, size_t *closest) {
	int i, n;
	for (i = 1, n = 1; n < val; n = 1 << ++i);
	*pwr   = i;
	*closest = n;
}

void *
malloc(size_t size) {
	char *p;

	if (! _sys.malloc_) mprof_init();

	mprof_lock();
	if (!_mprof_alloc_) {
		void *q;
		q = _sys.malloc_(size);
		mprof_unlock();
		return q;
	}

	_mprof_.totmem  += size;
	_mprof_.nonfree += size;
	if (_mprof_.nonfree > _mprof_.maxmem) 
		_mprof_.maxmem = _mprof_.nonfree;

	_mprof_.nmalloc++;

	mprof_fprintf(stdout, "malloc(%u)", size); 

	if (size < 65536) {
		int i;
		i = mprof_power_of_2(size);
		_mprof_.amsizes[i].size += size;
		_mprof_.amsizes[i].amount++;
	} else {

		_mprof_.nspecials++;
		if (!_mprof_.specials) {
			if (!(_mprof_.specials = (size_t *) _sys.malloc_(sizeof(size_t) * _mprof_.nspecials))) {
				_mprof_.specials[_mprof_.nspecials - 1] = size;
			} else {
				mprof_fprintf(stderr, "pb w/ counting special blocs (alloc ko) (nspecials = %d)\n", _mprof_.nspecials);
				_mprof_.nspecials = 0;
				mprof_fprintf(stdout, " failed\n");
			}
	
		} else if ((p = (char *) _sys.realloc_(_mprof_.specials, _mprof_.nspecials * sizeof(pamsize_t)))) {
			_mprof_.specials = (size_t *) p;
			_mprof_.specials[_mprof_.nspecials - 1] = size;
		} else {
			mprof_fprintf(stderr, "pb w/ counting special blocs (realloc ko)\n");
			mprof_fprintf(stdout, " failed\n");
		}
	}
	p = (char *) _sys.malloc_(shift + size);
	* (size_t *) p = size;
	mprof_unlock();
	mprof_fprintf(stdout, " -> %x\n", (p + shift));
	return (void *) (p + shift);
}

size_t
mprof_size_from_ptr(void *p) {
	return *(size_t *) ((char *) p - shift);
}

void
free(void *p) {
	char *q;
	size_t size;

	if (!p) return;
	if (! _sys.free_) mprof_init();
	mprof_lock();

	if (!_mprof_alloc_) {
		_sys.free_(p);
		mprof_unlock();
		return;
	}

	q = (char *) p - shift;
	_mprof_.nfree++;
	size = *(size_t *) q;	
	_mprof_.nonfree -= size;
	
	_sys.free_(q);
	memset(q, 0, size);
	mprof_unlock();

	mprof_fprintf(stdout, "free(%x)\n", p);
}

void *
realloc(void *p, size_t size) {
	void *n;

	if (!_sys.realloc_) mprof_init();

	if (!_mprof_alloc_) {
		void *q;

		mprof_lock();
		q = _sys.realloc_(p, size);
		mprof_unlock();
		return q;
	}

	_mprof_.nrealloc++;
	_mprof_.nfree--;
	_mprof_.nmalloc--;

	mprof_fprintf(stdout, "realloc(%x, %u)", p, size);

	if (!(n = malloc(size))) {
		mprof_fprintf(stderr, "cannot rellocate\n");
		return n;
	}
	if (p) { 
		/* realloc not used as malloc: */
		memcpy(n, p, size);
		free(p);
	}

	return n;
}

void * 
calloc(size_t nelem, size_t elsize) {
	char *p;

	if (!_sys.calloc_) mprof_init();

	if (!_mprof_alloc_) {
		void *q;

		mprof_lock();
		q =  _sys.calloc_(nelem, elsize);
		mprof_unlock();
		return q;
	}

	_mprof_.ncalloc++;
	_mprof_.nmalloc--;

	mprof_fprintf(stdout, "calloc(%u, %u) ", nelem, elsize);

	p = (char *) malloc(nelem * elsize);
	memset(p, 0, nelem * elsize);
	return p;
}

void *memalign(size_t alignment, size_t size) {
	mprof_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return NULL;
}
void *valloc(size_t size) {
	mprof_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return NULL;
}

int brk(void *endds) {
	mprof_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return 0;
}

void *sbrk(intptr_t incr) {
	mprof_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return NULL;
}

