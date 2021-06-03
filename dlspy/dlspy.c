#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#ifdef  _LINUX_
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#include <dlfcn.h>

typedef struct {
	char *libname;
	void *handle;
} dls_lookup_t;

static int 	__nentries = 0;
static dls_lookup_t __lookuptable[1024];

char *
dlsym_lookup(void *p) {
	int i;
	for (i = 0; i < __nentries && __lookuptable[i].handle != p; i++);
	if (i < __nentries) return  __lookuptable[i].libname;
	return NULL;
}

void 
dlsym_add_lib(char *libname, void *handle) {
	if (__nentries >= 1024) return;
 	__lookuptable[__nentries].handle  = handle;
 	__lookuptable[__nentries].libname = strdup(libname);
	__nentries++;
}

void dlsym_free_lookup() {
	int i;
	for (i = 0; i < __nentries; i++) if (__lookuptable[i].libname) free(__lookuptable[i].libname);
}

typedef struct {
    void *handle;
    void *(*open)(const char  *, int);
    void *(*sym)(void *, const char *);
    char *(*error)(void);
    int   (*close)(void *);
} dlspy_t;

dlspy_t __dlspy__ = { NULL, NULL, NULL, NULL, NULL };

#ifdef LINUX
#include "config.h"
#ifndef DLSPY_GLIBC_VERSION
#define DLSPY_GLIBC_VERSION "GLIBC_2.0"
#endif
#else
extern void * _dlsym(void *, const char *);
extern void * _dlopen(const char *, int);
extern char * _dlerror(void);
extern int    _dlclose(void *);
#endif

void 
dlspy_init() {
    /* 1st step: save standard dl* functions */
#if defined(SUN)    
    __dlspy__.sym   = (void *(*)(void *, const char *)) _dlsym;
    __dlspy__.open  = (void *(*)(const char *, int))    _dlopen;
    __dlspy__.error = (char *(*)(void))                 _dlerror;
    __dlspy__.close = (int   (*)(void *))               _dlclose;
#elif defined(LINUX)
    if (!(__dlspy__.sym = dlvsym(RTLD_NEXT, "dlsym", DLSPY_GLIBC_VERSION))) {
        perror("dlspy fatal: cannot fetch system's dlsym entry point\n");
        exit(1);
    }
    __dlspy__.open  = (void *(*)(const char *, int)) __dlspy__.sym(RTLD_NEXT, "dlopen");
    __dlspy__.error = (char *(*)(void))              __dlspy__.sym(RTLD_NEXT, "dlerror");
    __dlspy__.close = (int   (*)(void *))            __dlspy__.sym(RTLD_NEXT, "dlclose");
#else 
#error ERROR: cannot get dl* original entry points.;
#endif

	dlsym_add_lib("RTLD_NEXT", RTLD_NEXT);
#ifdef SUN
	dlsym_add_lib("RTLD_DEFAULT", RTLD_DEFAULT);
	dlsym_add_lib("RTLD_SELF", RTLD_SELF);
#endif
}

void 
dlspy_echo(char *fmt, ...) {
    va_list args;
    char buf[1024], buf2[1024];
    va_start(args, fmt);
    sprintf(buf2, "dlspy: %s", fmt);
    vsprintf(buf, buf2, args);
    fprintf(stderr, "%s\n", buf);
    va_end(args);
}

char *
dlerror(void) {
    static char *errstr = NULL;
    char *t;
    if (!__dlspy__.open) dlspy_init();
    t = __dlspy__.error();
    if (!t && errstr) return errstr;
    if (t) { 
        errstr = t;
        return t;
    }
    return (char *) strdup(" "); 
}

void *
dlopen(const char *name, int mode) {
    void *t = NULL;

    if (!__dlspy__.open) dlspy_init();
        
    dlspy_echo("try to open \"%s\" shared object with mode %d...", name, mode);
    if (!(t = __dlspy__.open(name, mode))) dlspy_echo("failed (%s)\n", dlerror());  
    else dlspy_echo("ok at handle %x\n", t);
	dlsym_add_lib((char *) name, t);
    return t;
}

void *
dlsym(void *h, const char *name) {
    void *t = NULL;
    if (!__dlspy__.open) dlspy_init();
    dlspy_echo("try to bind symbol \"%s\" from %s (handle %x) as new entry point...", name, dlsym_lookup(h), h);
    if (!(t = __dlspy__.sym(h, name))) dlspy_echo("failed (%s)\n", dlerror());  
    else dlspy_echo("ok at %x\n", t);
    return t;
}

int
dlclose(void *h) {
    if (!__dlspy__.open) dlspy_init();
    dlspy_echo("dlclose(%p)", h);
    return __dlspy__.close(h);
}

