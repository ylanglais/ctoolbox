
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    
*/   

#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "err.h"

char pal_MODULE[]  = "Pal: persistant allocator based on memory mapped file";
char pal_PURPOSE[] = "Allow data persistancy";
char pal_VERSION[] = "1.0.0";
char pal_DATEVER[] = "08/04/2010";

static char pal_MAGIC[]  = "PaLlOc_";
static long pal_FMTVER = 0xFF000001;

typedef struct {
	char 	magic[8];
	long 	fmtver;
	char 	hostname[64];
	pid_t   pid;
	uid_t   uid;
	size_t  size;
	size_t  tsize;
	int 	fid;
	int 	lock;
	int     userlock;
} pal_hdr_t, *ppal_hdr_t;

typedef struct {
	pal_hdr_t hdr;
	char     data[8];
} pal_t, *ppal_t;
	
#if 0
#define _pal_c_
#include "pal.h"
#undef  _pal_c_
#endif 

size_t pal_sizeof()     { return sizeof(pal_t); }
size_t pal_hdr_sizeof() { return sizeof(pal_hdr_t); }

static ppal_hdr_t _pal_hdr_(void *p) { return (ppal_hdr_t) ((char *) p - offsetof(pal_t, data)); }

void pal_hdr_dump(void *p) {
	ppal_hdr_t h;
	if (!p) {
		err_warning("Null pal pointer, ignored");
		return;
	}
	if (!(h = _pal_hdr_(p))) {
		err_warning("not a persistant storage pointer\n");
		return;
	}
	err_debug("persistant storage internal data:");
	err_debug("ppal :     %X", h);
	err_debug("magic :   \"%s\"", h->magic);
	err_debug("fmtver:    %X", h->fmtver);
	err_debug("hostname: \"%s\"", h->hostname);
    err_debug("pid :      %u", h->pid);
    err_debug("uid :      %u", h->uid);
    err_debug("size :     %u", h->size);
    err_debug("tsize :    %u", h->tsize);
    err_debug("fid :      %d", h->fid);
    err_debug("lock :     %d", h->lock);
	err_debug("userlock:  %d", h->userlock);
	err_debug("data:      %X", ((ppal_t) h)->data);
	//err_debug("data:     \"%s\"", (char *) ((ppal_t) h)->data);
	return;
}

#if 0
static void
_pal_lock_(ppal_t pal) {
	if (pal) {
		while (pal->hdr.lock);
		pal->hdr.lock = 0;
	}
}

static void 
_pal_unlock_(ppal_t pal) {
	if (pal->hdr.lock) pal->hdr.lock = 0;
}
#endif

void *
prealloc(void *p, size_t size) {
	void *q;
	ppal_t	pal;
	long long tsize;
	
	if (!p) return NULL;
	if (!(pal = (ppal_t) _pal_hdr_(p))) return NULL;

	/* Compute actual real size */
	tsize = pal->hdr.tsize + (size - pal->hdr.size);
	
	if (ftruncate(pal->hdr.fid, tsize)) {
		err_error("cannot realloc : %s", strerror(errno));
		return NULL;
	}
	pal->hdr.size  = size;
	pal->hdr.tsize = tsize;
	
	/* remap file: */
	if (!(q = mmap((void *) pal, pal->hdr.tsize, 0, MAP_SHARED, pal->hdr.fid, 0))) {
		err_error("cannot remap resized persistant storage: %s", strerror(errno));
		return NULL;
	}	
	
	return q;	
}

void *
pfree(void *p) {
	ppal_hdr_t h;

	if (!p) return NULL;

	h = _pal_hdr_(p);

	if (h->fid >= 0) {
		int fid;
		size_t tsize;
		
		fid    = h->fid;
		tsize  = h->tsize;
		h->fid = -1;

		msync((void *) h, h->tsize, MS_SYNC);
		munmap((void *) h, tsize);
		close(fid);
	}
	return NULL;
}

void *
pmalloc(char *filename, size_t size) {
	int isnew = 0;
	//ppal_t pal;
	ppal_hdr_t h;
	struct stat stats;
	int fid;
	size_t tsize;

	/* Check if file exists or not: */
	if  (stat(filename, &stats)) {
		isnew = 1;
		tsize = sizeof(pal_t) - 8 * sizeof(char) + size;
	} else tsize = stats.st_size;

	if ((fid = open(filename, O_RDWR | O_SYNC | O_CREAT, 00640)) < 0) {
		return NULL;
	}

	if (isnew) {
		if (ftruncate(fid, tsize)) {
			perror(strerror(errno));
			close(fid);
			return NULL;
		}

		if (!(h = (ppal_hdr_t) mmap(NULL, tsize, PROT_READ | PROT_WRITE, MAP_SHARED, fid, 0))) {
			perror(strerror(errno));
			close(fid);
			return NULL;
		}
		strcpy(h->magic, pal_MAGIC);	
		h->fmtver = pal_FMTVER; 
		gethostname(h->hostname, 64);	
		h->uid   = getuid();
		h->pid   = getpid();
		h->fid   = fid;
		h->tsize = tsize;
		h->size  = size;
		h->lock  = 0;
		h->userlock = 0;
	} else {
		if (!(h = (ppal_hdr_t) mmap(NULL, tsize, PROT_READ | PROT_WRITE, MAP_SHARED, fid, 0))) {
			err_error("cannot map file \"%s\": %s", filename, strerror(errno));
			close(fid);
			return NULL;
		}

		/* post mapping check list */
		if (strcmp(h->magic, pal_MAGIC)) { 
			err_error("%s is a squib", filename);
			munmap((void *) h, tsize);
			close(fid);
			return NULL;
		}

		if ((h->fid != -1)) {
			err_warning("file %s seems to be already in use by uid %d on %s pid %d. If not the case, reset usage.", filename, h->uid, h->hostname, h->pid);
			//munmap(pal, tsize);
			//close(fid);
			//return NULL;
			if (fid != h->fid) err_warning("new fid different from previous!!!!");
		}
		gethostname(h->hostname,64);	
		h->uid   = getuid();
		h->pid   = getpid();
		h->fid   = fid;

		//err_debug("------> after load:");
		//pal_hdr_dump(&(((ppal_t) h)->data));
	}

	return (void *) &(((ppal_t) h)->data);
}

#ifdef _test_pal_
#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }

int
main(void) {
	char *p;

	err_init(NULL, err_DEBUG);

	printf("Try to create 10 kb persistant storage file\n");
	if (!(p = pmalloc("paltest.mem", 1024 * 10))) {
		printf("cannot allocate persistant memory\n");
		return 1;
	}
		
	pal_hdr_dump(p);
	
	printf("copy \"0123456789\" to persistant storage\n");
	strcpy(p, "0123456789");

	printf("close persistant storage\n");
	p = pfree(p);

	printf("reopen persistant storage\n");
	if (!(p = pmalloc("paltest.mem", 0))) {
		printf("cannot reopent presistant storage\n");
		return 2;
	}

	printf("persistant storage contains \"%s\"\n", p);
	pal_hdr_dump(p);

	printf("resize persistant storage to 1MB\n");
	if (!(prealloc(p, 1024 * 1024))) {
		printf("err resizing pstorage to 1GB\n");
	}
	
	printf("copy \"9876543210\" to persistant storage\n");
	strcpy(p, "9876543210");

	printf("close persistant storage\n");
	p = pfree(p);

	printf("reopen persistant storage\n");
	if (!(p = pmalloc("paltest.mem", 0))) {
		printf("cannot reopent presistant storage\n");
		return 2;
	}

	printf("persistant storage contains \"%s\"\n", p);
	pal_hdr_dump(p);
	
	p = pfree(p);
	return 0;
}
	

#endif
