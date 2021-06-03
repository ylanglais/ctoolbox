#ifndef _dyna_h_
#define _dyna_h_
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    
*/   

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _dyna_c_
typedef void *pdyna_t;
#endif

size_t   dyna_sizeof();

pdyna_t  dyna_destroy(pdyna_t da);
pdyna_t  dyna_new(size_t unit, size_t grain);
char    *dyna_alloc(pdyna_t da);
char    *dyna_add(pdyna_t da, char *pdata);
char    *dyna_insert(pdyna_t da, int index, char *pdata);
int      dyna_delete(pdyna_t da, int index);
char    *dyna_push(pdyna_t da, char *pdata);
char    *dyna_pop_first(pdyna_t da);
char    *dyna_pop_last(pdyna_t da);
int      dyna_delete_ptr(pdyna_t da, void *ptr);
char    *dyna_get(pdyna_t da, int i);
char    *dyna_set(pdyna_t da, int i, char *pdata);

int dyna_count(pdyna_t da);

int dyna_used(pdyna_t da);
int dyna_allocated(pdyna_t da);
int dyna_available(pdyna_t da);
int dyna_grain(pdyna_t da);
int dyna_unit(pdyna_t da);
int dyna_pages(pdyna_t da);
void *dyna_data_ptr(pdyna_t da, int i);

#ifdef __cplusplus
}
#endif

#endif
