#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <dlfcn.h>

#include <errno.h>

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
//	void *  pad;
    int     nstacks;
    void   *pstack;
    char    magic_head[nddal_HEAD_LEN];
//   char    align64[12];
} nddal_block_head_t;

typedef struct {
    char    magic_tail[nddal_TAIL_LEN];
    char    align64[nddal_TAIL_LEN];
} nddal_block_tail_t;

static void nddal_check_block(nddal_block_head_t *hb);
static void nddal_check_all();
static void _nddal_check_all();
static void _nddal_check_atexit();

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
static char  _nddal_keep_freed_data		= 0;
static char  _nddal_protect             = 0;

static int   _nddal_lock                = 0;

static char nddal_magic_head[] = "#MeM cHeCkEr: HeAdErMaGicStRiNg";
static char nddal_magic_tail[] = "*mEm ChEcKeR: tAiLeRmAgICsTrInG";

static char *
nddal_strncpy(char *d, char *s, unsigned int n) {
	char *p;
	if (n <= 0) return NULL;
	p = d;
	while (n-- && (*p++ = *s++));
	*p = 0;	
	return d;
}

static int
nddal_strncmp(char *a, char *b, unsigned int n) {
	if (n <= 0) return 0;
	while (--n && *a++ == *b++);
	if (n) return *a - *b;
	return 0;
}

/* nddal locking: */
static void 
nddal_priv_lock()  { 
	if (!_nddal_protect) return;
    if (mprotect(_nddal_private, _nddal_private_len, PROT_NONE))
        fmt_print("mprotect lock FAILED\n");
}

static void 
nddal_priv_read()  { 
	if (!_nddal_protect) return;
    if (mprotect(_nddal_private, _nddal_private_len, PROT_READ))
        fmt_print("mprotect read unlock FAILED\n");
}

static void 
nddal_priv_write() { 
	if (!_nddal_protect) return;
    if (mprotect(_nddal_private, _nddal_private_len, PROT_READ | PROT_WRITE))
        fmt_print("mprotect write unlock FAILED\n");
}

static void 
nddal_lock()   {
    while (_nddal_lock);
    _nddal_lock = 1;
}

static void 
nddal_unlock() {
    _nddal_lock = 0; 
}

/* Signal handling: */
void
_nddal_signals(int sig) {
	char t[10];
    fmt_print("nddal caught signal ");
	fmt_print(fmt_u(t, sig));
	if (_nddal_stack) {
		fmt_print("\nHere is the callstack when signal has been emitted :\n");
		stack_live_dump();
	}
    fmt_print("\nNow preform memory check...\n");
    _nddal_check_all();
    fmt_print("done\n");
}

void
nddal_sigusr(int sig) {
	fmt_print("\n*** SIGUSR memory check\n");
	_nddal_check_all();
	signal(sig, nddal_sigusr);
}

/* alignment: */
static size_t 
nddal_aligned(size_t size) {
    unsigned int residue;
    if ((residue = size % ALIGN))
        size += (ALIGN - residue);
    return size;
}

void 
_nddal_init() {
	char b[1024];
	size_t stackbufsize;
    long pagesize;
    pagesize = sysconf(_SC_PAGE_SIZE);
	void  *_nddal_stack_from = 0;

    /* fetch real "mem core function" */
    _nddal_sbrk = (fcore) dlsym(RTLD_NEXT, "sbrk");

    /* make sure we start on pagesize multiple */
    if (((size_t) _nddal_sbrk(0)) % pagesize) _nddal_morecore(pagesize - ((size_t) _nddal_sbrk(0) % pagesize));

    _nddal_private = _nddal_sbrk(0);

	if (getenv("NDDAL_PROTECT")) 			 _nddal_protect             = 1;
	if (getenv("NDDAL_DBG"))                 _nddal_dbg                 = 1;
	if (getenv("NDDAL_DUMP"))                _nddal_dump                = 1;
	if (getenv("NDDAL_STACK"))               _nddal_stack               = 1;
	if (getenv("NDDAL_REALLOC_AS_MALLOC"))   _nddal_realloc_as_malloc   = 1;
	if (getenv("NDDAL_ALWAYS_CHECK"))        _nddal_always_check        = 1;
	if (getenv("NDDAL_SIGHANDLE"))			 _nddal_sighandle           = 1;
	if (getenv("NDDAL_NO_FREE_NULL"))        _nddal_no_free_null        = 1;
	if (getenv("NDDAL_NO_REALLOC_TRUNCATE")) _nddal_no_realloc_truncate = 1;
	if (getenv("NDDAL_KEEP_FREED_DATA"))     _nddal_keep_freed_data     = 1;

	if (getenv("NDDAL_FILE")) 
		fmt_fd_set(open(getenv("NDDAL_FILE"), O_WRONLY | O_CREAT | O_TRUNC, 0644));

	fmt_print("_nddal_private = ");
	fmt_ptr(b, _nddal_private);
	fmt_print(b);
	fmt_print("\n");

	stackbufsize = nddal_STACKBUFFERLEN;
   	if (getenv("NDDAL_STACKBUFFSIZE")) {
		stackbufsize = atol(getenv("NDDAL_STACKBUFFSIZE"));
	}

	_nddal_stack_from = _nddal_sbrk(stackbufsize * sizeof(void *));
	stack_init(_nddal_private, stackbufsize * sizeof(void *));

    /* make sure we start on pagesize multiple */
    if (((size_t) _nddal_sbrk(0)) % pagesize) _nddal_morecore(pagesize - ((size_t) _nddal_sbrk(0) % pagesize));

	if (signal(SIGUSR1, nddal_sigusr)) fmt_print("*** cannot redirect SIGUSR1!\n");
	if (signal(SIGUSR2, nddal_sigusr)) fmt_print("*** cannot redirect SIGUSR2!\n");
 

	if (_nddal_sighandle) signal(SIGSEGV, _nddal_signals);

    _nddal_base = _nddal_sbrk(0);

	fmt_print("_nddal_base = ");
	fmt_ptr(b, _nddal_base);
	fmt_print(b);
	fmt_print("\n");

    _nddal_private_len    = (char *) _nddal_base - (char *) _nddal_private;
    _nddal_stats.nmalloc  = 
    _nddal_stats.nrealloc = 
    _nddal_stats.nfree    = 0;

    nddal_priv_lock();       
    
    atexit(_nddal_check_atexit);
}

#ifdef SOLARIS
#pragma init (_nddal_init)
#endif
#ifdef LINUX
void _nddal_init() __attribute__((constructor));
#endif

static void *
_nddal_malloc(size_t size) {
    static int         count = 0;
    size_t             gsize, real;
    void               *p; 
    char               *q;
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
        fmt_print("ALIGNMENT ERROR: ");
		fmt_print(fmt_ptr(t, p));
		fmt_print(" % ");
		fmt_print(fmt_u(t, ALIGN));
		fmt_print(" = ");
		fmt_print(fmt_u(t, ((unsigned long) p) % ALIGN));
		fmt_print("\n");
	}

    if (count) h->next = p;
    
    h = (nddal_block_head_t *) (q = (char *) p);

    h->size   = size;
    h->status = nddal_Used;
    h->next   = NULL;
	h->prev   = _nddal_curr;

    strcpy(h->magic_head, nddal_magic_head);

	/* Fill the allocated but not used bytes withs 'X' to check small leaks: */
    if (size < gsize) memset(q +  sizeof(nddal_block_head_t) + size, 'X', gsize - size);
    strcpy(q + sizeof(nddal_block_head_t) + gsize, nddal_magic_tail);

    /* Find out a way to protect small amounts of mem HERE!!! */

    nddal_priv_write();
    h->pstack   = stack_curr_get();
    h->nstacks  = stack_trace();
    nddal_priv_lock();

    _nddal_curr = (void *) h;
    count++;

    if (_nddal_dbg) {
		char t[50];
		fmt_print("--> new block @ 0x");
		fmt_print(fmt_ptr(t,  h));
		fmt_print("\n");
	}
    return q + sizeof(nddal_block_head_t);
}

static void 
_nddal_free(void *ptr) {
    char *p;
    nddal_block_head_t *hb;
	if (!ptr) return;

    p = (char *) ptr;
    hb = (nddal_block_head_t *) (p - sizeof(nddal_block_head_t));

    nddal_check_block(hb);
    hb->status = nddal_Free;
}
static void 
nddal_free(void *ptr) {
    char *p;
    nddal_block_head_t *hb;

	if (!ptr) return;
    p = (char *) ptr;

    hb = (nddal_block_head_t *) (p - sizeof(nddal_block_head_t));
	_nddal_free(ptr);

	if (!_nddal_keep_freed_data) 
		memset(p, 0, hb->size); /* XXXXXXX */
}

static void 
_nddal_dump_data(char *buff, size_t size) {
    int i, j;
    char hex[256], asc[256], tmp[512], final[512];
	char *h, *a, *t;

    if (buff == NULL) {
       	fmt_print("   null pointer\n");
        return;
    }

    fmt_print("   block dump  :\n");
	
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
		fmt_print("      ");	
		fmt_print(fmt_ptr(tmp, buff + i)); 
		fmt_print(": ");
		fmt_print(hex);
		fmt_print("  ");
		fmt_print(asc);
        fmt_print("\n");
    }
}

#if 0
static void
_nddal_hdr_dump(nddal_block_head_t *h) {
	char t[500];

    if (!h) return;
    fmt_print("haddr  = ");
	fmt_print(fmt_ptr(t, h));
	fmt_print("\n");
	fmt_print("size   = ");
	fmt_print(fmt_u(t, h->size));
	fmt_print("\n");

    if (h->status < nddal_Unset || h->status > nddal_Free) {
         fmt_print("invalid status\n");
    } else { 
		fmt_print("status = ");
		fmt_print(nddal_status_str[h->status]);
		fmt_print("\n");
	}
    fmt_print("next   = ");
	fmt_print(fmt_ptr(t, h->next));
	fmt_print("\n");
	fmt_print("magic1 = \"");
	fmt_print(h->magic_head);
	fmt_print("\n");
	fmt_print("magic2 = \"");
	fmt_print((char *) h + sizeof(nddal_block_head_t) + nddal_aligned(h->size));
	fmt_print("\n");
    fmt_print("---------------------------\n\n");
}
#endif

static void 
_nddal_stats_print() {
	char t[50];

    fmt_print("nddal stats:\n");
    fmt_print("nddal stats: memory statistics:\n");
    fmt_print("nddal stats: ------------------\n");
    fmt_print("nddal stats: malloc:       ");
	fmt_print(fmt_u_n(t,	_nddal_stats.nmalloc, 8, ' '));
    fmt_print("\nnddal stats: realloc:      ");
	fmt_print(fmt_u_n(t,	_nddal_stats.nrealloc, 8, ' '));
    fmt_print("\nnddal stats: free:         ");
	fmt_print(fmt_u_n(t,	_nddal_stats.nfree, 8, ' '));
    fmt_print("\nnddal stats: still in use: ");
	fmt_print(fmt_u_n(t, _nddal_stats.nmalloc - _nddal_stats.nfree, 8, ' '));
	fmt_print("\n");
}   

/* Checking functions: */
static void
nddal_check_block(nddal_block_head_t *hb) {
    int dump = 0;
    nddal_block_tail_t *ht;
	char t[500];	

    ht = (nddal_block_tail_t *) ((char *) hb + sizeof(nddal_block_head_t) + nddal_aligned(hb->size));
    if (strcmp(hb->magic_head, nddal_magic_head)) dump += 1;
	if (nddal_strncmp((char *) hb + sizeof(nddal_block_head_t) + hb->size, "XXXXXXXX", nddal_aligned(hb->size) - hb->size)) dump += 2;
    if (strcmp((char *) ht, nddal_magic_tail))    dump += 4;
	
    if (dump) {
		fmt_print("trying to free corrupted block at ");
		fmt_print(fmt_ptr(t, hb));
		fmt_print(":\n");

        if (dump & 1) {
			fmt_print("   magic header corrupted (\"");
			fmt_print(hb->magic_head);
			fmt_print("\" ");
			_nddal_dump_data(hb->magic_head, nddal_HEAD_LEN);
			fmt_print("\n");	
		}
	
		if (dump & 2) {
			fmt_print("   tail padding corrupted (\"");
			nddal_strncpy(t, (char *) hb + sizeof(nddal_block_head_t) + hb->size, nddal_aligned(hb->size) - hb->size);
			fmt_print(t);
			fmt_print("\" should be \"");
			nddal_strncpy(t, "XXXXXXXX", nddal_aligned(hb->size) - hb->size);
			fmt_print(t);
			fmt_print("\")\n");
		}

        if (dump & 4) {
			fmt_print("   magic tailer corrupted (\"");
			fmt_print((char *) hb + sizeof(nddal_block_head_t) + nddal_aligned(hb->size));
			fmt_print("\"\n");
		}


        if (hb->status < nddal_Unset || hb->status > nddal_Free) {
			fmt_print("   block status: ");
			fmt_print(fmt_u(t, hb->status));
			fmt_print("(invalid)\n");
		} else {
			fmt_print("   block status: ");
			fmt_print(nddal_status_str[hb->status]);
			fmt_print("\n");
		}
		fmt_print("   block size  : ");
		fmt_print(fmt_u(t, nddal_aligned(hb->size)));

		fmt_print("\n   block next  : ");
		fmt_print(fmt_ptr(t, hb->next));
		
		fmt_print("\n   user ptr    : ");
		fmt_print(fmt_ptr(t, (char *) hb + sizeof(nddal_block_head_t)));
		fmt_print("\n");

        _nddal_dump_data((char *) hb + sizeof(nddal_block_head_t), nddal_aligned(hb->size));

        if (_nddal_stack) { 
			fmt_print("   allocation time stack dump:\n");
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

    ht = (nddal_block_tail_t *) ((char *) hb + sizeof(nddal_block_head_t) + nddal_aligned(hb->size));

	if (strcmp(hb->magic_head, nddal_magic_head)) {
		fmt_print("   magic header corrupted : ");
		_nddal_dump_data(hb->magic_head, nddal_HEAD_LEN);
		fmt_print("\n");
	}

	if (strcmp((char *) ht, nddal_magic_tail)) {
		fmt_print("   magic tailer corrupted : ");
		_nddal_dump_data((char *) ht, nddal_TAIL_LEN);
		fmt_print("\n");
	}

	fmt_print("   block address: ");
	fmt_print(fmt_ptr(t, hb));
	fmt_print("\n");
	
	fmt_print("   block status : ");
	fmt_print(nddal_status_str[hb->status]);
	fmt_print("\n");

	fmt_print("   block size   : ");
	fmt_print(fmt_u(t, nddal_aligned(hb->size)));
	fmt_print("\n");
	

	fmt_print("   block next   : ");
	fmt_print(fmt_ptr(t, hb->next));
	fmt_print("\n");

	fmt_print("   user ptr     : ");
	fmt_print(fmt_ptr(t, (char *) hb + sizeof(nddal_block_head_t)));
	fmt_print("\n");

	_nddal_dump_data((char *) hb + sizeof(nddal_block_head_t), hb->size);
}

static void 
_nddal_check(int n, nddal_block_head_t *hb) {
    int dump = 0;
    nddal_block_tail_t *ht;
	char t[500];

    ht = (nddal_block_tail_t *) ((char *) hb + sizeof(nddal_block_head_t) + nddal_aligned(hb->size));

	if (strcmp(hb->magic_head, nddal_magic_head)) 
		dump += 1;

    if (strcmp((char *) ht, nddal_magic_tail)) 
        dump += 2;

    if (dump || _nddal_dump) {
		fmt_print("\nBlock number ");
		fmt_print(fmt_u_n(t, n, 4, ' '));
		fmt_print(":\n");

        fmt_print("------------------\n");

		_nddal_block_dump(hb);

		fmt_print("   allocation time stack dump:\n");
		nddal_priv_read();       
		stack_dump(hb->nstacks, hb->pstack);
		nddal_priv_lock();

    	if (strcmp(hb->magic_head, nddal_magic_head)) {
			if (hb->prev) {
				fmt_print("\nPrevious block: \n");
				_nddal_block_dump(hb->prev);
				nddal_priv_read();       
				stack_dump(hb->prev->nstacks, hb->prev->pstack);
				nddal_priv_lock();
			} else fmt_print("\n***No track of previous block ***\n");
		}

		if (strcmp((char *) ht, nddal_magic_tail)) {
			if (hb->next) {
				fmt_print("\nNext block: \n");
				_nddal_block_dump(hb->next);
				nddal_priv_read();       
				stack_dump(hb->next->nstacks, hb->next->pstack);
				nddal_priv_lock();
			} else fmt_print("\n***No track of next block ***\n");
		}
    }
}

static void
nddal_check_all() {
    static int count = 0;
    int i = 0;
    nddal_block_head_t *hb;
	char t[500];
    
    fmt_print("check memory blocks (");
	fmt_print(fmt_u(t, ++count));
	fmt_print(")\n");

    hb = (nddal_block_head_t *) _nddal_base;
    do {
        _nddal_check(i, hb);
        if (hb->next && (long) hb->next < (long) hb) {
            fmt_print("block # ");
			fmt_print(fmt_u(t, i));
			fmt_print(" is corrupted, stopping check\n");
		    if (_nddal_stack) { 
				fmt_print("Here is the stack :\n");
				stack_live_dump();
			}
            break;
        }
        hb = hb->next;
        i++;
    } while (hb);
} 

static void
_nddal_check_atexit() {
    fmt_print("\n");
    fmt_print("--------------------\n");
    fmt_print(" -- atexit check -- \n");
    fmt_print("--------------------\n");
    fmt_print("\n");
	_nddal_check_all();
}
	

static void 
_nddal_check_all() {
    int i = 0;
    nddal_block_head_t *hb = (nddal_block_head_t *) _nddal_base;

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
int 
_brk(void *foo) {
    char t[500];

	fmt_print("**** nddal ALERT: _brk is directly called from program\n");

    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        fmt_print("_brk(");
		fmt_print(fmt_ptr(t, foo));
		fmt_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return 0;
}

int 
brk(void *foo) {
    char t[500];
    fmt_print("**** nddal ALERT: brk is directly called from program\n");
    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        fmt_print("brk(");
		fmt_print(fmt_ptr(t, foo));
		fmt_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return 0;
}

void *
_sbkr(int size) {
    char t[500];
    fmt_print("nddal ALERT: _sbrk is directly called from program\n");
    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        fmt_print("_sbrk(");
		fmt_print(fmt_u(t, size));
		fmt_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return _malloc(size);
}

void *
sbrk(intptr_t size) {
    char t[500];
    fmt_print("nddal ALERT: sbrk is directly called from program\n");
    if (!_nddal_private) _nddal_init();
    if (_nddal_dump) {
        fmt_print("sbrk(");
		fmt_print(fmt_u(t, size));
		fmt_print(")\n");
        if (_nddal_stack) stack_live_dump();
    }
    return _malloc(size);
}

void *
_malloc(size_t size) {
	char t[50];
    void *p;

    if (!_nddal_private) {
        fmt_print("init called from malloc()\n");
        _nddal_init();
    }
    if (_nddal_dbg) {
        fmt_print("malloc(");
		fmt_print(fmt_u(t, size));
        if (_nddal_stack) {
			fmt_print(") called from:\n");
			stack_live_dump();
		} else {
			fmt_print(")\n");
		}
    }
    if (size == 0) fmt_print("malloc(0)\n");

    nddal_lock();
    p = _nddal_malloc(size); 
    nddal_unlock();
    _nddal_stats.nmalloc++;

    if (!p) {
		fmt_print("malloc(");
		fmt_print(fmt_u(t, size));
		fmt_print(") FAILED\n");
        if (_nddal_stack) stack_live_dump();      
    }


    if (p && _nddal_dbg) {
		fmt_print("allocated ");
		fmt_print(fmt_u(t, size));
		fmt_print(" bytes at ");
		fmt_print(fmt_ptr(t, p));
		fmt_print("\n");
	}
    return p;       
}

void 
_free(void *ptr) {
	char t[50];

    if (!_nddal_private) return;
    if (_nddal_dbg) {
        fmt_print("free(");
		fmt_print(fmt_ptr(t, ptr));
		if (_nddal_stack) {
			fmt_print(") called from:\n");
			stack_live_dump();
		} else  {
			fmt_print(")\n");
		}
    }
    if (!ptr) {
		if (!_nddal_no_free_null) {
			fmt_print("free(NULL):\n");
			if (_nddal_stack) stack_live_dump();
		}
        return;
    } 
    nddal_lock();
    nddal_free(ptr);
    nddal_unlock();

    _nddal_stats.nfree++;

    if (_nddal_dbg) {
		fmt_print("freed ");
		fmt_print(fmt_ptr(t, ptr));	
		fmt_print("\n");
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
			fmt_print("realloc(");
			fmt_print(fmt_ptr(t, ptr));
			fmt_print(", ");
		} else fmt_print("realloc(0, ");

		fmt_print(fmt_u(t, newsize));

		if (_nddal_stack) {
			fmt_print(") called from:\n");
			stack_live_dump();
		} else {
			fmt_print(")\n");
		}
		if (ptr) {
			p = (char *) ptr;
			hb = (nddal_block_head_t *) (p - sizeof(nddal_block_head_t));
			nddal_check_block(hb);
		}
    }
    if (!ptr) {
        if (!_nddal_realloc_as_malloc) {
            fmt_print("realloc used as malloc (null input pointer)\n");
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
            fmt_print("realloc truncate current data\n");
            if (_nddal_stack) stack_live_dump();
        }
    } else {
		fmt_print("realloc("); 
        fmt_print(fmt_ptr(t, ptr));
		fmt_print(", ");
        fmt_print(fmt_u(t, newsize));
		fmt_print(") (was ");
        fmt_print(fmt_u(t, size));
		fmt_print(") FAILED\n");
        if (_nddal_stack) stack_live_dump();
    }
    _nddal_stats.nrealloc++;

	if (_nddal_dbg) {
		fmt_print("reallocated ");
		fmt_print(fmt_u(t, newsize));
		fmt_print(" (from ");
		fmt_print(fmt_u(t, size));
		fmt_print(") at ");
		fmt_print(fmt_ptr(t, ptr));
		fmt_print(" (from ");
		fmt_print(fmt_ptr(t, ptr));
		fmt_print(")\n");
	}

    return q;
}

/* Redefine malloc, realloc and free : */
void *
malloc(size_t size) { 
	return _malloc(size); 
}

void *
realloc(void *ptr, size_t newsize) { 
	return _realloc(ptr, newsize); 
}

void 
free(void *ptr) { 
	_free(ptr); 
}
