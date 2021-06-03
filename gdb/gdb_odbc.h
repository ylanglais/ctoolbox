#ifndef _od_h_
#define _od_h_

#ifndef _od_c_
typedef struct _od_t *pod_t;
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* constructor: */
pod_t od_new(char *datasrv, char *database, char *login, char *passwd);
/* destructor: */
pod_t od_destroy(pod_t od);

/* execute a statement: */
int    od_cmd(pod_t od, char *statement);

/* fetch next row: */
int    od_next_row(pod_t od);

/* Get column (value or new allocated pointer to char): */
void * od_get_col(pod_t od, int colnum);
void * od_get_col_by_name(pod_t od, char *name); /* optional */

/* other optional stuff: */
int    od_get_col_size(pod_t od, int colnum);
char * od_get_col_name(pod_t od, int colnum);
#ifdef __cplusplus
}
#endif
#endif 
