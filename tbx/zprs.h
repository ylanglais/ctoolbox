#ifndef _zprs_h_
#define _zprs_h_

#ifdef __cplusplus
extern "C" {
#endif

char * zprs_compress(char *data, size_t *size);
char * zprs_compress_level(char *data, size_t *size, int level);
char * zprs_uncompress(char * data, size_t *size);

#ifdef __cplusplus
}
#endif

#endif
