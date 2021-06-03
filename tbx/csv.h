#ifndef _csv_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _csv_c_
typedef void *pcsv_t;
#endif

pcsv_t  csv_destroy(pcsv_t c);
pcsv_t  csv_new(char *filename, char separator);

int     csv_lines(pcsv_t c);
int     csv_fields(pcsv_t c);

char *  csv_line(pcsv_t c, int i);
char *  csv_field_get(pcsv_t c, int i, int j);

#ifdef __cplusplus
}
#endif

#endif
