
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    
*/   

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

char pal_NAME[]    = "pal: persistant allocator based on memory mapped file";
char pal_VERSION[] = "1.0.0";
char pal_DATEVER[] = "08/04/2010";

typedef struct {
	size_t  size;
	int 	fid;
	char   *data;
	int 	open_flag;
	int 	mmap_flag;
	int 	lock;
} pal_t, *ppal_t;

#define _pal_c_
#include "pal.h"
#undef  _pal_c_

static size_t pal_sizeof() { return sizeof(pal_t); }

static void
_pal_lock_(ppal_t pal) {
	if (pal) {
		while (pal->lock);
		pal->lock = 0;
	}
}

static void 
_pal_unlock_(ppal_t pal) {
	if (pal->lock) pal->lock = 0;
}

static int
_pal_resize_(ppal_t pal, int delta) {
	char *p;
	if (!pal || !pal->data)      return 1;
	ftruncate(pal->fid, pal->size + delta);
	if (!(p = mmap(pal->data, pal->size + delta, 0, MAP_SHARED, pal->fid, 0))) 
		                         return 2;
	if (p != pal->data)          return 3;
/*	printf("oldsize = %d\n", pal->size); */
	pal->size += delta;
/*	printf("newsize = %d\n", pal->size); */
	return 0;
}

static ppal_t
pal_destroy(ppal_t pal) {
	if (pal) {
		if (pal->fid > 0) {
			msync(pal->data, pal->size, MS_SYNC);
			munmap(pal->data, pal->size);
			close(pal->fid);
		}
		memset(pal, 0, sizeof(pal_t));
		free(pal); pal = NULL;
	}
	return NULL;
}

	
static ppal_t 
pal_new(char *filename, size_t size) {
	ppal_t pal;
	struct stat stats;

	if (stat(filename, &stats)) return NULL;

	if (!(pal = (ppal_t) malloc(sizeof(pal_t)))) return NULL;
	memset(pal, 0, sizeof(pal_t));

	pal->size = stats.st_size;

	if (flag) pal->open_flag = O_RDONLY | O_SYNC;
	else      pal->open_flag = O_RDWR   | O_SYNC;

	if ((pal->fid = open(filename, pal->open_flag)) < 0) {
		return pal_destroy(pal); 
	}

	if (flag) pal->mmap_flag = PROT_READ;
	else      pal->mmap_flag = PROT_READ | PROT_WRITE;
	
	if (!(pal->data = mmap(NULL, pal->size, pal->mmap_flag, MAP_SHARED, pal->fid, 0))) {
		return pal_destroy(pal);
	}
	return pal;
}

static int 
pal_size_set(ppal_t pal, size_t size) {
	return _pal_resize_(pal, size - pal->size);
}

static size_t
pal_size_get(ppal_t pal) {
	if (pal) return pal->size;
	return 0;
}

static char *
pal_data_get(ppal_t pal) {
	if (!pal || !pal->data) return NULL;
	if (pal)                return pal->data;
	return NULL;
}

static int
pal_flag_get(ppal_t pal) {
	if (pal) return pal->flag;
	return -1;
}

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

void *
palloc(char *filename, size_t size) {
	ppal_t p;
	if (!(p = pal_new(filename, size)))
		return O
	return p->data;
}

void *
prealloc(void *ptr, size_t size) {
	return pal_size_set(ptr - pal_sizeof(), size);
}

void 
pfree(void *ptr) {
	pal_destroy(ptr - pal_sozeof());
}

void *
preload(char *filename) {
}

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

#ifdef _test_pal_

#include <stdio.h>
#include <errno.h>

static char orig[] = "012345678901234567890";
static char file[] = "pal_test.data";

void
write_orig() {
	int f;
	f = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
	write(f, orig, strlen(orig));
	close(f);
}

void 
cat_orig() {
	int f;
	char *buf;
	struct stat stats;
	if (!stat(file, &stats)) {
		if ((buf = (char *) malloc(stats.st_size))) {
			memset(buf, 0, stats.st_size);
			f = open(file, O_RDONLY);
			read(f, buf, stats.st_size);
			write(1, buf, stats.st_size);
			write(1, "\n", 1);
			free(buf);
		}
	}
	close(f);
}
	
void 
size_orig() {
	struct stat stats;
	if (!stat(file, &stats)) {
		printf("size = %d\n", stats.st_size);
	}
}

#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }

int 
main(void) {
	char *p;
	ppal_t pal;
	char *str1="hahaha";
	int i;

	echo_cmd(write_orig());
	echo_cmd(cat_orig());
	echo_cmd(pal = pal_new(file, 0));
	echo_cmd(p   = pal_data_get(pal));
	echo_cmd(pal_insert_at(pal, p, str1, strlen(str1)));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());	
	echo_cmd(pal_delete_at(pal, p, 6)); 
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	echo_cmd(pal_replace_at(pal, p+10, "iiiiii", 5, 6));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	echo_cmd(pal_insert_at(pal, p, "ohohoh", 6));
	echo_cmd(pal_destroy(pal));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	return 0;
}
#endif 
