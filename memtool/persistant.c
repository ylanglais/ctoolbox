
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

char prsm_NAME[]    = "persistant memory based on memory mapped files";
char prsm_VERSION[] = "1.0.0";
char prsm_DATEVER[] = "29/06/2007";

typedef struct {
	size_t  size;
	int 	fid;
	char   *data;	
	int 	flag;
	int 	open_flag;
	int 	mprsm_flag;
	int 	lock;
} prsm_t, *pprsm_t;

#define _prsm_c_
#include "prsm.h"
#undef  _prsm_c_

static void
_prsm_lock_(pprsm_t prsm) {
	if (prsm) {
		while (prsm->lock);
		prsm->lock = 0;
	}
}

static void 
_prsm_unlock_(pprsm_t prsm) {
	if (prsm->lock) prsm->lock = 0;
}

static int
_prsm_resize_(pprsm_t prsm, int delta) {
	char *p;
	if (!prsm || !prsm->data)      return 1;
	if (prsm->flag == prsmRDONLY)  return 55;
	ftruncate(prsm->fid, prsm->size + delta);
	if (!(p = mprsm(prsm->data, prsm->size + delta, prsm->mprsm_flag, MAP_FIXED | MAP_SHARED, prsm->fid, 0))) 
		                         return 2;
	if (p != prsm->data)          return 3;
/*	printf("oldsize = %d\n", prsm->size); */
	prsm->size += delta;
/*	printf("newsize = %d\n", prsm->size); */
	return 0;
}

pprsm_t
prsm_destroy(pprsm_t prsm) {
	if (prsm) {
		if (prsm->fid > 0) {
			msync(prsm->data, prsm->size, MS_SYNC);
			munprsm(prsm->data, prsm->size);
			close(prsm->fid);
		}
		memset(prsm, 0, sizeof(prsm_t));
		free(prsm); prsm = NULL;
	}
	return NULL;
}

pprsm_t prsm_new(char *filename, int flag) {
	pprsm_t prsm;
	struct stat stats;
	int f;

	if (stat(filename, &stats)) return NULL;

	if (!(prsm = (pprsm_t) malloc(sizeof(prsm_t)))) return NULL;
	memset(prsm, 0, sizeof(prsm_t));

	prsm->size = stats.st_size;
	prsm->flag = flag;

	if (flag) prsm->open_flag = O_RDONLY | O_SYNC;
	else      prsm->open_flag = O_RDWR   | O_SYNC;

	if ((prsm->fid = open(filename, prsm->open_flag)) < 0) {
		return prsm_destroy(prsm); 
	}

	if (flag) prsm->mprsm_flag = PROT_READ;
	else      prsm->mprsm_flag = PROT_READ | PROT_WRITE;
	
	if (!(prsm->data = mprsm(NULL, prsm->size, prsm->mprsm_flag, MAP_SHARED, prsm->fid, 0))) {
		return prsm_destroy(prsm);
	}
	return prsm;
}

size_t
prsm_size_get(pprsm_t prsm) {
	if (prsm) return prsm->size;
	return 0;
}

char *
prsm_data_get(pprsm_t prsm) {
	if (!prsm || !prsm->data) return NULL;
	if (prsm)                 return prsm->data;
	return NULL;
}

int
prsm_flag_get(pprsm_t prsm) {
	if (prsm) return prsm->flag;
	return -1;
}

int
prsm_insert_at(pprsm_t prsm, char *p, char *data, size_t size) {
	if (!prsm || !prsm->data)     return 1;
	if (prsm->flag == prsmRDONLY) return 55;
	_prsm_lock_(prsm);

	if (_prsm_resize_(prsm, size)) {
		_prsm_unlock_(prsm);
		return 2;
	}
	memmove(p + size, p, (int) prsm->size - ((int) p - (int) prsm->data) - size);
	memcpy(p, data, size);
	/* msync(prsm->data, prsm->size, MS_SYNC); */
	_prsm_unlock_(prsm);
	return 0;
}

int
prsm_delete_at(pprsm_t prsm, char *p, size_t size) {
	if (!prsm || !prsm->data)     return 1;
	if (prsm->flag == prsmRDONLY) return 55;

	_prsm_lock_(prsm);
	if (size > (size_t) ((int) prsm->size - (int) p + (int) prsm->data)) 
		size = (size_t) ((int) prsm->size - (int) p + (int) prsm->data);

	memmove(p, p  + size, (size_t) ((int) prsm->size - (int) p + (int) prsm->data - (int) size));
	if (_prsm_resize_(prsm, -size)) {
		_prsm_unlock_(prsm);
		return 2;
	}
	/* msync(prsm->data, prsm->size, MS_SYNC); */
	_prsm_unlock_(prsm);
	return 0;	
}

int
prsm_replace_at(pprsm_t prsm, char *p, char *data, size_t oldsize, size_t newsize) {
	if (!prsm || !prsm->data)    		    return 1;
	if (prsm->flag == prsmRDONLY)         return 55;
	if (prsm_delete_at(prsm, p, oldsize)) return 2;
	if (prsm_insert_at(prsm, p, data, newsize)) return 3;
	return 0;
}

#ifdef _test_prsm_

#include <stdio.h>
#include <errno.h>

static char orig[] = "012345678901234567890";
static char file[] = "prsm_test.data";

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
	pprsm_t prsm;
	char *str1="hahaha";
	int i;

	echo_cmd(write_orig());
	echo_cmd(cat_orig());
	echo_cmd(prsm = prsm_new(file, 0));
	echo_cmd(p   = prsm_data_get(prsm));
	echo_cmd(prsm_insert_at(prsm, p, str1, strlen(str1)));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());	
	echo_cmd(prsm_delete_at(prsm, p, 6)); 
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	echo_cmd(prsm_replace_at(prsm, p+10, "iiiiii", 5, 6));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	echo_cmd(prsm_insert_at(prsm, p, "ohohoh", 6));
	echo_cmd(prsm_destroy(prsm));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	return 0;
}
#endif 
