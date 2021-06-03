#ifndef _list_h_
#define _list_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _list_c_ /* avoid redeclaration of plist_t in list.c */
/* 
 * 	Declare plist_t as a private structrure to avoid user dereferenciation:
 */
typedef void *plist_t;
#endif

typedef int  (*list_cmp_f)(void *, void *);
typedef void (*list_cbk_f)(void *);


size_t list_sizeof();

/* 
 * 	Constructor / destructor: 
 */
plist_t list_new();
plist_t list_destroy(plist_t pl);

/*
 *	Standard navigation:
 */
void *list_first(plist_t pl);
void *list_prev(plist_t pl);
void *list_next(plist_t pl);
void *list_last(plist_t pl);

/* 
 * Non list-optimized navigation: 
 */
void *list_moveto_i(plist_t pl, int index);
void *list_moveto_p(plist_t pl, void *data);

/*
 *	Get list info (count, current data, current index):
 */
int   list_count(plist_t pl);
void *list_current(plist_t pl);
int   list_current_index(plist_t pl);

/*
 *	stack / queue simulators:
 */
void *list_push(plist_t pl, void *data);
void *list_pop_first(plist_t pl); /* queue pop simulation (FIFO model) */
void *list_pop_last(plist_t pl);  /* stack pop simulation (LIFO model) */
/* 

 * add / insert / del / change 
 */
void *list_add(plist_t pl, void *data);
void *list_insert(plist_t pl, void *data);
void *list_del(plist_t pl);
void *list_change(plist_t pl, void *pnew);

/* 
 * 	Loop function:
 */
void  list_foreach(plist_t pl, list_cbk_f callback);

/* 
 * 	Set a comparison function: 
 */
int list_compare_set(plist_t pl, list_cmp_f compare);

/* 
 *	comparision and sorting: 
 */
void *list_min(plist_t pl);
void *list_max(plist_t pl);
int   list_sort(plist_t pl);

/* 
 * Debug functions: 
 */
void  list_dump(plist_t pl);
void  list_dump_callback(plist_t pl, list_cbk_f callback);

#if defined(__debug__) && !defined(_test_list_c_)
int list_test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
