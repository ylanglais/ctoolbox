#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	30/05/2016  1.1  add tbx std tags
	04/08/2016  1.2  add changelog
*/   

char shmem_MODULE[]  = "Simple shared memory encapsulation";
char shmem_PURPOSE[] = "Allow creation or attachment to an existing shared memory segment";
char shmem_VERSION[] = "1.2";
char shmem_DATEVER[] = "04/08/2016";

#define MAGIC ((char *) 0x0EFFACEE)
#define shmemDEFMODE 0664

typedef struct {
	char   *magic;
	int     id;
	int     refcount;
	int 	lock;
	key_t   key;
	int     mode;
	size_t  size;
	size_t  real;
} shmem_t, *pshmem_t;

pshmem_t 
_shmem_ptr_(char *p) {
	return (pshmem_t) (p - sizeof(shmem_t));
}

int 
is_pshmem(char *p) {
	return p && (p != (char *) -1) && ((_shmem_ptr_(p))->magic == MAGIC);
}

#define _shmem_c_
#include "shmem.h"
#undef _shmem_c_

char *
shmem_attach(int key) {
	int id;
	pshmem_t s;

	if ((id = shmget((key_t) key, 0, 0)) < 0) return NULL;
	if ((s = (pshmem_t) shmat(id, NULL, 0)) == (pshmem_t) -1) 		  return NULL;
	s->refcount++;
	return ((char *) s + sizeof(shmem_t));
}

int
shmem_detach(char *p) {
	pshmem_t s;
	if (is_pshmem(p)) {
		s = _shmem_ptr_(p);
		s->refcount--; 
		return shmdt((void *) s);
	}
	return 1;
}

char *
shmem_destroy(char *p) {
	pshmem_t s;
	if (is_pshmem(p)) {
		int id;	
		s = _shmem_ptr_(p);
		s->refcount--; 
		id = shmem_id(p);
		shmdt((void *) s);
		shmctl(id, IPC_RMID, NULL);
	}
	return NULL;
}

size_t shmem_sizeof() { return sizeof(shmem_t); }

char *
shmem_new(int key, size_t size, int mode) {
	pshmem_t s;
	int id;
	int rc = 1;
	
	if (!mode) mode = shmemDEFMODE;

	if ((id = shmget((key_t) key,  size + sizeof(shmem_t), mode)) < 0) {
		if ((id = shmget((key_t) key, size + sizeof(shmem_t), IPC_CREAT | mode)) < 0) return NULL;
		rc = 0;
	}
	
	if ((s = (pshmem_t) shmat(id, NULL, 0)) == (pshmem_t) -1) return  NULL;
		
	if (rc) s->refcount++;
	else {
		s->magic	= MAGIC;
		s->id       = id;
		s->key      = (key_t) key;
		s->refcount = 1;
		s->lock     = 0;
		s->mode     = mode;
		s->size     = size;	
		s->real     = size + sizeof(shmem_t);
	} 
	return ((char *) s + sizeof(shmem_t));
}

int
shmem_stat(key_t id, struct shmid_ds *stat) {
	if (!stat) return 1;
	if (shmctl(id, IPC_STAT, stat)) return 2;
	return 0;
}

int
shmem_mode_get(char *p) {
	pshmem_t s;
	struct shmid_ds perm;

	if (!is_pshmem(p)) return 1; 
	s = _shmem_ptr_(p);

	if (shmem_stat(s->id, &perm)) return 2;
	return perm.shm_perm.mode;
}

int
shmem_owner_get(char *p) {
	pshmem_t s;
	struct shmid_ds perm;

	if (!is_pshmem(p)) return 1; 
	s = _shmem_ptr_(p);

	if (shmem_stat(s->id, &perm)) return 2;

	return perm.shm_perm.uid;
}

int 
shmem_group_get(char *p) {
	pshmem_t s;
	struct shmid_ds perm;

	if (!is_pshmem(p)) return 1; 
	s = _shmem_ptr_(p);

	if (shmem_stat(s->id, &perm)) return 2;

	return perm.shm_perm.gid;
}

int
shmem_owner_set(char *p, uid_t uid) {
	pshmem_t s;
	struct shmid_ds perm;

	if (!is_pshmem(p)) return 1; 
	s = _shmem_ptr_(p);

	if (shmem_stat(s->id, &perm)) return 2;

	perm.shm_perm.uid = uid;

	if (shmctl(s->id, IPC_SET,  &perm)) return 3;
	
	return 0;
}
	
int
shmem_mode_set(char *p, int mode) {
	pshmem_t s;
	struct shmid_ds perm;

	if (!is_pshmem(p)) return 1; 
	s = _shmem_ptr_(p);

	if (shmem_stat(s->id, &perm)) return 2;

	perm.shm_perm.mode = mode;

	if (shmctl(s->id, IPC_SET,  &perm)) return 3;
	
	return 0;
}

int
shmem_group_set(char *p, gid_t gid) {
	pshmem_t s;
	struct shmid_ds perm;

	if (!is_pshmem(p)) return 1; 
	s = _shmem_ptr_(p);

	if (shmem_stat(s->id, &perm)) return 2;

	perm.shm_perm.gid = gid;

	if (shmctl(s->id, IPC_SET,  &perm)) return 3;
	
	return 0;
}

int
shmem_id(char *p) { 
	if (!is_pshmem(p)) return 0; 
	return _shmem_ptr_(p)->id;
}

int
shmem_refcount(char *p) {
	if (!is_pshmem(p)) return 0; 
	return _shmem_ptr_(p)->refcount;
}

int
shmem_key(char *p) {
	if (!is_pshmem(p)) return 0; 
	return (int) _shmem_ptr_(p)->key;
}

size_t
shmem_size(char *p) {
	if (!is_pshmem(p)) return 0; 
	return _shmem_ptr_(p)->size;
}

size_t
shmem_real(char *p) {
	if (!is_pshmem(p)) return 0; 
	return _shmem_ptr_(p)->real;
}

#ifdef _test_shmem_

#include <stdio.h>
#include <unistd.h>

#define KEY 666
#define SIZE 1024

void
part1() {
	char *s;

	printf("part1: start %d\n", getpid());
	printf("part1: create shm\n");	
	if (!(s = shmem_new(KEY, SIZE, 0))) exit(1);
	printf("part1: s        = %x\n", s);
	printf("part1: id       = %d\n", shmem_id(s));
	printf("part1: refcount = %d\n", shmem_refcount(s));
	printf("part1: key      = %d\n", shmem_key(s));
	printf("part1: size     = %d\n", shmem_size(s));
	printf("part1: real     = %d\n", shmem_real(s));
	printf("part1: set double\n");
	*(double *) s = 1234.56789;
	printf("part1: double   = %f\n", *(double *) s);
	printf("part1: sleep(3)\n");
	sleep(3);
	printf("part1: destroy shm\n");
	shmem_destroy(s);
	printf("part1: end\n");
}

void 
part2() {
	char *s;

	printf("part2: start\n");
	printf("part2: sleep(2)\n");
	sleep(2);
	printf("part2: attach shm\n");	
	if (!(s = shmem_attach(KEY))) exit(2);
	printf("part2: s        = %x\n", s);
	printf("part2: id       = %d\n", shmem_id(s));
	printf("part2: refcount = %d\n", shmem_refcount(s));
	printf("part2: key      = %d\n", shmem_key(s));
	printf("part2: size     = %d\n", shmem_size(s));
	printf("part2: real     = %d\n", shmem_real(s));
	printf("part2: double   = %f\n", *(double *) s);
	printf("part2: detach shm\n");
	shmem_detach(s);
	printf("part2: end\n");
}

int
main(void) {
	int p;

	p = fork(); 

    if (p == 0) { 
        part2();
		return 0;
    } else if (p < 0) {
        printf("fork failed (p = %d)\n", p);
        exit(22);
    }
	part1();
	return 0;
}

#endif
