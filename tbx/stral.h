#ifndef _stral_h_
#define _stral_h_


#ifdef __cplusplus
extern "C" {
#endif


#ifndef _stral_c_
typedef void *pstral_t;
#endif

size_t   stral_sizeof();
pstral_t stral_new();
pstral_t stral_destroy(pstral_t s);
char *   stral_alloc(pstral_t s, size_t size);
char *   stral_dup(pstral_t s, char *str);

#ifdef __cplusplus
}
#endif

#endif
