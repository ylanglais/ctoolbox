
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    20/11/2017  1.0 Creation
	31/05/2018  1.1 Fix mem allocation for ending 0 byte in futl_load
*/   

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <fcntl.h>
#include <errno.h>
#include <fnmatch.h>

#include "err.h"
#include "mem.h"
#include "crc_32.h"
#include "futl.h"

char futl_MODULE[]  = "File utils";
char futl_PURPOSE[] = "File utils";
char futl_VERSION[] = "1.1";
char futl_DATEVER[] = "31/05/2018";

char *
futl_load(char *fname, size_t *size) {
	char *input;
	size_t lsize;
	struct stat stats;
	int f;

	if (size) *size = 0; 

	if (stat(fname, &stats)) {
		err_error("cannot stat file %s", fname);
		return NULL;
	}

	lsize = stats.st_size;

	if (!(input = mem_zmalloc(lsize + 1))) {
		err_error("cannot allocate %lu bytes", size);
		return NULL;
	}

	if ((f = open(fname, O_RDONLY)) < 0) { 
		err_error("cannot open %s : %s\n", fname, strerror(errno));
		free(input);
		return NULL;
	}
	if (!read(f, input, lsize - 1)) {
		err_error("error reading %s : %s\n", fname, strerror(errno));
		free(input);
		return NULL;
	}
	close(f);

	if (size) *size = lsize;
	return input;
}

int
futl_write(char *fname, char *data, size_t size) {
	int f;
	if ((f = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) { 
		err_error("cannot open %s : %s\n", fname, strerror(errno));
		return 1;
	}
	size_t s;
	if ((s = write(f, data, size)) < size) {
		err_warning("parial writing (%lu instead of %lu bytes) reason:\n", s, size, strerror(errno));
		return 2;
	}
	close(f);
	return 0;
}

size_t
futl_size(char *fname) {
	struct stat stats;

	if (stat(fname, &stats)) {
		err_error("cannot stat file %s", fname);
		return 0;
	}

	return stats.st_size;
}

int futl_exists(char *path) {
	struct stat stats;
	if (stat(path, &stats)) {
		return 0;
	}
	return 1;
}

int
futl_is_file(char *path) {
	struct stat stats;

	if (stat(path, &stats)) {
		err_error("cannot stat file %s", path);
		return 0;
	}
	return S_ISREG(stats.st_mode);
}

int
futl_is_dir(char *path) {
	struct stat stats;

	if (stat(path, &stats)) return 0;
	return S_ISDIR(stats.st_mode);
}

int
futl_is_sock(char *path) {
	struct stat stats;

	if (stat(path, &stats)) return 0;
	return S_ISSOCK(stats.st_mode);
}

int
futl_is_link(char *path) {
	struct stat stats;

	if (stat(path, &stats)) return 0;
	return S_ISLNK(stats.st_mode);
}

int
futl_is_fifo(char *path) {
	struct stat stats;

	if (stat(path, &stats)) return 0;
	return S_ISFIFO(stats.st_mode);
}

int
futl_is_rwx(char *path) {
	return access(path, R_OK | W_OK | X_OK);
}

int
futl_is_rw(char *path) {
	return access(path, R_OK | W_OK);
}
int
futl_is_rx(char *path) {
	return access(path, R_OK | X_OK);
}

int 
futl_is_w(char *path) {
	return access(path, W_OK);;
}

int 
futl_is_r(char *path) {
	return access(path, R_OK);
}

int
futl_is_x(char *path) {
	return access(path, X_OK);
}

int
futl_rm(char *fname) {
	return unlink(fname);
}

int
futl_cp(char *src, char *dst) {
	int r;
	char *data;
	size_t size;
	if (!(data = futl_load(src, &size))) return 1;
	r = futl_write(dst, data, size);
	free(data); 
	return r;
}

int
futl_mv(char *src, char *dst) {
	if (futl_cp(src, dst)) return 1;
	return futl_rm(src);
}

int futl_mkdir(char *path) {
	if ( mkdir(path, 0777)) {
		err_error("cannot create dirctory %s : %s\n", path, strerror(errno));
		return 1;
	 }
	return 0;
}


char *
futl_realpath(char *path) {
	return realpath(path, NULL);
}

unsigned long
futl_crc_32(char *fname) {
	char *p; 
	size_t size;
	unsigned long crc;

	if (!(p = futl_load(fname, &size))) return 0;
	crc = crc_32((unsigned char *) p, size);
	free(p);
	return crc;
}

int
futl_tspec_cmp(tspec_t t1, tspec_t t2) {
	if (t1.tv_sec  > t2.tv_sec)  return -1;	
	if (t1.tv_sec  < t2.tv_sec)  return  1;	
	if (t1.tv_nsec > t2.tv_nsec) return -1;
	if (t1.tv_nsec < t2.tv_nsec) return  1;
	return 0;
}

int
futl_ctime_cmp(const void *t1, const void *t2) {
	pfutl_file_info_t a, b;

	a = (pfutl_file_info_t) t1;
	b = (pfutl_file_info_t) t2;
	
	return futl_tspec_cmp(a->ctime, b->ctime);
}

int
futl_ctime_rcmp(const void *t1, const void *t2) {
	return -futl_ctime_cmp(t1, t2);
}

int
futl_mtime_cmp(const void *t1, const void *t2) {
	pfutl_file_info_t a, b;

	a = (pfutl_file_info_t) t1;
	b = (pfutl_file_info_t) t2;
	
	return futl_tspec_cmp(a->mtime, b->mtime);
}

int
futl_mtime_rcmp(const void *t1, const void *t2) {
	return -futl_mtime_cmp(t1, t2);
}
int

futl_atime_cmp(const void *t1, const void *t2) {
	pfutl_file_info_t a, b;

	a = (pfutl_file_info_t) t1;
	b = (pfutl_file_info_t) t2;
	
	return futl_tspec_cmp(a->atime, b->atime);
}

int
futl_atime_rcmp(const void *t1, const void *t2) {
	return -futl_atime_cmp(t1, t2);
}

int
futl_name_cmp(const void *t1, const void *t2) {
	pfutl_file_info_t a, b;

	a = (pfutl_file_info_t) t1;
	b = (pfutl_file_info_t) t2;
	return strcmp(a->fname, b->fname);
}

int
futl_name_rcmp(const void *t1, const void *t2) {
	return -futl_name_cmp(t1, t2);
}

int
futl_size_cmp(const void *t1, const void *t2) {
	pfutl_file_info_t a, b;
	a = (pfutl_file_info_t) t1;
	b = (pfutl_file_info_t) t2;

	if (a->size > b->size) return -1;
	if (a->size < b->size) return  1;
	return 0;
}

int
futl_size_rcmp(const void *t1, const void *t2) {
	return -futl_size_cmp(t1, t2);
}

pfutl_file_info_t
futl_dir(char *path, char *filter, futl_sort_t sf, futl_order_t so, int *count) {
	DIR *dir;
	struct dirent *de;
	int i, n;
	pfutl_file_info_t out;

	typedef int (*cmp_t)(const void *, const void *);

	cmp_t cmpf = NULL;

	i   = *count =    0;
	n            =  100;

	out =  (pfutl_file_info_t) malloc(n * sizeof(futl_file_info_t));
	
	if (!(dir = opendir(path))) return NULL;
	while ((de = readdir(dir))) {
		if (!filter || (filter && fnmatch(filter, de->d_name, FNM_PATHNAME))) {
			struct stat st;
			if (i == n) {
				n += 100;
				out = (pfutl_file_info_t) realloc(out, n  * (NAME_MAX+1) * sizeof(char));	
			}	
			stat(de->d_name, &st);
			strcpy(out[i].fname, de->d_name);
			out[i].size  = st.st_size;
			out[i].uid	 = st.st_uid;
			out[i].gid	 = st.st_gid;
			out[i].mode	 = st.st_mode;
			out[i].ctime = (tspec_t) st.st_ctim;
			out[i].mtime = (tspec_t) st.st_mtim;
			out[i].atime = (tspec_t) st.st_atim;
		}
	}
	closedir(dir);

	if (sf > futl_SORT_NONE && sf <= futl_SORT_SIZE) {
		switch (sf) {
		case futl_SORT_NONE: 
			/* just for the compiler */
			cmpf = NULL;
			break;
		case futl_SORT_CTIME:
			if (so == futl_ORDER_ASC) 
				cmpf = futl_ctime_cmp;
 
			else 
				cmpf = futl_ctime_rcmp;
			break;
		case futl_SORT_MTIME:
			if (so == futl_ORDER_ASC) 
				cmpf = futl_mtime_cmp;
			else 
				cmpf = futl_mtime_rcmp;
			break;
		case futl_SORT_ATIME:
			if (so == futl_ORDER_ASC) 
				cmpf = futl_atime_cmp;
			else 
				cmpf = futl_atime_rcmp;
			break;
		case futl_SORT_NAME:
			if (so == futl_ORDER_ASC) 
				cmpf = futl_name_cmp;
			else 
				cmpf = futl_name_rcmp;
			break;
		case futl_SORT_SIZE:
			if (so == futl_ORDER_ASC) 
				cmpf = futl_size_cmp;
			else 
				cmpf = futl_size_rcmp;
			break;
		}
		if (cmpf) qsort(out, i, sizeof(futl_file_info_t), cmpf);
	}	

	*count = i;
	return out;
}

#ifdef _test_futl_
#endif
