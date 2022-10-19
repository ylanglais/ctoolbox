#ifndef _fifo_h_
#define _fifo_h_
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _fifo_c_
typedef void *pfifo_t;
#endif

typedef void (*elmn2str_f)(void *);

pfifo_t fifo_new();
pfifo_t fifo_destroy(pfifo_t f);

void *  fifo_push(pfifo_t f, void *data);
void *  fifo_pop(pfifo_t f);

int 	fifo_is_empty(pfifo_t f);
void    fifo_dump(pfifo_t f, elmn2str_f elmn2str);

#ifdef __cplusplus
}
#endif
#endif
