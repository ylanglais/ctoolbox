#ifndef _fsf_h_
#define _fsf_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _fsf_c_
typedef void *pfsf_t;
#endif

pfsf_t fsf_destroy(pfsf_t s);
pfsf_t fsf_new(char *specfile, char *datafile);

int    fsf_lines(pfsf_t  s);
int    fsf_fields(pfsf_t s);

char * fsf_line(pfsf_t s, int i);
char * fsf_field_get(pfsf_t s, int i, int j);
char * fsf_field_named_get(pfsf_t s, int i, char *name);

#ifdef __cplusplus
}
#endif

#endif

