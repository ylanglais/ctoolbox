#ifndef __Btree_h__
#define __Btree_h__

#include <btree.h>

class Btree {
private:
	pbtree_t pb;

public:
	Btree(comp_t comp)        { pb = btree_new(pb); };
	~Btree()                  { pb = btree_destroy(pb); };
	
	void *store(void *data)   { return btree_store(pb, data); };
	void *get(void *data)     { return btree_get(pb, data); };
 
	void *go_to(void *data)   { return btree_goto(pb, data); };
	void *leftmost()          { return btree_leftmost(pb); }; 
	void *left()              { return btree_left(pb); };
	void *right()             { return btree_right(pb); };
	void *parent()            { return btree_parent(pb); };
	void *prev()              { return btree_prev(pb); };
	void *next()              { return btree_next(pb); };
	void *save()              { return btree_save(pb); };
	void *restore(void *mark) { return btree_restore(pb, void *mark); };
	
	void  foreach_raw(void (*userf)(void *)) { btree_foreach_raw(pb, userf); };
	void  foreach(void (*userf)(void *))     { btree_foreach(pb, userf);     };	
};

#endif
