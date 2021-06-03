#ifndef _sthpo_h_
#define _sthpo_h_

#ifdef __cplusplus
extern "C" {
#endif
int
sthpo_runpool(int tnum, void *(*worker)(void *), void *userdata);

#ifdef __cplusplus
}
#endif

#endif 
