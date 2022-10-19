#ifndef _lifo_h_
#define _lifo_h_
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _lifo_c_
typedef void *plifo_t;
#endif

typedef void (*elmn2str_f)(void *);

plifo_t lifo_new();
plifo_t lifo_destroy(plifo_t f);

void *  lifo_push(plifo_t f, void *data);
void *  lifo_pop(plifo_t f);

int 	lifo_is_empty(plifo_t f);
void    lifo_dump(plifo_t f, elmn2str_f elmn2str);

#ifdef __cplusplus
}
#endif
#endif
