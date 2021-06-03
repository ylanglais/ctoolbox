#ifndef _qdxml_h_
#define _qdxml_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef void (*start_f)(char *, char *);
typedef void (*stop_f )(char *, char *);
typedef void (*value_f)(char *, char *);

int qdxml_parse(char *data, char *buf, size_t size, start_f start, stop_f stop, value_f value);

#ifdef __cplusplus
};
#endif

#endif
