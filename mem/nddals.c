#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include <tbx/fmt.h>

#define nddals_HEAD_LEN 32
#define nddals_TAIL_LEN 32
#define nddals_MAXSTACKS (1024 * 1024 * sizeof(void *))

#define ALIGN 8 
#define nddals_BUFF 1000
#define nddals_ERR  2
#define nddals_OUT  1

enum {nddals_Unset, nddals_Used, nddals_Free};
static char *nddals_status_str[] = { "Unset", "InUse", "Freed" };

typedef struct {
    unsigned long nmalloc;
    unsigned long nrealloc;
    unsigned long nfree;
} stat_t, *pstat_t;

static stat_t _nddals_stats;

typedef struct {
    size_t  size;
    size_t  status;
    void   *next;
#if 0
    int     nstacks;
    void   *pstack;
#endif
    char    magic_head[nddals_HEAD_LEN];
    char    align64[12];
} nddals_block_head_t;

typedef struct {
    char    magic_tail[nddals_TAIL_LEN];
    char    align64[nddals_TAIL_LEN];
} nddals_block_tail_t;

static void nddals_print(char *s);
static void nddals_check_block(nddals_block_head_t *hb);
static void _nddals_check_all();

void * _nddals_morecore(size_t size);
void *_malloc(size_t size);

typedef void *(*fcore)(size_t size);
static fcore _nddals_sbrk              = NULL;

static size_t  _nddals_private_len     = 0;
static void   *_nddals_private         = NULL;
static void   *_nddals_base            = NULL;
static void   *_nddals_curr            = NULL;

static char  _nddals_stack             = 0;
static char  _nddals_dbg               = 0;
static char  _nddals_dump              = 0;
static char  _nddals_realloc_as_malloc = 0;
static char  _nddals_always_check      = 0;
static char  _nddals_no_free_null      = 0;

static int _nddals_lock                = 0;

static char nddals_magic_head[] = "#MeM cHeCkEr: HeAdErMaGicStRiNg";
static char nddals_magic_tail[] = "*mEm ChEcKeR: tAiLeRmAgICsTrInG";

/* nddals locking: */
static void nddals_priv_lock()  { 
    if (mprotect(_nddals_private, _nddals_private_len, PROT_NONE))
        nddals_print("mprotect lock FAILED\n");
}

static void nddals_priv_read()  { 
    if (mprotect(_nddals_private, _nddals_private_len, PROT_READ))
        nddals_print("mprotect read unlock FAILED\n");
}

#if 0
static void nddals_priv_write() { 
    if (mprotect(_nddals_private, _nddals_private_len, PROT_READ | PROT_WRITE))
        nddals_print("mprotect write unlock FAILED\n");
}
#endif

static void nddals_lock()   {
    while (_nddals_lock);
    _nddals_lock = 1;
}

static void nddals_unlock() {
    _nddals_lock = 0; 
}

/* Signal handling: */
void
_nddals_signals(int sig) {
	char t[10];
    nddals_print("nddals caught signal ");
	nddals_print(fmt_u(t, sig));
	if (_nddals_stack) {
		nddals_print("\nHere is the callstack when signal has been emitted :\n");
		//stack_live_dump();
	}
    nddals_print("\nNow preform memory check...\n");
    _nddals_check_all();
    nddals_print("done\n");
}

/* alignment: */
static size_t 
nddals_aligned(size_t size) {
    unsigned int residue;
    if ((residue = size % ALIGN))
        size += (ALIGN - residue);
    return size;
}

void _nddals_init() {
	char b[1024];
    long pagesize;
    pagesize = sysconf(_SC_PAGE_SIZE);

	/*nddals_print("_nddals_init\n"); */

    /* fetch real "mem core function" */
    _nddals_sbrk = (fcore) dlsym(RTLD_NEXT, "sbrk");

    /* make sure we start on pagesize multiple */
    if (((size_t) _nddals_sbrk(0)) % pagesize) _nddals_morecore(pagesize - ((size_t) _nddals_sbrk(0) % pagesize));

    _nddals_private = _nddals_sbrk(0);

	if (getenv("NDDAL_DBG"))               _nddals_dbg               = 1;
	if (getenv("NDDAL_DUMP"))              _nddals_dump              = 1;
//	if (getenv("NDDAL_STACK"))             _nddals_stack             = 1;
	if (getenv("NDDAL_REALLOC_AS_MALLOC")) _nddals_realloc_as_malloc = 1;
	if (getenv("NDDAL_ALWAYS_CHECK"))      _nddals_always_check      = 1;
	if (getenv("NDDAL_NO_FREE_NULL"))      _nddals_no_free_null      = 1;

	if (_nddals_dbg) {
		nddals_print("_nddals_private = ");
		fmt_ptr(b, _nddals_private);
		nddals_print(b);
		nddals_print("\n");
	}

    //stack_init(nddals_MAXSTACKS);
    signal(SIGSEGV, _nddals_signals);

    if (((size_t) _nddals_sbrk(0)) % pagesize) _nddals_morecore(pagesize - ((size_t) _nddals_sbrk(0) % pagesize));

    _nddals_base = _nddals_sbrk(0);

	if (_nddals_dbg) {
		nddals_print("_nddals_base = ");
		fmt_ptr(b, _nddals_base);
		nddals_print(b);
		nddals_print("\n");
	}

    _nddals_private_len = (char *) _nddals_base - (char *) _nddals_private - 1;
    _nddals_stats.nmalloc  = 
    _nddals_stats.nrealloc = 
    _nddals_stats.nfree    = 0;

    nddals_priv_lock();       
    
    atexit(_nddals_check_all);
}

#ifdef SOLARIS
#pragma init (_nddals_init)
#endif
#ifdef LINUX
void _nddals_init() __attribute__((constructor));
#endif

static void *_nddals_malloc(size_t size) {
    static int        count = 0;
    size_t            gsize, real;
    void              *p; 
    char              *q;
    nddals_block_head_t *h;

    /* 
     * compute actual reservation size as : 
     * header + tailer + size (adjusted)
     */
    
    h = _nddals_curr;

    gsize = nddals_aligned(size);
    real = gsize + sizeof(nddals_block_head_t) + sizeof(nddals_block_tail_t);

    if ((p = _nddals_morecore(real)) == (void *) -1) return NULL;
    if ((unsigned long) p % ALIGN) { 
		char t[50];
        nddals_print("ALIGNMENT ERROR: ");
		nddals_print(fmt_ptr(t, p));
		nddals_print(" % ");
		nddals_print(fmt_u(t, ALIGN));
		nddals_print(" = ");
		nddals_print(fmt_u(t, ((unsigned long) p) % ALIGN));
		nddals_print("\n");
		
	}

    if (count) h->next = p;
    
    h = (nddals_block_head_t *) (q = (char *) p);

    h->size   = gsize;
    h->status = nddals_Used;
    h->next   = NULL;
    strcpy(h->magic_head, nddals_magic_head);

	/* Fill the allocated but not used bytes withs 'X' to check small leaks: */
    if (size < gsize) memset(q +  sizeof(nddals_block_head_t) + size, 'X', gsize - size);
    strcpy(q + sizeof(nddals_block_head_t) + gsize, nddals_magic_tail);

    /* Find out a way to protect small amounts of mem HERE!!! */

#if 0
    nddals_priv_write();
    h->pstack  = stack_curr_get();
  	h->nstacks = stack_trace();
    nddals_priv_lock();
#endif

    _nddals_curr = (void *) h;
    count++;

    if (_nddals_dbg) {
		char t[50];
		nddals_print("new block @ 0x");
		nddals_print(fmt_ptr(t,  h));
		nddals_print("\n");
	}
    return q + sizeof(nddals_block_head_t);
}

static void _nddals_free(void *ptr) {
    char *p;
    nddals_block_head_t *hb;

    p = (char *) ptr;
    hb = (nddals_block_head_t *) (p - sizeof(nddals_block_head_t));

    nddals_check_block(hb);
    hb->status = nddals_Free;
}

/* writing and dumping: */
static void 
nddals_print(char *s) {
	if (s) write(2, s, strlen(s));
}

static void _nddals_dump_data(char *buff, size_t size) {
    int i, j;
    char hex[256], asc[256], tmp[512], final[512];
	char *h, *a;

    if (buff == NULL) {
       	nddals_print("   null pointer\n");
        return;
    }

    nddals_print("   block dump  :\n");
	
    /* 00 11 22 33 44 55 66 77 - 88 99 AA BB CC DD EE FF   azertyui - qsdfghjk */
    for (i = 0; i < size; i += 16) {
        *hex = *asc = *final = 0;
		h = hex, a = asc;
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
		nddals_print("      ");	
		nddals_print(fmt_ptr(tmp, buff + i)); 
		nddals_print(": ");
		nddals_print(hex);
		nddals_print("  ");
		nddals_print(asc);
        nddals_print("\n");
    }
}

#if 0
static void
_nddals_hdr_dump(nddals_block_head_t *h) {
	char t[500];

    if (!h) return;
    nddals_print("haddr  = ");
	nddals_print(fmt_ptr(t, h));
	nddals_print("\n");
	nddals_print("size   = ");
	nddals_print(fmt_u(t, h->size));
	nddals_print("\n");

    if (h->status < nddals_Unset || h->status > nddals_Free) {
         nddals_print("invalid status\n");
    } else { 
		nddals_print("status = ");
		nddals_print(nddals_status_str[h->status]);
		nddals_print("\n");
	}
    nddals_print("next   = ");
	nddals_print(fmt_ptr(t, h->next));
	nddals_print("\n");
	nddals_print("magic1 = \"");
	nddals_print(h->magic_head);
	nddals_print("\n");
	nddals_print("magic2 = \"");
	nddals_print((char *) h + sizeof(nddals_block_head_t) + h->size);
	nddals_print("\n");
    nddals_print("---------------------------\n\n");
}
#endif

static void _nddals_stats_print() {
	char t[50];

    nddals_print("nddals stats:\n");
    nddals_print("nddals stats: memory statistics:\n");
    nddals_print("nddals stats: ------------------\n");
    nddals_print("nddals stats: malloc:       ");
	nddals_print(fmt_u_n(t,	_nddals_stats.nmalloc, 8, ' '));
    nddals_print("\nnddals stats: realloc:      ");
	nddals_print(fmt_u_n(t,	_nddals_stats.nrealloc, 8, ' '));
    nddals_print("\nnddals stats: free:         ");
	nddals_print(fmt_u_n(t,	_nddals_stats.nfree, 8, ' '));
    nddals_print("\nnddals stats: still in use: ");
	nddals_print(fmt_u_n(t, _nddals_stats.nmalloc - _nddals_stats.nfree, 8, ' '));
	nddals_print("\n");
}   

/* Checking functions: */

static void
nddals_check_block(nddals_block_head_t *hb) {
    int dump = 0;
    nddals_block_tail_t *ht;
	char t[500];	

    ht = (nddals_block_tail_t *) ((char *) hb + sizeof(nddals_block_head_t) + hb->size);

    if (strcmp(hb->magic_head, nddals_magic_head)) dump += 1;
    if (strcmp((char *) ht, nddals_magic_tail))    dump += 2;

    if (dump) {
		nddals_print("trying to free corrupted block at : ");
		nddals_print(fmt_ptr(t, hb));
		nddals_print(":\n");

        if (dump & 1) {
			nddals_print("   magic header corrupted (\"");
			nddals_print(hb->magic_head);
			nddals_print("\"\n");
		}
	
        if (dump & 2) {
			nddals_print("   magic tailer corrupted (\"");
			nddals_print((char *) hb + sizeof(nddals_block_head_t) + hb->size);
			nddals_print("\"\n");
		}

        if (hb->status < nddals_Unset || hb->status > nddals_Free) {
			nddals_print("   block status: ");
			nddals_print(fmt_u(t, hb->status));
			nddals_print("(invalid)\n");
		} else {
			nddals_print("   block status: ");
			nddals_print(nddals_status_str[hb->status]);
			nddals_print("\n");
		}
		nddals_print("   block size  : ");
		nddals_print(fmt_u(t, hb->size));

		nddals_print("\n   block next  : ");
		nddals_print(fmt_ptr(t, hb->next));
		
		nddals_print("\n   user ptr    : ");
		nddals_print(fmt_ptr(t, (char *) hb + sizeof(nddals_block_head_t)));
		nddals_print("\n");

        _nddals_dump_data((char *) hb + sizeof(nddals_block_head_t), hb->size);

        if (_nddals_stack) { 
			nddals_print("   allocation time stack dump:\n");
			nddals_priv_read();       
			//stack_dump(hb->nstacks, hb->pstack);
			nddals_priv_lock();
		}
    }
}

static void 
_nddals_check(int n, nddals_block_head_t *hb) {
    int dump = 0;
    nddals_block_tail_t *ht;
	char t[500];

    ht = (nddals_block_tail_t *) ((char *) hb + sizeof(nddals_block_head_t) + hb->size);

    if (strcmp(hb->magic_head, nddals_magic_head)) 
        dump += 1;
    if (strcmp((char *) ht, nddals_magic_tail)) 
        dump += 2;

    if (dump || _nddals_dump) {

		nddals_print("\nBlock number ");
		nddals_print(fmt_u_n(t, n, 4, ' '));
		nddals_print(":\n");

        nddals_print("------------------\n");

        if   (dump & 1) nddals_print("   magic header corrupted\n");
        if   (dump & 2) nddals_print("   magic tailer corrupted\n");

		nddals_print("   block address: ");
		nddals_print(fmt_ptr(t, hb));
		nddals_print("\n");
		
        nddals_print("   block status : ");
		nddals_print(nddals_status_str[hb->status]);
		nddals_print("\n");


        nddals_print("   block size   : ");
		nddals_print(fmt_u(t, hb->size));
		nddals_print("\n");
		

        nddals_print("   block next   : ");
		nddals_print(fmt_ptr(t, hb->next));
		nddals_print("\n");

        nddals_print("   user ptr     : ");
		nddals_print(fmt_ptr(t, (char *) hb + sizeof(nddals_block_head_t)));
		nddals_print("\n");

        _nddals_dump_data((char *) hb + sizeof(nddals_block_head_t), hb->size);
        
     	if (_nddals_stack) { 
			nddals_print("   allocation time stack dump:\n");
			nddals_priv_read();       
			//stack_dump(hb->nstacks, hb->pstack);
			nddals_priv_lock();
		}
    }
}

static void
nddals_check_all() {
    static int count = 0;
    int i = 0;
    nddals_block_head_t *hb;
	char t[500];
    
    nddals_print("check memory blocks (");
	nddals_print(fmt_u(t, ++count));
	nddals_print(")\n");

    hb = (nddals_block_head_t *) _nddals_base;
    do {
        _nddals_check(i, hb);
        if (hb->next && (long) hb->next < (long) hb) {
            nddals_print("block # ");
			nddals_print(fmt_u(t, i));
			nddals_print(" is corrupted, stopping check\n");
		    if (_nddals_stack) { 
				nddals_print("Here is the stack :\n");
				// stack_live_dump();
			}
            break;
        }
        hb = hb->next;
        i++;
    } while (hb);
} 

static void
_nddals_check_all() {
    int i = 0;
    nddals_block_head_t *hb = (nddals_block_head_t *) _nddals_base;

    nddals_print("\n");
    nddals_print("--------------------\n");
    nddals_print(" -- atexit check -- \n");
    nddals_print("--------------------\n");
    nddals_print("\n");

    for (i = 0, hb = (nddals_block_head_t *) _nddals_base; hb; i++, hb = hb->next)
        _nddals_check(i, hb);
    _nddals_stats_print();
} 

/* lowest level allocator: */
void * 
_nddals_morecore(size_t size) {
    return _nddals_sbrk(size);        
}

/* redirected allocation fonctions: */
int _brk(void *foo) {
    char t[500];

	nddals_print("**** nddals ALERT: _brk is directly called from program\n");

    if (!_nddals_private) _nddals_init();
    if (_nddals_dump) {
        nddals_print("_brk(");
		nddals_print(fmt_ptr(t, foo));
		nddals_print(")\n");
        // if (_nddals_stack) stack_live_dump();
    }
    return 0;
}

int brk(void *foo) {
    char t[500];
    nddals_print("**** nddals ALERT: brk is directly called from program\n");
    if (!_nddals_private) _nddals_init();
    if (_nddals_dump) {
        nddals_print("brk(");
		nddals_print(fmt_ptr(t, foo));
		nddals_print(")\n");
        // if (_nddals_stack) stack_live_dump();
    }
    return 0;
}

void *_sbkr(int size) {
    char t[500];
    nddals_print("nddals ALERT: _sbrk is directly called from program\n");
    if (!_nddals_private) _nddals_init();
    if (_nddals_dump) {
        nddals_print("_sbrk(");
		nddals_print(fmt_u(t, size));
		nddals_print(")\n");
        // if (_nddals_stack) stack_live_dump();
    }
    return _malloc(size);
}

void *sbrk(intptr_t size) {
    char t[500];
    nddals_print("nddals ALERT: sbrk is directly called from program\n");
    if (!_nddals_private) _nddals_init();
    if (_nddals_dump) {
        nddals_print("sbrk(");
		nddals_print(fmt_u(t, size));
		nddals_print(")\n");
        // if (_nddals_stack) stack_live_dump();
    }
    return _malloc(size);
}

void *_malloc(size_t size) {
	char t[50];
    void *p;
    if (!_nddals_private) {
        nddals_print("init called from malloc()\n");
        _nddals_init();
    }
    if (_nddals_dbg) {
        nddals_print("malloc(");
		nddals_print(fmt_u(t, size));
		nddals_print(") called from:\n");
        // if (_nddals_stack) stack_live_dump();
    }
    if (size == 0) nddals_print("malloc(0)\n");
    nddals_lock();
    p = _nddals_malloc(size); 
    if (!p) {
		nddals_print("malloc(");
		nddals_print(fmt_u(t, size));
		nddals_print(") FAILED\n");
        // if (_nddals_stack) stack_live_dump();      
    }
    _nddals_stats.nmalloc++;
    nddals_unlock();
    if (p && _nddals_dbg) {
		nddals_print("allocated ");
		nddals_print(fmt_u(t, size));
		nddals_print("bytes at ");
		nddals_print(fmt_ptr(t, p));
		nddals_print("\n");
	}
    return p;       
}

void _free(void *ptr) {
	char t[50];

    if (!_nddals_private) return;
    if (_nddals_dbg) {
        nddals_print("free(");
		nddals_print(fmt_ptr(t, ptr));
		nddals_print(") called from:\n");
        // if (_nddals_stack) stack_live_dump();
    }
    if (!ptr) {
        nddals_print("free(NULL):\n");
        // if (_nddals_stack) stack_live_dump();
        return;
    } 
    nddals_lock();
    _nddals_free(ptr);
    _nddals_stats.nfree++;
    nddals_unlock();
    if (_nddals_dbg) {
		nddals_print("freed ");
		nddals_print(fmt_ptr(t, ptr));	
		nddals_print("\n");
	}
}

void *
_realloc(void *ptr, size_t newsize) {
    void *q;
    size_t size;
	char t[50];
	char *p;
	nddals_block_head_t *hb;
		
    if (!_nddals_private) _nddals_init();
    if (_nddals_dbg) {
		
		if (ptr) {
			nddals_print("realloc(");
			nddals_print(fmt_ptr(t, ptr));
			nddals_print(", ");
		} else nddals_print("realloc(0, ");

		nddals_print(fmt_u(t, newsize));

		if (_nddals_stack) {
			nddals_print(") called from:\n");
			// stack_live_dump();
		}
    }
	if (ptr) {
			p = (char *) ptr;
			hb = (nddals_block_head_t *) (p - sizeof(nddals_block_head_t));
			nddals_check_block(hb);
	}
    if (!ptr) {
        if (_nddals_realloc_as_malloc) {
            nddals_print("realloc used as malloc (null input pointer)\n");
            // if (_nddals_stack) stack_live_dump();
        }
        return _malloc(newsize);
    }

    nddals_lock();

    size = (((nddals_block_head_t *) ptr) - 1)->size;

    if (_nddals_always_check) nddals_check_all();
    
    _nddals_free(ptr);
    if ((q = _nddals_malloc(newsize))) {
        memcpy(q, ptr, newsize < size ? newsize : size);
        if (newsize < size) {
            nddals_print("realloc truncate current data:\n");
            // if (_nddals_stack) stack_live_dump();
        }
    } else {
		nddals_print("realloc("); 
        nddals_print(fmt_ptr(t, ptr));
		nddals_print(", ");
        nddals_print(fmt_u(t, newsize));
		nddals_print(") (was ");
        nddals_print(fmt_u(t, size));
		nddals_print(") FAILED\n");
        // if (_nddals_stack) stack_live_dump();
    }
    _nddals_stats.nrealloc++;
    nddals_unlock();
    nddals_print("reallocated ");
	nddals_print(fmt_u(t, newsize));
	nddals_print(" (from ");
	nddals_print(fmt_u(t, size));
	nddals_print(") at ");
	nddals_print(fmt_ptr(t, ptr));
	nddals_print(" (from ");
	nddals_print(fmt_ptr(t, ptr));
	nddals_print(")\n");
    return q;
}

/* Redefine malloc, realloc and free : */
void *malloc(size_t size) { return _malloc(size); }
void *realloc(void *ptr, size_t newsize) { return _realloc(ptr, newsize); }
void free(void *ptr) { _free(ptr); }
