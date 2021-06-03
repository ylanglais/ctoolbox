#ifndef _shmem_h_
#define _shmem_h_

#include <stdlib.h>
#ifndef _shmem_c_
#include <sys/shm.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t  shmem_sizeof();

char   *shmem_new(int key, size_t size, int mode);
char   *shmem_destroy(char *p);

char   *shmem_attach(int key);
int     shmem_detach(char *p);

int 	shmem_id(char *p);
int 	shmem_refcount(char *p);
int  	shmem_key(char *p);
size_t 	shmem_size(char *p);
size_t  shmem_real(char *p);

int shmem_stat(key_t id, struct shmid_ds *stat);

int shmem_mode_get(char *p);
int shmem_owner_get(char *p);
int shmem_group_get(char *p);

int shmem_mode_set(char *p, int mode);
int shmem_owner_set(char *p, uid_t uid);
int shmem_group_set(char *p, gid_t gid);

#ifdef __cplusplus
}
#endif

#endif
