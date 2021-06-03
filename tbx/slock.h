#ifndef _slock_h_
#define _slock_h_

#ifdef __cplusplus
extern "C" {
#endif


int slock_new(char *fname);
int slock_lock(int id);
int slock_unlock(int id);
int slock_semval(int id);
int slock_destroy(int id);

#ifdef __cplusplus
}
#endif

#endif
