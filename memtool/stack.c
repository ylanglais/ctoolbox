
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <ucontext.h>

#include <unistd.h>
#include <strings.h>
#include <string.h>

#include <tbx/fmt.h>
#include "stack.h"

static void   **_stack_area = 0;
static void   **_stack_curr = 0;
static size_t  _stack_size  = 0;
static void   **_stack_max  = 0;

#if !defined(SUN) && !defined(LINUX) 
void 
stack_init(int size) {
    fmt_print("stack error: platform not supported yet... functionality disabled\n");
}
#else
void
stack_init(void *from, size_t size) {
	char b[1000];
    extern void * _nddal_morecore(size_t); 

	_stack_area = from;
    _stack_size = size;

	fmt_print("Stack buffer allocated from ");
	fmt_print(fmt_ptr(b, _stack_area));
	fmt_print(" to ");
	fmt_print(fmt_ptr(b, (char *) _stack_area + _stack_size));
	fmt_print(".\n");
    _stack_curr = _stack_area;
	_stack_max  = (void *) ((char *) from + size - sizeof(void *)); 
}
#endif

void *stack_curr_get() { return (void *) _stack_curr; }

int
stack_print(void *ptr) {
    Dl_info info;
	char b[1024];

    if (dladdr(ptr, &info)) {
		fmt_print("     ");
		fmt_print(fmt_ptr(b, info.dli_saddr));
		if (info.dli_fname) {
			fmt_print(" file: ");
			fmt_print((char *) info.dli_fname);
		}
		if (info.dli_sname) {
			fmt_print(" function: ");
			fmt_print((char *) info.dli_sname);
		}
		fmt_print("\n");
#ifdef LINUX
		if (!strcmp(info.dli_fname, "__libc_start_main ")) return 1;
#endif
    } else return 1;

    return 0;
}

void 
stack_dump(int n, void *ptr) {
    int i;
    void **p;
    if (!_stack_area) return;
    if (n < 0 || n > _stack_size - 2) {
        fmt_print("stack error: bad stack count\n");
        return;
    }
    p = (void **) ptr;
    if (p < _stack_area || p > _stack_curr) {
        fmt_print("stack error: bad stack pointer\n");
        return;
    }
    for (i = 2; i < n - 1; i++) {
        Dl_info info;
		char b[1024];

        if (dladdr(p[i], &info)) {
			fmt_print("     [");
			fmt_print(fmt_u(b, (unsigned int) n - i - 1));
			fmt_print("] ");
			fmt_print(fmt_ptr(b, info.dli_saddr));
			if (info.dli_fname) {
				fmt_print(" file: ");
				fmt_print((char *) info.dli_fname);
			}
			if (info.dli_sname) {
				fmt_print(" function: ");
				fmt_print((char *) info.dli_sname);
			}
			fmt_print("\n");
        } else {
			fmt_print("stack warning: dladdr failed to retrieve information for ");
			fmt_print(fmt_ptr(b, ptr));
			fmt_print("\n");
		}
    }
}

#if defined(SUN)
#include <sys/frame.h>
#else 
struct frame {
    struct frame *fr_savfp;
    void         *fr_savpc;
};

#ifndef REG_EBP
#define REG_EBP (5 << 8) 
#endif
#define REG_SP REG_EBP
#endif

struct frame *
_stack_frame_get() {
    ucontext_t u;
    (void) getcontext(&u);
    return (((struct frame *) u.uc_mcontext.gregs[REG_SP])->fr_savfp);
}

void 
_stack_live_trace(struct frame *fp) {
    /* skip upper function stack: */
    if (fp) fp = fp->fr_savfp;
    
    /* descend stack: */
    while (fp && !stack_print((void *) fp->fr_savpc)) {
        fp = fp->fr_savfp;
    }
}

void
stack_live_dump() {
    _stack_live_trace(_stack_frame_get());
}

int
_stack_trace(struct frame *fp) {
    int     i = 0;

    if (_stack_curr >= _stack_max) {
        //fmt_print("NO MORE ROOM FOR STACK TRACES\n");
        return 0;
    }

    while (fp && _stack_curr < _stack_max) {
        *_stack_curr = (void *) fp->fr_savpc;
        _stack_curr++;
        fp = fp->fr_savfp;
        i++;
    }

	if (_stack_curr >= _stack_max) {
		fmt_print("* NO MORE ROOM FOR STACK TRACES *\n");
        return 0;
    }

    return i;
}

int 
stack_trace() {
    if (!_stack_area) return 0;
    if (_stack_curr >= _stack_max) return 0;
    return _stack_trace(_stack_frame_get()); 
}

