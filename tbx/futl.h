#ifndef _futl_h_
#define _futl_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>
#include <dirent.h>

typedef enum {
	futl_SORT_NONE,
	futl_SORT_CTIME,
	futl_SORT_MTIME,
	futl_SORT_ATIME,
	futl_SORT_NAME,
	futl_SORT_SIZE,
} futl_sort_t; 

typedef enum {
	futl_ORDER_ASC,
	futl_ORDER_DSC,
} futl_order_t;

typedef struct timespec tspec_t;

typedef struct {
	char 		fname[NAME_MAX+1];
	size_t		size;	
	uid_t		uid;
	gid_t		gid;
	mode_t		mode;
	tspec_t		ctime;
	tspec_t     mtime;
	tspec_t		atime;
} futl_file_info_t, *pfutl_file_info_t;

char *        futl_load(char *fname, size_t *size);
int           futl_write(char *fname, char *data, size_t size);

size_t        futl_size(char *fname);

int           futl_exists(char *path);
int           futl_is_file(char *path);
int           futl_is_dir (char *path);
int           futl_is_sock(char *path);
int           futl_is_link(char *path);
int           futl_is_fifo(char *path);

int	          futl_is_rwx(char *path);
int	          futl_is_rw (char *path);
int	          futl_is_rx (char *path);
int	          futl_is_r  (char *path);
int	          futl_is_w  (char *path);
int	          futl_is_x  (char *path);

int           futl_rm(char *fname);
int           futl_cp(char *src, char *dst);
int           futl_mv(char *src, char *dst);

int			  futl_mkdir(char *path);

char *        futl_realpath(char *path);

pfutl_file_info_t 
			  futl_dir(char *path, char *filter, futl_sort_t sf, futl_order_t so, int *count);


unsigned long futl_crc_32(char *fname);
#ifdef __cplusplus
}
#endif

#endif 
