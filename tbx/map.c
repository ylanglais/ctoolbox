
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		16/06/2006	1.0 creation
    
*/   

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "err.h"

extern int errno;
 
char map_MODULE[]  = "memory mapped based file access";
char map_PURPOSE[] = "Fast IO using memory mapped based file access";
char map_VERSION[] = "1.0";
char map_DATEVER[] = "16/06/2006";

typedef struct {
	size_t  size;
	int 	fid;
	char   *data;	
	int 	flag;
	int 	open_flag;
	int 	mmap_flag;
	int 	lock;
} map_t, *pmap_t;

#define _map_c_
#include "map.h"
#undef  _map_c_

static void
_map_lock_(pmap_t map) {
	if (map) {
		while (map->lock);
		map->lock = 0;
	}
}

static void 
_map_unlock_(pmap_t map) {
	if (map->lock) map->lock = 0;
}

static int
_map_resize_(pmap_t map, int delta) {
	char *p;
	if (!map || !map->data)      return 1;
	if (map->flag == mapRDONLY)  return 55;
	ftruncate(map->fid, map->size + delta);
	if (!(p = mmap(map->data, map->size + delta, map->mmap_flag, MAP_FIXED | MAP_SHARED, map->fid, 0))) 
		                         return 2;
	if (p != map->data)          return 3;
/*	printf("oldsize = %d\n", map->size); */
	map->size += delta;
/*	printf("newsize = %d\n", map->size); */
	return 0;
}

pmap_t
map_destroy(pmap_t map) {
	if (map) {
		if (map->fid > 0) {
			msync(map->data, map->size, MS_SYNC);
			munmap(map->data, map->size);
			close(map->fid);
		}
		memset(map, 0, sizeof(map_t));
		free(map); map = NULL;
	}
	return NULL;
}

size_t map_sizeof() { return sizeof(map_t); }

pmap_t map_new(char *filename, int flag) {
	pmap_t map;
	struct stat stats;

	memset(&stats, 0, sizeof(struct stat));

	if (stat(filename, &stats) && flag != mapCREATE) return NULL;

	if (!(map = (pmap_t) malloc(sizeof(map_t)))) return NULL;
	memset(map, 0, sizeof(map_t));

	if (stats.st_size > 0) 
		map->size = stats.st_size;
	else 
		map->size = 1;

	map->flag = flag;

	switch (flag) {
	case mapNORMAL:
		map->open_flag = O_RDWR   | O_SYNC;
		break;
	case mapRDONLY:
	    map->open_flag = O_RDONLY | O_SYNC;
		break;
	case mapCREATE:
		map->open_flag = O_CREAT  | O_RDWR | O_TRUNC | O_SYNC;
	}

	if ((map->fid = open(filename, map->open_flag, 0640)) < 0) {
		return map_destroy(map); 
	}

	if (flag == mapRDONLY) 
		map->mmap_flag = PROT_READ;
	else
		map->mmap_flag = PROT_READ | PROT_WRITE;
	
	if (!(map->data = mmap(NULL, map->size, map->mmap_flag, MAP_SHARED, map->fid, 0))) {
		return map_destroy(map);
	}

	if (map->data == MAP_FAILED) {
		err_error("cannot create map of file %s (%s)", filename, strerror(errno));
		return map_destroy(map);
	}
	return map;
}

int map_size_set(pmap_t map, size_t size) {
	return _map_resize_(map, size - map->size);
}

size_t
map_size_get(pmap_t map) {
	if (map) return map->size;
	return 0;
}

char *
map_data_get(pmap_t map) {
	if (!map || !map->data) return NULL;
	if (map)                return map->data;
	return NULL;
}

int
map_flag_get(pmap_t map) {
	if (map) return map->flag;
	return -1;
}

int
map_insert_at(pmap_t map, char *p, char *data, size_t size) {
	if (!map || !map->data)     return 1;
	if (map->flag == mapRDONLY) return 55;
	_map_lock_(map);

	if (_map_resize_(map, size)) {
		_map_unlock_(map);
		return 2;
	}
	memmove(p + size, p, (size_t) ((long) map->size - ((long) p - (long) map->data) - size));
	memcpy(p, data, size);
	/* msync(map->data, map->size, MS_SYNC); */
	_map_unlock_(map);
	return 0;
}

int
map_delete_at(pmap_t map, char *p, size_t size) {
	if (!map || !map->data)     return 1;
	if (map->flag == mapRDONLY) return 55;

	_map_lock_(map);
	if (size > (size_t) ((long) map->size - (long) p + (long) map->data)) 
		size = (size_t) ((long) map->size - (long) p + (long) map->data);

	memmove(p, p  + size, (size_t) ((long) map->size - (long) p + (long) map->data - (long) size));
	if (_map_resize_(map, -size)) {
		_map_unlock_(map);
		return 2;
	}
	/* msync(map->data, map->size, MS_SYNC); */
	_map_unlock_(map);
	return 0;	
}

int
map_replace_at(pmap_t map, char *p, char *data, size_t oldsize, size_t newsize) {
	if (!map || !map->data)    		    return  1;
	if (map->flag == mapRDONLY)         return 55;
	if (map_delete_at(map, p, oldsize)) return  2;
	if (map_insert_at(map, p, data, newsize)) return 3;
	return 0;
}

#ifdef _test_map_

#include <stdio.h>
#include <errno.h>

static char orig[] = "012345678901234567890";
static char file[] = "map_test.data";

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
			close(f);
		}
	}
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
	pmap_t map;
	char *str1="hahaha";

	echo_cmd(write_orig());
	echo_cmd(cat_orig());
	echo_cmd(map = map_new(file, 0));
	echo_cmd(p   = map_data_get(map));
	echo_cmd(map_insert_at(map, p, str1, strlen(str1)));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());	
	echo_cmd(map_delete_at(map, p, 6)); 
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	echo_cmd(map_replace_at(map, p+10, "iiiiii", 5, 6));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	echo_cmd(map_insert_at(map, p, "ohohoh", 6));
	echo_cmd(map_destroy(map));
	echo_cmd(size_orig());
	echo_cmd(cat_orig());
	return 0;
}
#endif 
