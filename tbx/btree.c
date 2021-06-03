#ifdef __REENTRANT__
#include <pthread.h> 
#define btree_lock(x)   pthread_mutex_lock((x)->mp)
#define btree_unlock(x) pthread_mutex_unlock((x)->mp)
#else 
#define btree_lock(x) 
#define btree_unlock(x) 
#endif

#include <strings.h>
#include <string.h>

#include "mem.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		19/09/2001	1.0 	Initial version	
		31/05/2018  1.1     Fix mutex alloc/free & deadlock

*/

char btree_MODULE[]  = "Binary tree management";
char btree_PURPOSE[] = "Sorting with btrees";
char btree_VERSION[] = "1.1";
char btree_DATEVER[] = "31/05/2018";

typedef int (*comp_t)(void *, void *);

typedef struct _node_t_ {
	void *data;
	struct _node_t_ *parent, *left, *right;
} node_t, *pnode_t;

typedef struct _btree_t_ {
	pnode_t root;
	pnode_t curr;
	comp_t  comp;
    #ifdef __REENTRANT__
	pthread_mutex_t  *mp;
	#endif
} btree_t, *pbtree_t;

#define __btree_c__
#include "btree.h"
#undef __btree_c__

static pnode_t _node_new_(void *data) {
	pnode_t p;
	if (!(p = (pnode_t) mem_zmalloc(sizeof(node_t)))) return NULL;
	p->data = data;
	p->left = p->right = p->parent = NULL;
	return p;
}

size_t btree_sizeof() { return sizeof(btree_t); }

pbtree_t btree_new(comp_t comp) {
	pbtree_t btree;
	if (!(btree = (pbtree_t) mem_zmalloc(sizeof(btree_t)))) return NULL;
	btree->root = btree->curr = NULL;
	btree->comp = comp;
	#ifdef __REENTRANT__
	btree->mp = (pthread_mutex_t *) mem_zmalloc(sizeof(pthread_mutex_t));
	#endif
	return btree;
}

static pbtree_t _btree_sub_destroy_(pnode_t pn) {
	if (!pn) return NULL;
	if (pn->left) _btree_sub_destroy_(pn->left);
	if (pn->right) _btree_sub_destroy_(pn->right);
	memset(pn, 0, sizeof(node_t));
	mem_free(pn);
	return NULL;
}

pbtree_t btree_destroy(pbtree_t pb) {
	if (!pb) return NULL;
	btree_lock(pb);
	_btree_sub_destroy_(pb->root);
	btree_unlock(pb);

	#ifdef __REENTRANT__
	pthread_mutex_destroy(pb->mp);
	mem_free(pb->mp);
	#endif

	memset(pb, 0, sizeof(btree_t));

	return mem_free(pb);
}

void *btree_save(pbtree_t pb) {
	if (!pb) return NULL;
	return (void *) pb->curr;
}

void *btree_restore(pbtree_t pb, void *mark) {
	if (!pb) return NULL;
	return (void *) (pb->curr = (pnode_t) mark);
}

void *btree_store(pbtree_t pb, void *data) {
	pnode_t node, nnew;
	if (!pb) return NULL;
	btree_lock(pb);
	nnew  = _node_new_(data);
	if (!pb->root) {
		pb->curr = pb->root = nnew;
		btree_unlock(pb);
		return pb->curr;
	}
	node = pb->root;
	while (node && nnew) {
		int cmp;
		cmp = pb->comp(node->data, data);
		nnew->parent = node;
		if (cmp < 0) {
			if (node->left) node = node->left;
			else {
				btree_unlock(pb);
				return pb->curr = node->left = nnew;
			}
		} else if (cmp > 0) {
			if (node->right) node = node->right;
			else {
				btree_unlock(pb);
				return pb->curr = node->right = nnew;
			}
		} else {
			nnew = mem_free(nnew);
		}
	}
	btree_unlock(pb);
	return NULL;
}

void *btree_get(pbtree_t pb, void *data) {
	if (pb && pb->curr) return pb->curr->data;
	return NULL;
}

void *btree_goto(register pbtree_t pb, register void *data) {
	register int c = 0;
	register pnode_t node;
	if (!pb || !pb->root) return NULL;
	btree_lock(pb);

	node = pb->root; 
	while (node && node->data && (c = pb->comp(data, node->data))) {
		if (c < 0) node = node->left;
		else if (c > 0) node = node->right;
	}
	btree_unlock(pb);
	if (c) return NULL;
	return node->data;
}

void *btree_leftmost(pbtree_t pb) {
	pnode_t node;
	if (!pb || !pb->root) return NULL;
	btree_lock(pb);
	node = pb->root;
	while (node && node->left) node = node->left;
	pb->curr = node;
	btree_unlock(pb);
	return node->data;
}

void *btree_rightmost(pbtree_t pb) {
	pnode_t node;
	if (!pb || !pb->root) return NULL;
	btree_lock(pb);
	node = pb->root;
	while (node && node->right) node = node->right;
	pb->curr = node;
	btree_unlock(pb);
	return node->data;
}

void *btree_left(pbtree_t pb) {
	if (!pb || !pb->curr || !pb->curr->left) return NULL; 
	pb->curr = pb->curr->left;
	return pb->curr->data;
}

void *btree_right(pbtree_t pb) {
	if (!pb || !pb->curr || !pb->curr->right) return NULL; 
	pb->curr = pb->curr->right;
	return pb->curr->data;
}

void *btree_parent(pbtree_t pb) {
	if (!pb || !pb->curr || !pb->curr->parent) return NULL; 
	pb->curr = pb->curr->parent;
	return pb->curr->data;
}

void *btree_prev(pbtree_t pb) {
	return btree_left(pb);
}

void *btree_next(pbtree_t pb) {
	return btree_right(pb);
}

static void _node_foreach_raw_(pnode_t node, void *userdata, void (*userf)(void *, void *)) {
	if (!node) return;
	userf(userdata, node->data);
	if (node->left)  _node_foreach_raw_(node->left,  userdata, userf);
	if (node->right) _node_foreach_raw_(node->right, userdata, userf); 
}

void btree_foreach_raw(pbtree_t pb, void *userdata, void (*userf)(void *, void *)) {
	if (pb) _node_foreach_raw_(pb->root, userdata, userf);
}

static void _node_foreach_(pnode_t node, void *userdata, void (*userf)(void *, void *)) {
	if (!node) return;
	if (node->left)  _node_foreach_(node->left, userdata, userf);
	userf(userdata, node->data);
	if (node->right) _node_foreach_(node->right, userdata, userf); 
}

static void _node_foreach_desc_(pnode_t node, void *userdata, void (*userf)(void *, void *)) {
	if (!node) return;
	if (node->right)  _node_foreach_desc_(node->right, userdata, userf);
	userf(userdata, node->data);
	if (node->left) _node_foreach_desc_(node->left, userdata, userf); 
}

void btree_foreach(pbtree_t pb, void *userdata, void (*userf)(void *, void *)) {
	if (pb) _node_foreach_(pb->root, userdata, userf);
}

void btree_foreach_desc(pbtree_t pb, void *userdata, void (*userf)(void *, void *)) {
	if (pb) _node_foreach_desc_(pb->root, userdata, userf);
}

#ifdef _test_btree_ 
#include <stdio.h>
void _node_print_raw_(pnode_t node, void *data, void (*print)(void *, void *)) {
	printf(" (");
	print(NULL, node->data);
	if (node->left)  _node_print_raw_(node->left, NULL, print);
	if (node->right) _node_print_raw_(node->right, NULL, print); 
	printf(")");
}

void btree_print_raw(pbtree_t pb, void *data, void (*print)(void *, void *)) {
	if (pb && pb->root) {
		printf("btree raw print:\n-->");
		_node_print_raw_(pb->root, NULL, print);
		printf("\n");
	}
}

void int_print(void *foo, void *d) {
	printf("%d ", (long) d);
}

int int_cmp(void *p0, void *p1) {
	 if (p0 < p1) return 1;
	 if (p0 > p1) return -1;
	 return 0;
}

#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }

int main(void) {
	pbtree_t btree = NULL;
	echo_cmd(btree = btree_new(int_cmp));

	echo_cmd(btree_store(btree, (void *) 10));
	echo_cmd(btree_store(btree, (void *) 1));
	echo_cmd(btree_store(btree, (void *) 9));
	echo_cmd(btree_store(btree, (void *) 2));
	echo_cmd(btree_store(btree, (void *) 6));
	echo_cmd(btree_store(btree, (void *) 4));
	echo_cmd(btree_store(btree, (void *) 5));
	echo_cmd(btree_store(btree, (void *) 3));
	echo_cmd(btree_store(btree, (void *) 7));
	echo_cmd(btree_store(btree, (void *) 8));
	echo_cmd(btree_store(btree, (void *) 0));
	
	
	
	printf("btree structure verification:\n");
	echo_cmd(btree_print_raw(btree, NULL, int_print));

	printf("btree raw print (as inserted) :\n");
	echo_cmd(btree_foreach_raw(btree, NULL, int_print));
	printf("\n");

	printf("btree sorted print:\n");
	echo_cmd(btree_foreach(btree, NULL, int_print));
	printf("\n");

	printf("btree sorted descending print:\n");
	echo_cmd(btree_foreach_desc(btree, NULL, int_print));
	printf("\n");
	
	
	echo_cmd(btree_destroy(btree));
	return 0;	
}

#endif
