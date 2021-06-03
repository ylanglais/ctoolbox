#ifndef _hash_h_
#define _hash_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#ifndef _hash_c_
typedef struct hash_t *phash_t;
typedef int (f_hashcode_t)(phash_t, char *);
#endif
	
size_t  hash__sizeof();
phash_t hash_new();
phash_t hash_new_sized(size_t size);
phash_t hash_new_fcode(f_hashcode_t f_code);
phash_t hash_new_set(size_t size, f_hashcode_t f_code);

phash_t hash_destroy(phash_t ph);
phash_t hash_set_code_function(phash_t ph, f_hashcode_t f_code);
phash_t hash_resize(phash_t ph, size_t size);
phash_t hash_rehash(phash_t ph, int size);

int	    hash_used(phash_t ph) ;
int	    hash_size(phash_t ph) ;

int     hash_decimal_code(phash_t ph, char *name);
int     hash_classic_code(phash_t ph, char *name);

int     hash_insert(phash_t ph, char *name, void *pdata);
void *  hash_retrieve(phash_t ph, char *name);
void *  hash_delete(phash_t ph, char *name);


#ifdef __HASH_STATISTICS__
void    hash_stats(phash_t ph);
void    hash_stats_reset(phash_t ph);
#endif
void    hash_dump(phash_t ph);

#ifdef __cplusplus
}
#endif

#endif 
