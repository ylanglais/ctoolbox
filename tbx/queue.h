#ifndef _queue_h_
#define _queue_h_
#include <tbx/list.h>

typedef plist_t pqueue_t;
typedef void (*queue_cbk_f)(void *);

#ifndef _list_h_

#ifdef __cplusplus
extern "C" {
#endif
/* 
 * 	Constructor / destructor: 
 */
pqueue_t queue_new();
pqueue_t queue_destroy(pqueue_t pl);

/*
 *	Get queue info (count, current data, current index):
 */
int   queue_count(pqueue_t pl);

/*
 *	stack / queue simulators:
 */
void *queue_push(pqueue_t pl, void *data);
void *queue_pop(pqueue_t pl);

/* 
 * 	Loop function:
 */
void  queue_foreach(pqueue_t pl, queue_cbk_f callback);

#ifdef __cplusplus
}
#endif

#else 
#define queue_new 		list_new
#define queue_destroy 	list_destroy
#define queue_count		list_count
#define queue_push		list_push
#define queue_pop		list_pop_first
#define queue_foreach	list_foreach
#endif



#endif
