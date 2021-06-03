#ifndef _byar_h_
#define _byar_h_

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef _byar_c_ 
typedef void *pbyar_t;
#endif

pbyar_t byar_new();
pbyar_t byar_destroy(pbyar_t b);
pbyar_t byar_new_from_data(char *data, size_t len);

int     byar_data_get(pbyar_t b, char **data, size_t *size);
int   	byar_push(pbyar_t b, int len, char *data);
char *	byar_pop(pbyar_t b, int len, char *data);
int     byar_rewind(pbyar_t b);

#ifdef __cplusplus
}
#endif 

#endif
