
#ifndef _fsf_h_
#define _fsf_h_
#ifndef _fsf_c_
typedef void *pfsf_t;
#endif

pfsf_t fsf_destroy(pfsf_t s);
pfsf_t fsf_new(char *specfile, char *datafile);

char * fsf_key(pfsf_t s, int i);

int    fsf_line_by_key(pfsf_t s, char *key);

char * fsf_field_get(pfsf_t s, int i, int j);
char * fsf_field_by_name_get(pfsf_t s, int i, char *name);

#endif

