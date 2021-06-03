#ifndef _map_h_
#define _map_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Flag definition */
#define mapNORMAL 0		/* Read Write        */
#define mapRDONLY 1		/* Read Only         */
#define mapCREATE 2		/* Read Write Create */

#ifndef _map_c_
typedef void *pmap_t;
#endif

/* Map a file to a pointer: */

size_t map_sizeof();

/* Constructor and destructor: */
/* map a file to a pointer in 'normal' memory (heap): */
pmap_t  map_new(char *filename, int flag);
/* unmap the file mapped by map_new: */
pmap_t  map_destroy(pmap_t map); 

/* Acces to map internal:      */
char   *map_data_get(pmap_t map);
size_t  map_size_get(pmap_t map);
int     map_flag_get(pmap_t map);

/* Modifying data: */
int map_size_set(pmap_t map, size_t size);

/* insert size bytes at p from data: */
int     map_insert_at(pmap_t map, char *p, char *data, size_t size);

/* delete size bytes at p: */
int     map_delete_at(pmap_t map, char *p, size_t size);

/* replace oldsize byte from p by newsize byte from data: */
int 	map_replace_at(pmap_t map, char *p, char *data, size_t oldsize, size_t newsize);

#ifdef __cplusplus
}
#endif

#endif
