
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>

#include <time.h>
#include <pthread.h>

typedef struct {
	size_t	  nmalloc;
	size_t 	  nfree;
	size_t	  nrealloc;
	size_t 	  ncalloc;
	size_t	  maxmem;
	size_t	  nonfree;
	size_t    totmem;
} mstats_t, *pmstats_t;

static mstats_t _mstats_;

typedef struct {
void *(*malloc_)(size_t);
void  (*free_)(void *);
void *(*realloc_)(void *, size_t);
void *(*calloc_)(size_t, size_t);
} sys_t;

static sys_t _sys = { NULL, NULL, NULL, NULL };

static int	_mstats_alloc_ = 1;

pthread_mutex_t  *_mstats_lock_;

// static int	_mstats_lock_  = 0;

static void mstats_lock()   { pthread_mutex_lock(_mstats_lock_); }
static void mstats_unlock() { pthread_mutex_unlock(_mstats_lock_); }
static void mstats_off()    { _mstats_alloc_ = 0; }
static void mstats_on()     { _mstats_alloc_ = 1; }

static FILE *mstats_out = NULL;


void
_mstats_fprintf(FILE *f, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(f, fmt, args);
}

void
mstats_fprintf(FILE *f, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	mstats_lock();
	mstats_off();	
	vfprintf(f, fmt, args);
	mstats_on();
	mstats_unlock();
}

#define shift  8

void
mstats_print() {
	mstats_off();
	_mstats_fprintf(mstats_out, "# of malloc  = %u\n", _mstats_.nmalloc);
	_mstats_fprintf(mstats_out, "# of free    = %u\n", _mstats_.nfree);
	_mstats_fprintf(mstats_out, "# of realloc = %u\n", _mstats_.nrealloc);
	_mstats_fprintf(mstats_out, "# of calloc  = %u\n", _mstats_.ncalloc);

	_mstats_fprintf(mstats_out, "max mem      = %u\n", _mstats_.maxmem);
	_mstats_fprintf(mstats_out, "non free     = %u\n", _mstats_.nonfree);
	_mstats_fprintf(mstats_out, "total mem    = %u\n", _mstats_.totmem);
	mstats_on();
	fclose(mstats_out);
}

char *
_mstats_timestamp(char *b) {
	struct tm *t;
	time_t tt;
	time(&tt);
	t = localtime(&tt);
	sprintf(b, "%02d/%02d/%04d %02d:%02d:%02d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
	return b;	
}

char *
mstats_timestamp(char *b) {
	char *p;
	mstats_lock();
	mstats_off();	
	p = _mstats_timestamp(b);	
	mstats_on();
	mstats_unlock();
	return p;
}

void
mstats_dump() {
	char tstamp[30];
		
	mstats_lock();
	mstats_off();	
	_mstats_fprintf(mstats_out, "%s %u %u %u %u %u %u %u\n", _mstats_timestamp(tstamp), 
		_mstats_.nmalloc, _mstats_.nfree, _mstats_.nrealloc, _mstats_.ncalloc, _mstats_.maxmem, _mstats_.nonfree, _mstats_.totmem);
	mstats_on();
	mstats_unlock();
}

void 
mstats_loop() {
	static int delay = 0;

	if (!delay) {
		if (getenv("MSTATS_DELAY")) 
			delay = atoi(getenv("MSTATS_DELAY"));
		if (delay <= 0) delay = 60;
	}
	while (1) {
		mstats_dump();
		sleep(delay);
	}
}

void 
mstats_fini() {
	/* static char b[] = ".mstats.report"; */
	mstats_fprintf(stderr, "in mstats_fini\n");
	mstats_print();
}

#ifdef SOLARIS
#pragma fini (mstats_fini)
#endif
#ifdef LINUX
void mstats_fini() __attribute__((destructor));
#endif

void
mstats_init() {
	char *p;

	if (_sys.malloc_) return;
	
	mstats_off();
	_sys.malloc_  = (void * (*)(size_t))         dlsym(RTLD_NEXT, "malloc");
	_sys.free_    = (void   (*)(void *))         dlsym(RTLD_NEXT, "free");
	_sys.realloc_ = (void * (*)(void *, size_t)) dlsym(RTLD_NEXT, "realloc");
	_sys.calloc_  = (void * (*)(size_t, size_t)) dlsym(RTLD_NEXT, "calloc");
	memset((char *) &_mstats_, 0, sizeof(mstats_t));
	_mstats_lock_ = (pthread_mutex_t *) _sys.malloc_(sizeof(pthread_mutex_t));


	if ((p = getenv("MSTATS_FILE"))) {
		if (!(mstats_out = fopen(p, "w"))) { 
			_mstats_fprintf(stderr, "Cannot open file \"%s\"\n", p);
		}
	} 
	if (!mstats_out) mstats_out = stderr;

	atexit(mstats_fini);

	if (getenv("MSTATS_TRACE")) {
		pthread_attr_t attr;
		pthread_t tid;

		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, (void *(*)(void *)) mstats_loop, NULL);
		
	}	
	_mstats_fprintf(stderr, "in mstats_init\n");
	mstats_on();
}

#ifdef SOLARIS
#pragma init (mstats_init)
#endif
#ifdef LINUX
void mstats_init() __attribute__((constructor));
#endif

void *
malloc(size_t size) {
	char *p;

	if (! _sys.malloc_) mstats_init();


	if (!_mstats_alloc_) {
		void *q;
		q = _sys.malloc_(size);
		return q;
	}

	mstats_lock();
	mstats_off();
	_mstats_fprintf(stderr, "malloc(%d)\n", size); 

	_mstats_.totmem  += size;
	_mstats_.nonfree += size;

	if (_mstats_.nonfree > _mstats_.maxmem) 
		_mstats_.maxmem = _mstats_.nonfree;

	_mstats_.nmalloc++;

	p = (char *) _sys.malloc_(shift + size);
	* (size_t *) p = size;
	mstats_on();
	mstats_unlock();
	return (void *) (p + shift);
}

size_t
mstats_size_from_ptr(void *p) {
	return *(size_t *) ((char *) p - shift);
}

void
free(void *p) {
	char *q;
	size_t size;

	if (!p) return;
	if (! _sys.free_) mstats_init();

	if (!_mstats_alloc_) {
		_sys.free_(p);
		return;
	}

	mstats_lock();
	mstats_off();
	_mstats_fprintf(stderr, "free(%x)\n", p);

	q = (char *) p - shift;
	_mstats_.nfree++;
	size = *(size_t *) q;	
	_mstats_.nonfree -= size;
	
	_sys.free_(q);
	mstats_on();
	mstats_unlock();
}

void *
realloc(void *p, size_t size) {
	void *n;

	if (!_sys.realloc_) mstats_init();

	if (!_mstats_alloc_) {
		void *q;
		q = _sys.realloc_(p, size);
		return q;
	}

	mstats_lock();
	mstats_off();
	_mstats_fprintf(stderr, "realloc(%x, %d)\n", p, size);

	_mstats_.nrealloc++;
	_mstats_.nfree--;
	_mstats_.nmalloc--;

	if (!(n = malloc(size))) {
		_mstats_fprintf(stderr, "cannot rellocate\n");
		mstats_on();
		mstats_unlock();
		return n;
	}
	if (p) { 
		/* realloc not used as malloc: */
		memcpy(n, p, size);
		free(p);
	}

	mstats_on();
	mstats_unlock();
	return n;
}

void * 
calloc(size_t nelem, size_t elsize) {
	char *p;

	if (!_sys.calloc_) mstats_init();

	if (!_mstats_alloc_) {
		void *q;
		q =  _sys.calloc_(nelem, elsize);
		return q;
	}

	mstats_lock();
	mstats_off();
	_mstats_.ncalloc++;
	_mstats_.nmalloc--;

	_mstats_fprintf(stderr, "calloc(%u, %u)\n ", nelem, elsize);

	p = (char *) malloc(nelem * elsize);
	memset(p, 0, nelem * elsize);
	mstats_on();
	mstats_unlock();
	return p;
}

void *memalign(size_t alignment, size_t size) {
	mstats_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return NULL;
}
void *valloc(size_t size) {
	mstats_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return NULL;
}

int brk(void *endds) {
	mstats_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return 0;
}

void *sbrk(intptr_t incr) {
	mstats_fprintf(stderr, "************************%s fuction not implemented ************************\n", __func__);
	return NULL;
}

