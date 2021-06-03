#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include <stack.h>
#include <tbx/fmt.h>

#define nddal_HEAD_LEN 32
#define nddal_TAIL_LEN 32
/* #define nddal_STACKBUFFERLEN (1024 * 1024 * sizeof(void *)) */
#define nddal_STACKBUFFERLEN 100000000 

#define ALIGN 8 
#define nddal_BUFF 1000
#define nddal_ERR  2
#define nddal_OUT  1

enum {nddal_Unset, nddal_Used, nddal_Free};
static char *nddal_status_str[] = { "Unset", "InUse", "Freed" };

typedef struct {
    unsigned long nmalloc;
    unsigned long nrealloc;
    unsigned long nfree;
} stat_t, *pstat_t;

static stat_t _nddal_stats;

typedef struct _nddal_block_head_t {
    size_t  size;
    size_t  status;
    struct _nddal_block_head_t *prev;
    struct _nddal_block_head_t *next;
	void *  pad;
    int     nstacks;
    void   *pstack;
    char    magic_head[nddal_HEAD_LEN];
    char    align64[12];
} nddal_block_head_t;

typedef struct {
    char    magic_tail[nddal_TAIL_LEN];
    char    align64[nddal_TAIL_LEN];
} nddal_block_tail_t;

static void nddal_print(char *s);
static void nddal_check_block(nddal_block_head_t *hb);
static void _nddal_check_all();

void * _nddal_morecore(size_t size);
void *_malloc(size_t size);

typedef void *(*fcore)(size_t size);
static fcore _nddal_sbrk                = NULL;

static size_t  _nddal_private_len       = 0;
static void   *_nddal_private           = NULL;
static void   *_nddal_base              = NULL;
static void   *_nddal_curr              = NULL;

static char  _nddal_stack               = 0;
static char  _nddal_dbg                 = 0;
static char  _nddal_dump                = 0;
static char  _nddal_realloc_as_malloc   = 0;
static char  _nddal_always_check        = 0;
static char  _nddal_sighandle           = 0;
static char  _nddal_no_free_null        = 0;
static char  _nddal_no_realloc_truncate = 0;

static int _nddal_lock                  = 0;

static char nddal_magic_head[] = "#MeM cHeCkEr: HeAdErMaGicStRiNg";
static char nddal_magic_tail[] = "*mEm ChEcKeR: tAiLeRmAgICsTrInG";

/* nddal locking: */
static void nddal_priv_lock()  { 
	return;
	//if (!_nddal_stack) return;
    if (mprotect(_nddal_private, _nddal_private_len, PROT_NONE))
        nddal_print("mprotect lock FAILED\n");
}

static void nddal_priv_read()  { 
	return;
	//if (!_nddal_stack) return;
    if (mprotect(_nddal_private, _nddal_private_len, PROT_READ))
        nddal_print("mprotect read unlock FAILED\n");
}

static void nddal_priv_write() { 
	return;
	//if (!_nddal_stack) return;
    if (mprotect(_nddal_private, _nddal_private_len, PROT_READ | PROT_WRITE))
        nddal_print("mprotect write unlock FAILED\n");
}

static void nddal_lock()   {
    while (_nddal_lock);
    _nddal_lock = 1;
}

static void nddal_unlock() {
    _nddal_lock = 0; 
}

/* Signal handling: */
void
_nddal_signals(int sig) {
	char t[10];
    nddal_print("nddal caught signal ");
	nddal_print(fmt_u(t, sig));
	if (_nddal_stack) {
		nddal_print("\nHere is the callstack when signal has been emitted :\n");
		stack_live_dump();
	}
    nddal_print("\nNow preform memory check...\n");
    _nddal_check_all();
    nddal_print("done\n");
}

/* alignment: */
static size_t 
nddal_aligned(size_t size) {
    unsigned int residue;
    if ((residue = size % ALIGN))
        size += (ALIGN - residue);
    return size;
}

void _nddal_init() {
	char b[1024];
	size_t stackbufsize;
    long pagesize;
    pagesize = sysconf(_SC_PAGE_SIZE);
	void  *_nddal_stack_from = 0;

	/* nddal_print("_nddal_init\n"); */

    /* fetch real "mem core function" */
    _nddal_sbrk = (fcore) dlsym(RTLD_NEXT, "sbrk");

    /* make sure we start on pagesize multiple */
    if (((size_t) _nddal_sbrk(0)) % pagesize) _nddal_morecore(pagesize - ((size_t) _nddal_sbrk(0) % pagesize));

    _nddal_private = _nddal_sbrk(0);

	if (getenv("NDDAL_DBG"))                 _nddal_dbg                 = 1;
	if (getenv("NDDAL_DUMP"))                _nddal_dump                = 1;
	if (getenv("NDDAL_STACK"))               _nddal_stack               = 1;
	if (getenv("NDDAL_REALLOC_AS_MALLOC"))   _nddal_realloc_as_malloc   = 1;
	if (getenv("NDDAL_ALWAYS_CHECK"))        _nddal_always_check        = 1;
	if (getenv("NDDAL_SIGHANDLE"))			 _nddal_sighandle           = 1;
	if (getenv("NDDAL_NO_FREE_NULL"))        _nddal_no_free_null        = 1;
	if (getenv("NDDAL_NO_REALLOC_TRUNCATE")) _nddal_no_realloc_truncate = 1;

	nddal_print("_nddal_private = ");
	fmt_ptr(b, _nddal_private);
	nddal_print(b);
	nddal_print("\n");

	stackbufsize = nddal_STACKBUFFERLEN;
   	if (getenv("NDDAL_STACKBUFFSIZE")) {
		stackbufsize = atol(getenv("NDDAL_STACKBUFFSIZE"));
	}

	_nddal_stack_from = _nddal_sbrk(stackbufsize * sizeof(void *));
	stack_init(_nddal_private, stackbufsize * sizeof(void *));

    /* make sure we start on pagesize multiple */
    if (((size_t) _nddal_sbrk(0)) % pagesize) _nddal_morecore(pagesize - ((size_t) _nddal_sbrk(0) % pagesize));

	if (_nddal_sighandle) signal(SIGSEGV, _nddal_signals);


    _nddal_base = _nddal_sbrk(0);

	nddal_print("_nddal_base = ");
	fmt_ptr(b, _nddal_base);
	nddal_print(b);
	nddal_print("\n");

    _nddal_private_len = (char *) _nddal_base - (char *) _nddal_private;
    _nddal_stats.nmalloc  = 
    _nddal_stats.nrealloc = 
    _nddal_stats.nfree    = 0;

    nddal_priv_lock();       
    
    atexit(_nddal_check_all);
}

#ifdef SOLARIS
#pragma init (_nddal_init)
#endif
#ifdef LINUX
void _nddal_init() __attribute__((constructor));
#endif

static void *_nddal_malloc(size_t size) {
    static int         count = 0;
    size_t             gsize, real;
    void               *p; 
    char               *q, *r;
    nddal_block_head_t *h;

    /* 
     * compute actual reservation size as : 
     * header + tailer + size (adjusted)
     */
    
    h = _nddal_curr;

    gsize = nddal_aligned(size);
    real = gsize + sizeof(nddal_block_head_t) + sizeof(nddal_block_tail_t);

    if ((p = _nddal_morecore(real)) == (void *) -1) return NULL;
    if ((unsigned long) p % ALIGN) { 
		char t[50];
        nddal_print("ALIGNMENT ERROR: ");
		nddal_print(fmt_ptr(t, p));
		nddal_print(" % ");
		nddal_print(fmt_u(t, ALIGN));
		nddal_print(" = ");
		nddal_print(fmt_u(t, ((unsigned long) p) % ALIGN));
		nddal_print("\n");
	}

    if (count) h->next = p;
    
    h = (nddal_block_head_t *) (q = (char *) p);

    h->size   = gsize;
    h->status = nddal_Used;
    h->next   = NULL;
	h->prev   = _nddal_curr;

    strcpy(h->magic_head, nddal_magic_head);

	/* Fill the allocated but not used bytes withs 'X' to check small leaks: */
    if (size < gsize) memset(q +  sizeof(nddal_block_head_t) + size, 'X', gsize - size);
    strcpy(q + sizeof(nddal_block_head_t) + gsize, nddal_magic_tail);

    /* Find out a way to protect small amounts of mem HERE!!! */

    nddal_priv_write();
    h->pstack  = stack_curr_get();
    h->nstacks = stack_trace();
    nddal_priv_lock();

    _nddal_curr = (void *) h;
    count++;

    if (_nddal_dbg) {
		char t[50];
		nddal_print("--> new block @ 0x");
		nddal_print(fmt_ptr(t,  h));
		nddal_print("\n");
	}
    return q + sizeof(nddal_block_head_t);
}

static void _nddal_free(void *ptr) {
    char *p;
    nddal_block_head_t *hb;

    p = (char *) ptr;
    hb = (nddal_block_head_t *) (p - sizeof(nddal_block_head_t));

    nddal_check_block(hb);
    hb->status = nddal_Free;
}

/* writing and dumping: */
static void 
nddal_print(char *s) {
	if (s) write(2, s, strlen(s));
}

static void _nddal_dump_data(char *buff, size_t size) {
    int i, j;
    char hex[256], asc[256], tmp[512], final[512];
	char *h, *a, *t, *f;

    if (buff == NULL) {
       	nddal_print("   null pointer\n");
        return;
    }

    nddal_print("   block dump  :\n");
	
    /* 00 11 22 33 44 55 66 77 - 88 99 AA BB CC DD EE FF   azertyui - qsdfghjk */
    for (i = 0; i < size; i += 16) {
        *hex = *asc = *final = 0;
		h = hex, a = asc, t = tmp;
        for (j = 0; j < 16 && i + j < size; j++) {
            if (j == 8) {
                h = fmt_str_append(h, "- ");
                a = fmt_str_append(a, " - ");
            }
			h = fmt_str_append(h, fmt_xchar(tmp, (unsigned char) buff[i+j]));
			h = fmt_str_append(h, " ");
			a = fmt_str_append(a, fmt_char(tmp, buff[i + j]));
			h = fmt_str_append(h, " ");
        }
		nddal_print("      ");	
		nddal_print(fmt_ptr(tmp, buff + i)); 
		nddal_print(": ");
		nddal_print(hex);
		nddal_print("  ");
		nddal_print(asc);
        nddal_print("\n");
    }
}

static void
_nddal_hdr_dump(nddal_block_head_t *h) {
	char t[500];

    if (!h) return;
    nddal_print("haddr  = ");
	nddal_print(fmt_ptr(t, h));
	nddal_print("\n");
	nddal_print("size   = ");
	nddal_print(fmt_u(t, h->size));
	nddal_print("\n");

    if (h->status < nddal_Unset || h->status > nddal_Free) {
         nddal_print("invalid status\n");
    } else { 
		nddal_print("status = ");
		nddal_print(nddal_status_str[h->status]);
		nddal_print("\n");
	}
    nddal_print("next   = ");
	nddal_print(fmt_ptr(t, h->next));
	nddal_print("\n");
	nddal_print("magic1 = \"");
	nddal_print(h->magic_head);
	nddal_print("\n");
	nddal_print("magic2 = \"");
	nddal_print((char *) h + sizeof(nddal_block_head_t) + h->size);
	nddal_print("\n");
    nddal_print("---------------------------\n\n");
}

static void _nddal_stats_print() {
	char t[50];

    nddal_print("nddal stats:\n");
    nddal_print("nddal stats: memory statistics:\n");
    nddal_print("nddal stats: ------------------\n");
    nddal_print("nddal stats: malloc:       ");
	nddal_print(fmt_u_n(t,	_nddal_stats.nmalloc, 8, ' '));
    nddal_print("\nnddal stats: realloc:      ");
	nddal_print(fmt_u_n(t,	_nddal_stats.nrealloc, 8, ' '));
    nddal_print("\nnddal stats: free:         ");
	nddal_print(fmt_u_n(t,	_nddal_stats.nfree, 8, ' '));
    nddal_print("\nnddal stats: still in use: ");
	nddal_print(fmt_u_n(t, _nddal_stats.nmalloc - _nddal_stats.nfree, 8, ' '));
	nddal_print("\n");
}   

/* Checking functions: */
static void
nddal_check_block(nddal_block_head_t *hb) {
    int i, dump = 0;
    nddal_block_tail_t *ht;
	char t[500];	

    ht = (nddal_block_tail_t *) ((char *) hb + sizeof(nddal_block_head_t) + hb->size);

    if (strcmp(hb->magic_head, nddal_magic_head)) dump += 1;
    if (strcmp((char *) ht, nddal_magic_tail))    dump += 2;

    if (dump) {
		nddal_print("trying to free corrupted block at ");
		nddal_print(fmt_ptr(t, hb));
		nddal_print(":\n");

        if (dump & 1) {
			nddal_print("   magic header corrupted (\"");
			nddal_print(hb->magic_head);
			nddal_print("\"\n");
		}
	
        if (dump & 2) {
			nddal_print("   magic tailer corrupted (\"");
			nddal_print((char *) hb + sizeof(nddal_block_head_t) + hb->size);
			nddal_print("\"\n");
		}

        if (hb->status < nddal_Unset || hb->status > nddal_Free) {
			nddal_print("   block status: ");
			nddal_print(fmt_u(t, hb->status));
			nddal_print("(invalid)\n");
		} else {
			nddal_print("   block status: ");
			nddal_print(nddal_status_str[hb->status]);
			nddal_print("\n");
		}
		nddal_print("   block size  : ");
		nddal_print(fmt_u(t, hb->size));

		nddal_print("\n   block next  : ");
		nddal_print(fmt_ptr(t, hb->next));
		
		nddal_print("\n   user ptr    : ");
		nddal_print(fmt_ptr(t, (char *) hb + sizeof(nddal_block_head_t)));
		nddal_print("\n");

        _nddal_dump_data((char *) hb + sizeof(nddal_block_head_t), hb->size);

        if (_nddal_stack) { 
			nddal_print("   allocation time stack dump:\n");
			nddal_priv_read();       
			stack_dump(hb->nstacks, hb->pstack);
			nddal_priv_lock();
		}
    }
}

static void 
_nddal_block_dump(nddal_block_head_t *hb) {
	nddal_block_tail_t *ht;
	char t[500];	

    ht = (nddal_block_tail_t *) ((char *) hb + sizeof(nddal_block_head_t) + hb->size);

	if (strcmp(hb->magic_head, nddal_magic_head)) {
		nddal_print("   magic header corrupted : ");
		_nddal_dump_data(hb->magic_head, nddal_HEAD_LEN);
		nddal_print("\n");
	}

	if (strcmp((char *) ht, nddal_magic_tail)) {
		nddal_print("   magic tailer corrupted : ");
		_nddal_dump_data((char *) ht, nddal_TAIL_LEN);
		nddal_print("\n");
	}

	nddal_print("   block address: ");
	nddal_print(fmt_ptr(t, hb));
	nddal_print("\n");
	
	nddal_print("   block status : ");
	nddal_print(nddal_status_str[hb->status]);
	nddal_print("\n");


	nddal_print("   block size   : ");
	nddal_print(fmt_u(t, hb->size));
	nddal_print("\n");
	

	nddal_print("   block next   : ");
	nddal_print(fmt_ptr(t, hb->next));
	nddal_print("\n");

	nddal_print("   user ptr     : ");
	nddal_print(fmt_ptr(t, (char *) hb + sizeof(nddal_block_head_t)));
	nddal_print("\n");

	_nddal_dump_data((char *) hb + sizeof(nddal_block_head_t), hb->size);
}

static void 
_nddal_check(int n, nddal_block_head_t *hb) {
    int i, dump = 0;
    nddal_block_tail_t *ht;
	char t[500];

    ht = (nddal_block_tail_t *) ((char *) hb + sizeof(nddal_block_head_t) + hb->size);

	if (strcmp(hb->magic_head, nddal_magic_head)) 
		dump += 1;

    if (strcmp((char *) ht, nddal_magic_tail)) 
        dump += 2;

    if (dump || _nddal_dump) {
		nddal_print("\nBlock number ");
		nddal_print(fmt_u_n(t, n, 4, ' '));
		nddal_print(":\n");

        nddal_print("------------------\n");

		_nddal_block_dump(hb);

		nddal_print("   allocation time stack dump:\n");
		nddal_priv_read();       
		stack_dump(hb->nstacks, hb->pstack);
		nddal_priv_lock();

    	if (strcmp(hb->magic_head, nddal_magic_head)) {
			if (hb->prev) {
				nddal_print("\nPrevious block: \n");
				_nddal_block_dump(hb->prev);
				nddal_priv_read();       
				stack_dump(hb->prev->nstacks, hb->prev->pstack);
				nddal_priv_lock();
			} else nddal_print("\n***No track of previous block ***\n");
		}

		if (strcmp((char *) ht, nddal_magic_tail)) {
			if (hb->next) {
				nddal_print("\nNext block: \n");
				_nddal_block_dump(hb->next);
				nddal_priv_read();       
				stack_dump(hb->next->nstacks, hb->next->pstack);
				nddal_priv_lock();
			} else nddal_print("\n***No track of next block ***\n");
		}
    }
}

static void
nddal_check_all() {
    static int count = 0;
    int i = 0;
    nddal_block_head_t *hb;
	char t[500];
    
    nddal_print("check memory blocks (");
	nddal_print(fmt_u(t, ++count));
	nddal_print(")\n");

    hb = (nddal_block_head_t *) _nddal_base;
    do {
        _nddal_check(i, hb);
        if (hb->next && (long) hb->next < (long) hb) {
            nddal_print("block # ");
			nddal_print(fmt_u(t, i));
			nddal_print(" is corrupted, stopping check\n");
		    if (_nddal_stack) { 
				nddal_print("Here is the stack :\n");
				stack_live_dump();
			}
            break;
        }
        hb = hb->next;
        i++;
    } while (hb);
} 

static void
_nddal_check_all() {
    int i = 0;
    nddal_block_head_t *hb = (nddal_block_head_t *) _nddal_base;

    nddal_print("\n");
    nddal_print("--------------------\n");
    nddal_print(" -- atexit check -- \n");
    nddal_print("--------------------\n");
    nddal_print("\n");

    for (i = 0, hb = (nddal_block_head_t *) _nddal_base; hb; i++, hb = hb->next)
        _nddal_check(i, hb);
    _nddal_stats_print();
} 

/* lowest level allocator: */
void * 
_nddal_morecore(size_t size) {
    return _nddal_sbrk(size);        
}

/* redirected allocation fonctions: */
int _brk(void *foo) {
    char t[500];

	nddal_print("**** nddal ALERT: _brk is directly called from program\n");

    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        nddal_print("_brk(");
		nddal_print(fmt_ptr(t, foo));
		nddal_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return 0;
}

int brk(void *foo) {
    char t[500];
    nddal_print("**** nddal ALERT: brk is directly called from program\n");
    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        nddal_print("brk(");
		nddal_print(fmt_ptr(t, foo));
		nddal_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return 0;
}

void *_sbkr(int size) {
    char t[500];
    nddal_print("nddal ALERT: _sbrk is directly called from program\n");
    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        nddal_print("_sbrk(");
		nddal_print(fmt_u(t, size));
		nddal_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return _malloc(size);
}

void *sbrk(intptr_t size) {
    char t[500];
    nddal_print("nddal ALERT: sbrk is directly called from program\n");
    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        nddal_print("sbrk(");
		nddal_print(fmt_u(t, size));
		nddal_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return _malloc(size);
}

void *
_malloc(size_t size) {
	char t[50];
    void *p;

    if (!_nddal_private) {
        nddal_print("init called from malloc()\n");
        _nddal_init();
    }
    if (_nddal_dbg) {
        nddal_print("malloc(");
		nddal_print(fmt_u(t, size));
        if (_nddal_stack) {
			nddal_print(") called from:\n");
			stack_live_dump();
		} else {
			nddal_print(")\n");
		}
    }
    if (size == 0) nddal_print("malloc(0)\n");

    nddal_lock();
    p = _nddal_malloc(size); 
    nddal_unlock();
    _nddal_stats.nmalloc++;

    if (!p) {
		nddal_print("malloc(");
		nddal_print(fmt_u(t, size));
		nddal_print(") FAILED\n");
        if (_nddal_stack) stack_live_dump();      
    }


    if (p && _nddal_dbg) {
		nddal_print("allocated ");
		nddal_print(fmt_u(t, size));
		nddal_print(" bytes at ");
		nddal_print(fmt_ptr(t, p));
		nddal_print("\n");
	}
    return p;       
}

void _free(void *ptr) {
	char t[50];

    if (!_nddal_private) return;
    if (_nddal_dbg) {
        nddal_print("free(");
		nddal_print(fmt_ptr(t, ptr));
		if (_nddal_stack) {
			nddal_print(") called from:\n");
			stack_live_dump();
		} else  {
			nddal_print(")\n");
		}
    }
    if (!ptr) {
		if (!_nddal_no_free_null) {
			nddal_print("free(NULL):\n");
			if (_nddal_stack) stack_live_dump();
		}
        return;
    } 
    nddal_lock();
    _nddal_free(ptr);
    nddal_unlock();

    _nddal_stats.nfree++;

    if (_nddal_dbg) {
		nddal_print("freed ");
		nddal_print(fmt_ptr(t, ptr));	
		nddal_print("\n");
	}
}

void *
_realloc(void *ptr, size_t newsize) {
    void *q;
    size_t size;
	char t[50];
		
    if (!_nddal_private) _nddal_init();
    if (_nddal_dbg) {
		char *p;
		nddal_block_head_t *hb;
		
		if (ptr) {
			nddal_print("realloc(");
			nddal_print(fmt_ptr(t, ptr));
			nddal_print(", ");
		} else nddal_print("realloc(0, ");

		nddal_print(fmt_u(t, newsize));

		if (_nddal_stack) {
			nddal_print(") called from:\n");
			stack_live_dump();
		} else {
			nddal_print(")\n");
		}
		if (ptr) {
			p = (char *) ptr;
			hb = (nddal_block_head_t *) (p - sizeof(nddal_block_head_t));
			nddal_check_block(hb);
		}
    }
    if (!ptr) {
        if (!_nddal_realloc_as_malloc) {
            nddal_print("realloc used as malloc (null input pointer)\n");
            if (_nddal_stack) stack_live_dump();
        } 
        return malloc(newsize);
    }

    if (_nddal_always_check) nddal_check_all();

    nddal_lock();
    size = (((nddal_block_head_t *) ptr) - 1)->size;
    _nddal_free(ptr);
	q = _nddal_malloc(newsize);
	nddal_unlock();

    if (q) {
        memcpy(q, ptr, newsize < size ? newsize : size);

        if (newsize < size && !_nddal_no_realloc_truncate) {
            nddal_print("realloc truncate current data\n");
            if (_nddal_stack) stack_live_dump();
        }
    } else {
		nddal_print("realloc("); 
        nddal_print(fmt_ptr(t, ptr));
		nddal_print(", ");
        nddal_print(fmt_u(t, newsize));
		nddal_print(") (was ");
        nddal_print(fmt_u(t, size));
		nddal_print(") FAILED\n");
        if (_nddal_stack) stack_live_dump();
    }
    _nddal_stats.nrealloc++;

	if (_nddal_dbg) {
		nddal_print("reallocated ");
		nddal_print(fmt_u(t, newsize));
		nddal_print(" (from ");
		nddal_print(fmt_u(t, size));
		nddal_print(") at ");
		nddal_print(fmt_ptr(t, ptr));
		nddal_print(" (from ");
		nddal_print(fmt_ptr(t, ptr));
		nddal_print(")\n");
	}

    return q;
}

/* Redefine malloc, realloc and free : */
void *malloc(size_t size) { return _malloc(size); }
void *realloc(void *ptr, size_t newsize) { return _realloc(ptr, newsize); }
void free(void *ptr) { _free(ptr); }
