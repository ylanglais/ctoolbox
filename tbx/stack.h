#ifndef _stac_tk_h_
#define _stack_h_
#include <tbx/list.h>

typedef plist_t pstack_t;
typedef void (*stack_cbk_f)(void *);

#ifndef _list_h_

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * 	Constructor / destructor: 
 */
pstack_t stack_new();
pstack_t stack_destroy(pstack_t st);

/*
 *	Get stack info (count, current data, current index):
 */
int   stack_count(pstack_t st);

/*
 *	stack / stack simulators:
 */
void *stack_push(pstack_t st, void *data);
void *stack_pop(pstack_t st);

/* 
 * 	Loop function:
 */
void  stack_foreach(pstack_t st, stack_cbk_f callback);

#ifdef __cplusplus
}
#endif

#else 
#define stack_new 		list_new
#define stack_destroy 	list_destroy
#define stack_count		list_count
#define stack_push		list_push
#define stack_pop		list_pop_last
#define stack_foreach	list_foreach
#endif

#endif
