#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include <stack.h>
#include <mdbg_utils.h>

void _mdbg_init();

#ifdef SOLARIS
#pragma init (_mdbg_init)
#endif
#ifdef LINUX
void _mdbg_init() __attribute__((constructor));
#endif

static size_t  _mdbg_private_len = 0;
static void   *_mdbg_private     = NULL;
static void   *_mdbg_base        = NULL;
static void   *_mdbg_curr        = NULL;

void *_malloc(size_t size);

/*static unsigned long _mdbg_incr = 0; */

static char mdbg_magic_head[] = "#MeM cHeCkEr: HeAdErMaGicStRiNg";
static char mdbg_magic_tail[] = "*mEm ChEcKeR: tAiLeRmAgICsTrInG";
#define mdbg_HEAD_LEN 32
#define mdbg_TAIL_LEN 32

#define mdbg_MAXSTACKS (1024 * 1024 * sizeof(void *))

typedef void *(*fcore)(size_t size);

static fcore _mdbg_sbrk = NULL;

static int _mdbg_lock = 0;

enum {mdbg_Unset, mdbg_Used, mdbg_Free};
static char *mdbg_status_str[] = {
    "Unset", "InUse", "Freed"
};

typedef struct {
    unsigned long nmalloc;
    unsigned long nrealloc;
    unsigned long nfree;
} stat_t, *pstat_t;

static stat_t _mdbg_stats;

void 
mdbg_print2(char *s) {
	write(2, s, strlen(s));
}

void mdbg_priv_lock()  { 
    if (mprotect(_mdbg_private, _mdbg_private_len, PROT_NONE))
        mdbg_print2("mprotect lock FAILED\n");
}

void mdbg_priv_read()  { 
    if (mprotect(_mdbg_private, _mdbg_private_len, PROT_READ))
        mdbg_print2("mprotect read unlock FAILED\n");
}

void mdbg_priv_write() { 
    if (mprotect(_mdbg_private, _mdbg_private_len, PROT_READ | PROT_WRITE))
        mdbg_print2("mprotect write unlock FAILED\n");
}

void mdbg_lock()   {
    while (_mdbg_lock);
    _mdbg_lock = 1;
}
void mdbg_unlock() {
    _mdbg_lock = 0; 
}

typedef struct {
    size_t  size;
    size_t  status;
    void    *next;
    int             nstacks;
    void    *pstack;
    char    magic_head[mdbg_HEAD_LEN];
    char    align64[12];
} mdbg_block_head_t;

typedef struct {
    char    magic_tail[mdbg_TAIL_LEN];
    char    align64[mdbg_TAIL_LEN];
} mdbg_block_tail_t;

#define ALIGN 8 

#define mdbg_BUFF 1000
#define mdbg_ERR  2
#define mdbg_OUT  1

void _mdbg_dump_data(char *buff, size_t size) {
    int i, j;
    char hex[256], asc[256], tmp[512], final[512];
	char *h, *a, *t, *f;

    if (buff == NULL) {
       	mdbg_print2("   null pointer\n");
        return;
    }

    mdbg_print2("   block dump  :\n");
	

    /* 00 11 22 33 44 55 66 77 - 88 99 AA BB CC DD EE FF   azertyui - qsdfghjk */
    for (i = 0; i < size; i += 16) {
        *hex = *asc = *final = 0;
		h = hex, a = asc, t = tmp;
        for (j = 0; j < 16 && i + j < size; j++) {
            if (j == 8) {
                h = mdbg_str_apend(h, "- ");
                a = mdbg_str_apend(a, " - ");
            }
			h = mdbg_str_apend(h, mdbg_fmt_xchar(tmp, (unsigned char) buff[i+j]));
			h = mdbg_str_apend(h, " ");
			a = mdbg_str_apend(a, mdbg_fmt_char(tmp, buff[i + j]));
			h = mdbg_str_apend(h, " ");
        }
		mdbg_print2("      ");	
		mdbg_print2(mdbg_fmt_ptr(tmp, buff + i)); 
		mdbg_print2(": ");
		mdbg_print2(hex);
		mdbg_print2("  ");
		mdbg_print2(asc);
        mdbg_print2("\n");
    }
}

void
_mdbg_hdr_dump(mdbg_block_head_t *h) {
	char t[500];

    if (!h) return;
    mdbg_print2("haddr  = ");
	mdbg_print2(mdbg_fmt_ptr(t, h));
	mdbg_print2("\n");
	mdbg_print2("size   = ");
	mdbg_print2(mdbg_fmt_u(t, h->size));
	mdbg_print2("\n");

    if (h->status < mdbg_Unset || h->status > mdbg_Free) {
         mdbg_print2("invalid status\n");
    } else { 
		mdbg_print2("status = ");
		mdbg_print2(mdbg_status_str[h->status]);
		mdbg_print2("\n");
	}
    mdbg_print2("next   = ");
	mdbg_print2(mdbg_fmt_ptr(t, h->next));
	mdbg_print2("\n");
	mdbg_print2("magic1 = \"");
	mdbg_print2(h->magic_head);
	mdbg_print2("\n");
	mdbg_print2("magic2 = \"");
	mdbg_print2((char *) h + sizeof(mdbg_block_head_t) + h->size);
	mdbg_print2("\n");
    mdbg_print2("---------------------------\n\n");
}

int _brk(void *foo) {
    char t[500];
	mdbg_print2("mdbg ALERT: _brk is directly called from program\n");

    if (!_mdbg_private) _mdbg_init();
    if (getenv("MDBG_DUMP")) {
        mdbg_print2("_brk(");
		mdbg_print2(mdbg_fmt_ptr(t, foo));
		mdbg_print2(")\n");
        stack_live_dump();
    }
    return 0;
}

int brk(void *foo) {
    char t[500];
    mdbg_print2("mdbg ALERT: brk is directly called from program\n");
    if (!_mdbg_private) _mdbg_init();
    if (getenv("MDBG_DUMP")) {
        mdbg_print2("brk(");
		mdbg_print2(mdbg_fmt_ptr(t, foo));
		mdbg_print2(")\n");
        stack_live_dump();
    }
    return 0;
}

void *_sbkr(int size) {
    char t[500];
    mdbg_print2("mdbg ALERT: _sbrk is directly called from program\n");
    if (!_mdbg_private) _mdbg_init();
    if (getenv("MDBG_DUMP")) {
        mdbg_print2("_sbrk(");
		mdbg_print2(mdbg_fmt_u(t, size));
		mdbg_print2(")\n");
        stack_live_dump();
    }
    return _malloc(size);
}

void *sbrk(intptr_t size) {
    char t[500];
    mdbg_print2("mdbg ALERT: sbrk is directly called from program\n");
    if (!_mdbg_private) _mdbg_init();
    if (getenv("MDBG_DUMP")) {
        mdbg_print2("sbrk(");
		mdbg_print2(mdbg_fmt_u(t, size));
		mdbg_print2(")\n");
        stack_live_dump();
    }
    return _malloc(size);
}

void * 
_mdbg_morecore(size_t size) {
    return _mdbg_sbrk(size);        
}

void
mdbg_check_free(mdbg_block_head_t *hb) {
    int i, dump = 0;
    mdbg_block_tail_t *ht;
	char t[500];	

    ht = (mdbg_block_tail_t *) ((char *) hb + sizeof(mdbg_block_head_t) + hb->size);

    if (strcmp(hb->magic_head, mdbg_magic_head)) dump += 1;
    if (strcmp((char *) ht, mdbg_magic_tail))    dump += 2;

    if (dump) {
		mdbg_print2("trying to free corrupted block at : ");
		mdbg_print2(mdbg_fmt_ptr(t, hb));
		mdbg_print2(":\n");

        if (dump & 1) {
			mdbg_print2("   magic header corrupted (\"");
			mdbg_print2(hb->magic_head);
			mdbg_print2("\"\n");
		}
	
        if (dump & 2) {
			mdbg_print2("   magic tailer corrupted (\"");
			mdbg_print2((char *) hb + sizeof(mdbg_block_head_t) + hb->size);
			mdbg_print2("\"\n");
		}

        if (hb->status < mdbg_Unset || hb->status > mdbg_Free) {
			mdbg_print2("   block status: ");
			mdbg_print2(mdbg_fmt_u(t, hb->status));
			mdbg_print2("(invalid)\n");
		} else {
			mdbg_print2("   block status: ");
			mdbg_print2(mdbg_status_str[hb->status]);
			mdbg_print2("\n");
		}
		mdbg_print2("   block size  : ");
		mdbg_print2(mdbg_fmt_u(t, hb->size));

		mdbg_print2("\n   block next  : ");
		mdbg_print2(mdbg_fmt_ptr(t, hb->next));
		
		mdbg_print2("\n   user ptr    : ");
		mdbg_print2(mdbg_fmt_ptr(t, (char *) hb + sizeof(mdbg_block_head_t)));
		mdbg_print2("\n");

        _mdbg_dump_data((char *) hb + sizeof(mdbg_block_head_t), hb->size);

        mdbg_print2("   allocation time stack dump:\n");
        mdbg_priv_read();       
        stack_dump(hb->nstacks, hb->pstack);
        mdbg_priv_lock();
    }
}

void 
_mdbg_check(int n, mdbg_block_head_t *hb) {
    int i, dump = 0;
    mdbg_block_tail_t *ht;
	char t[500];

    ht = (mdbg_block_tail_t *) ((char *) hb + sizeof(mdbg_block_head_t) + hb->size);

    if (strcmp(hb->magic_head, mdbg_magic_head)) 
        dump += 1;
    if (strcmp((char *) ht, mdbg_magic_tail)) 
        dump += 2;

    if (dump || getenv("MDBG_DUMP")) {

		mdbg_print2("\nBlock number ");
		mdbg_print2(mdbg_fmt_u_n(t, n, 4, ' '));
		mdbg_print2(":\n");

        mdbg_print2("------------------\n");

        if   (dump & 1) mdbg_print2("   magic header corrupted\n");
        if   (dump & 2) mdbg_print2("   magic tailer corrupted\n");

		mdbg_print2("   block address: ");
		mdbg_print2(mdbg_fmt_ptr(t, hb));
		mdbg_print2("\n");
		
        mdbg_print2("   block status : ");
		mdbg_print2(mdbg_status_str[hb->status]);
		mdbg_print2("\n");


        mdbg_print2("   block size   : ");
		mdbg_print2(mdbg_fmt_u(t, hb->size));
		mdbg_print2("\n");
		

        mdbg_print2("   block next   : ");
		mdbg_print2(mdbg_fmt_ptr(t, hb->next));
		mdbg_print2("\n");

        mdbg_print2("   user ptr     : ");
		mdbg_print2(mdbg_fmt_ptr(t, (char *) hb + sizeof(mdbg_block_head_t)));
		mdbg_print2("\n");

        _mdbg_dump_data((char *) hb + sizeof(mdbg_block_head_t), hb->size);
        
		mdbg_print2("   allocation time stack dump:\n");
        mdbg_priv_read();       
        stack_dump(hb->nstacks, hb->pstack);
        mdbg_priv_lock();
    }
}

void
mdbg_check_all() {
    static int count = 0;
    int i = 0;
    mdbg_block_head_t *hb;
	char t[500];
    
    mdbg_print2("check memory blocks (");
	mdbg_print2(mdbg_fmt_u(t, ++count));
	mdbg_print2(")\n");

    hb = (mdbg_block_head_t *) _mdbg_base;
    do {
        _mdbg_check(i, hb);
        if (hb->next && (long) hb->next < (long) hb) {
            mdbg_print2("block # ");
			mdbg_print2(mdbg_fmt_u(t, i));
			mdbg_print2(" is corrupted, stopping check\n");
			mdbg_print2("Here is the stack :\n");
			stack_live_dump();
            break;
        }
        hb = hb->next;
        i++;
    } while (hb);
} 

void _mdbg_stats_print() {
	char t[50];

    mdbg_print2("mdbg stats:\n");
    mdbg_print2("mdbg stats: memory statistics:\n");
    mdbg_print2("mdbg stats: ------------------\n");
    mdbg_print2("mdbg stats: malloc:       ");
	mdbg_print2(mdbg_fmt_u_n(t,	_mdbg_stats.nmalloc, 8, ' '));
    mdbg_print2("\nmdbg stats: realloc:      ");
	mdbg_print2(mdbg_fmt_u_n(t,	_mdbg_stats.nrealloc, 8, ' '));
    mdbg_print2("\nmdbg stats: free:         ");
	mdbg_print2(mdbg_fmt_u_n(t,	_mdbg_stats.nfree, 8, ' '));
    mdbg_print2("\nmdbg stats: still in use: ");
	mdbg_print2(mdbg_fmt_u_n(t, _mdbg_stats.nmalloc - _mdbg_stats.nfree, 8, ' '));
	mdbg_print2("\n");
}   

void
_mdbg_check_all() {
    int i = 0;
    mdbg_block_head_t *hb;

    mdbg_print2("\n");
    mdbg_print2("--------------------\n");
    mdbg_print2(" -- atexit check -- \n");
    mdbg_print2("--------------------\n");
    mdbg_print2("\n");

    for (i = 0, hb = (mdbg_block_head_t *) _mdbg_base; hb; i++, hb = hb->next)
        _mdbg_check(i, hb);
    _mdbg_stats_print();
} 

void
_mdbg_signals(int sig) {
	char t[10];
    mdbg_print2("mdbg caught signal ");
	mdbg_print2(mdbg_fmt_u(t, sig));
    mdbg_print2("\nHere is the callstack when signal has been emitted :\n");
    stack_live_dump();
    mdbg_print2("Now preform memory check...\n");
    _mdbg_check_all();
    mdbg_print2("done\n");
}

size_t 
mdbg_aligned(size_t size) {
    unsigned int residue;
    if ((residue = size % ALIGN))
        size += (ALIGN - residue);
    return size;
}

void _mdbg_init() {
    long pagesize;
    pagesize = sysconf(_SC_PAGE_SIZE);

/*mdbg_print2("_mdbg_init\n"); */

    /* fetch real "mem core function" */
    _mdbg_sbrk = (fcore) dlsym(RTLD_NEXT, "sbrk");

    /* make sure we start on pagesize multiple */
    if (((size_t) _mdbg_sbrk(0)) % pagesize) _mdbg_morecore(pagesize - ((size_t) _mdbg_sbrk(0) % pagesize));


    _mdbg_private = _mdbg_sbrk(0);
    stack_init(mdbg_MAXSTACKS);
    signal(SIGSEGV, _mdbg_signals);

    if (((size_t) _mdbg_sbrk(0)) % pagesize) _mdbg_morecore(pagesize - ((size_t) _mdbg_sbrk(0) % pagesize));
    _mdbg_base = _mdbg_sbrk(0);
    _mdbg_private_len = (char *) _mdbg_base - (char *) _mdbg_private - 1;
    _mdbg_stats.nmalloc  = 
    _mdbg_stats.nrealloc = 
    _mdbg_stats.nfree    = 0;

    mdbg_priv_lock();       
    
    atexit(_mdbg_check_all);
}

void *_mdbg_malloc(size_t size) {
    static int        count = 0;
    size_t            gsize, real;
    void              *p; 
    char                      *q, *r;
    mdbg_block_head_t *h;

    if (getenv("MDBG_ALWAYS_CHECK") && count > 0) mdbg_check_all();

    /* 
     * compute actual reservation size as : 
     * header + tailer + size (adjusted)
     */
    
    h = _mdbg_curr;

    gsize = mdbg_aligned(size);
    real = gsize + sizeof(mdbg_block_head_t) + sizeof(mdbg_block_tail_t);

    if ((p = _mdbg_morecore(real)) == (void *) -1) return NULL;
    if ((unsigned long) p % ALIGN) { 
		char t[50];
        mdbg_print2("ALIGNMENT ERROR: ");
		mdbg_print2(mdbg_fmt_ptr(t, p));
		mdbg_print2(" % ");
		mdbg_print2(mdbg_fmt_u(t, ALIGN));
		mdbg_print2(" = ");
		mdbg_print2(mdbg_fmt_u(t, ((unsigned long) p) % ALIGN));
		mdbg_print2("\n");
		
	}

    if (count) h->next = p;
    
    h = (mdbg_block_head_t *) (q = (char *) p);

    h->size   = gsize;
    h->status = mdbg_Used;
    h->next   = NULL;
    strcpy(h->magic_head, mdbg_magic_head);

/* Fill the allocated but not used bytes withs 'X' to check small leaks: */
    if (size < gsize) memset(q +  sizeof(mdbg_block_head_t) + size, 'X', gsize - size);
    strcpy(q + sizeof(mdbg_block_head_t) + gsize, mdbg_magic_tail);

    /* Find out a way to protect small amounts of mem HERE!!! */

    mdbg_priv_write();
    h->pstack  = stack_curr_get();
    h->nstacks = stack_trace();
    mdbg_priv_lock();

    _mdbg_curr = (void *) h;
    count++;

    if (getenv("MDBG_DBGS")) {
		char t[50];
		mdbg_print2("new block @ 0x");
		mdbg_print2(mdbg_fmt_ptr(t,  h));
		mdbg_print2("\n");
	}
    return q + sizeof(mdbg_block_head_t);
}

void _mdbg_free(void *ptr) {
    char *p;
    mdbg_block_head_t *hb;

    p = (char *) ptr;
    hb = (mdbg_block_head_t *) (p - sizeof(mdbg_block_head_t));

    mdbg_check_free(hb);
    hb->status = mdbg_Free;
}

void *_malloc(size_t size) {
	char t[50];
    void *p;
    if (!_mdbg_private) {
        mdbg_print2("init called from malloc()\n");
        _mdbg_init();
    }
    if (getenv("MDBG_DBG")) {
        mdbg_print2("malloc(");
		mdbg_print2(mdbg_fmt_u(t, size));
		mdbg_print2(") called from:\n");
        stack_live_dump();
    }
    if (size == 0) mdbg_print2("malloc(0)\n");
    mdbg_lock();
    p = _mdbg_malloc(size); 
    if (!p) {
		mdbg_print2("malloc(");
		mdbg_print2(mdbg_fmt_u(t, size));
		mdbg_print2(") FAILED\n");
        if (getenv("MDBG_DBG")) stack_live_dump();      
    }
    _mdbg_stats.nmalloc++;
    mdbg_unlock();
    if (p && getenv("MDBG_DBG")) {
		mdbg_print2("allocated ");
		mdbg_print2(mdbg_fmt_u(t, size));
		mdbg_print2("bytes at ");
		mdbg_print2(mdbg_fmt_ptr(t, p));
		mdbg_print2("\n");
	}
    return p;       
}

void _free(void *ptr) {
	char t[50];

    if (!_mdbg_private) return;
    if (getenv("MDBG_DBG")) {
        mdbg_print2("free(");
		mdbg_print2(mdbg_fmt_ptr(t, ptr));
		mdbg_print2(") called from:\n");
        stack_live_dump();
    }
    if (!ptr) {
        mdbg_print2("free(NULL):\n");
        stack_live_dump();
        return;
    } 
    mdbg_lock();
    _mdbg_free(ptr);
    _mdbg_stats.nfree++;
    mdbg_unlock();
    if (getenv("MDBG_DBG")) {
		mdbg_print2("freed ");
		mdbg_print2(mdbg_fmt_ptr(t, ptr));	
		mdbg_print2("\n");
	}
}

void *
_realloc(void *ptr, size_t newsize) {
    void *q;
    size_t size;
	char t[50];

    if (!_mdbg_private) _mdbg_init();
    if (getenv("MDBG_DBG")) {
        mdbg_print2("realloc(");
		mdbg_print2(mdbg_fmt_ptr(t, ptr));
		mdbg_print2(", ");
		mdbg_print2(mdbg_fmt_u(t, newsize));
		
		mdbg_print2(") called from:\n");
        stack_live_dump();
    }
    if (!ptr) {
        if (getenv("MDBG_REALLOC_AS_MALLOC")) {
            mdbg_print2("realloc used as malloc (null input pointer)\n");
            if (getenv("MDBG_DUMP")) stack_live_dump();
        }
        return _malloc(newsize);
    }

    mdbg_lock();

    size = (((mdbg_block_head_t *) ptr) - 1)->size;

    if (getenv("MDBG_ALWAYS_CHECK")) mdbg_check_all();
    
    _mdbg_free(ptr);
    if ((q = _mdbg_malloc(newsize))) {
        memcpy(q, ptr, newsize < size ? newsize : size);
        if (newsize < size) {
            mdbg_print2("realloc truncate current data:\n");
            stack_live_dump();
        }
    } else {
		mdbg_print2("realloc("); 
        mdbg_print2(mdbg_fmt_ptr(t, ptr));
		mdbg_print2(", ");
        mdbg_print2(mdbg_fmt_u(t, newsize));
		mdbg_print2(") (was ");
        mdbg_print2(mdbg_fmt_u(t, size));
		mdbg_print2(") FAILED\n");
        stack_live_dump();
    }
    _mdbg_stats.nrealloc++;
    mdbg_unlock();
    mdbg_print2("reallocated ");
	mdbg_print2(mdbg_fmt_u(t, newsize));
	mdbg_print2(" (from ");
	mdbg_print2(mdbg_fmt_u(t, size));
	mdbg_print2(") at ");
	mdbg_print2(mdbg_fmt_ptr(t, ptr));
	mdbg_print2(" (from ");
	mdbg_print2(mdbg_fmt_ptr(t, ptr));
	mdbg_print2(")\n");
    return q;
}

/* Redefine malloc, realloc and free : */
void *malloc(size_t size) { return _malloc(size); }
void *realloc(void *ptr, size_t newsize) { return _realloc(ptr, newsize); }
void free(void *ptr) { _free(ptr); }
