#ifndef _mlock_h_
#define _mlock_h_

#ifdef __cplusplus
extern "C" {
#endif

int mlock_new(int k, int nlocks);
int mlock_lock(int id, int ilock);
int mlock_unlock(int id, int ilock);
int mlock_val(int id, int ilock);
int mlock_destroy(int id);

#ifdef __cplusplus
}
#endif

#endif
