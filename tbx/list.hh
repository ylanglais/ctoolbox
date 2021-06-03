#ifndef _list_hh_
#define _list_hh_

/* 
 * 	Declare plist_t as a private structrure to avoid user dereferenciation:
 */

#include <list.h>

class List {
private:
	plist_t list;

public:
	List()  { list = list_new(); }
	List(plist_t l) { list = l;  }

	~List() { list = list_desroy(list); }

	/*
	 *	Standard navigation:
	 */

	void * first() { return list_first(list); }
	void * prev() { return list_first(list); }
	void * next() { return list_first(list); }
	void * last() { return list_first(list); }

	/* 
	 * Non list-optimized navigation: 
	 */

	void * operator [] (int i)      { return list_moveto_i(list, i);    }
	void * operator [] (void *data) { return list_moveto_p(list, data); }

	/*
	 *	Get list info (count, current data, current index):
	 */

	int    count()   { return list_count(list); }
	void * current() { return list_current(list); }
	int    index()   { return list_current_index(list); }
	
	/*
	 *	stack / queue simulators:
	 */
	
	void * push(void *data) { return list_push(list, data); }
	/* queue pop simulation (FIFO model) */
	void * pop_first()      { return list_pop_first(list); }
	/* stack pop simulation (LIFO model) */
	void * pop_last()       { return list_pop_last(list); }

	/* 
	 * add / insert / del / change 
	 */
	void * add(void *data)    { return *list_add(list, data); }
	void * insert(void *data) { return *list_insert(list, data); }
	void * del() { return list_del(list); }
	void * change(void *data) { return *list_change(list, data); }

	/* 
	 * 	Loop function:
	 */
	
	void   foreach(list_cbk_f callback) { list_foreach(list, callback); }

	/* 
	 * 	Set a comparison function: 
	 */
	int    compare_set(list_cmp_f compare) { return list_compare_set(list, compare); }

	/* 
	 *	comparision and sorting: 
	 */

	void * min()  { return list_min(list);  }
	void * max()  { return list_max(list);  }
	int    sort() { return list_sort(list); }

	/* 
	 * Debug functions: 
	 */
	
	void dump() { list_dump(list); }
	void dump_callback(list_cbk_f callback) { list_dump_callback(list, callback); }
}

#endif
