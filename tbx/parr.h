#ifndef _parr_h_
#define _parr_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _parr_c_ 
typedef void *pslot_t;
typedef void *ppage_t;
typedef void *pparr_t;
#endif

pparr_t parr_new(char *path, char *filespec, size_t unit, size_t grain);
pparr_t parr_destroy(pparr_t arr);
int		parr_clear(parr_filename_t fname);

char * 	parr_alloc(pparr_t arr);
char *	parr_add(pparr_t arr, char *data);
char * 	parr_insert(pparr_t arr, int index, char *data);
int     parr_delete(pparr_t arr, int index);
char *	parr_push(pparr_t arr, char *data);
char * 	parr_pop_first(pparr_t arr); 
char * 	parr_pop_last(pparr_t arr);
char * 	parr_get(pparr_t arr, int i);
char * 	parr_set(pparr_t arr, int i, char *data);
int 	parr_count(pparr_t arr);

#ifdef __cplusplus
}
#endif

#endif
