#ifndef _b64_h_
#define _b64_h_

#ifdef __cplusplus
extern "C" {
#endif

unsigned char * b64_encode(const unsigned char *src, size_t len, size_t *out_len); 
unsigned char * b64_decode(const unsigned char *src, size_t len, size_t * out_len);

#ifdef __cplusplus
}
#endif

#endif 
