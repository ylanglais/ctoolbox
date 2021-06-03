
ifdef __reentrant__
#include <pthread.h> 
#define tree_lock(x)   pthread_mutex_lock((x)->mp)
#define tree_unlock(x) pthread_mutex_unlock((x)->mp)
#else 
#define tree_lock(x) 
#define tree_unlock(x) 
#endif

char tree_NAME[]    = "Tree management";
char tree_VERSION[] = "0.1.0";
char tree_DATEVER[] = "31/05/2006";

/***************************
           NODE
 ***************************/

typedef struct _node_t {
	void 		    *data;
	struct _node_t  *parent;
	int				 nchildren;
	struct _node_t **chidren;
} node_t, *pnode_t;

/* Constructor/Destructor: */

static pnode_t 
node_destroy(pnode_t a) {
	
}

static pnode_t 
node_new() {
	pnode_t a;

}

/* set/get: */

static void *
node_data_get(pnode_t a) {
}

static void *
node_data_set(pnode_t a, void *data) {
}

static pnode_t 
node_parent_get(pnode_t a) {
}

static pnode_t
node_parent_set(pnode_t a, pnode_t newparent) {
}

static pnode_t 
node_child_add(pnode_t a, void *data) {
}

static pnode_t 
pnode_child_del(pnode_t a, int childindex) {
}

/***************************
           TREE
 ***************************/

typedef struct _tree_t {
	pnode_t current;
	pnode_t root;
	#ifdef __reentrant__
	pthread_mutex_t  *mp;
	#endif	
} tree_t, *ptree_t;

/* */

ptree_t
tree_destroy(ptree_t t) {
}

ptree_t 
tree_new(ptree_t t) {
}

/* navigation: */
int 

	if (!t)           return -1;
	if (!t->current)  return -2;
	return t->current->nchildren;
}

void *
tree_current(ptree_t t) {
	if (!t) return NULL;
	return (void *) t->current
}

ptree_t 
tree_subtree_get(ptree_t t) {
	ptree_t st;
	if (!t || !t->current)  return NULL;
	if (!(st = tree_new())) return NULL;
	st->root = st->current = t->current;
	return st;
}

void *
tree_root(ptree_t t) {
	if (!t) return NULL;
	return t->root;
}

void *
tree_down(ptree_t t) {
	if (!t) return NULL;
	if (!t->current || !t->current->parent) return NULL;
	t->current = t->current->parent;
}

void * tree_parent(ptree_t t) { return tree_down(t); }

void *
tree_child_foreach(ptree_t t, ) {
}

/* add/delete: */
void *
tree_child_add(ptree_t t, void *data) {
}

void *
tree_child_del(ptree_t t) {
}

/* reparent: */
void *
tree_child_reparent(ptree_t dest, ptree_t subtree) {
}

/* tree foreach: */

