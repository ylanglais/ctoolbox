/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		31/05/2006	1.0	Creation
    
*/   

#include <stdlib.h>
#include <strings.h>
#include <string.h>

char tree_MODULE[]  = "Tree management";
char tree_PURPOSE[] = "Tree manageemnt";
char tree_VERSION[] = "1.0";
char tree_DATEVER[] = "31/05/2006";

/***************************
           NODE
 ***************************/

typedef struct _node_t {
	void 		    *data;
	struct _node_t  *parent;
	int				 curr_child;
	int				 nchildren;
	struct _node_t **children;
} node_t, *pnode_t;

/* Constructor/Destructor: */

static pnode_t 
node_destroy(pnode_t a) {
	int i;
	if (a) {
		for (i = 0; i < a->nchildren; i++) 
			node_destroy(a->children[i]);
		if (a->children) 
			free(a->children);
		free(a);
	}
	return NULL;
} 
			
static pnode_t 
node_new() {
	pnode_t n;
	if (!(n = malloc(sizeof(node_t)))) return NULL;
	n->data     = NULL;
	n->parent   = NULL;
	n->children = NULL;
	n->curr_child = n->nchildren = 0;
	return n;
}

/***************************
           TREE
 ***************************/

typedef struct _tree_t {
	pnode_t curr;
	pnode_t root;
} tree_t, *ptree_t;

ptree_t
tree_destroy(ptree_t t) {
	if (t) {
		node_destroy(t->root);
		free(t);
	}
	return NULL;
}

size_t tree_sizeof() { return sizeof(tree_t); }
ptree_t 
tree_new() {
	ptree_t p;
	if (!(p = (ptree_t) malloc(sizeof(tree_t)))) return NULL;
	p->curr = p->root = NULL;
	return p;
}

/*************************************************************************************************
 * 
 *	Some special undocumented stuff for "friend" modules :
 *  ATTENTION: DO NOT REMOVE THE SAVED ATOM BEFORE RESTORING IT !!!!
 *  If possible the save/restore should be done in a single atomic function or under a lock
 *  to prevent alteration of the tree between save and restore calls !!!!
 *
 *  ------------------------> BE VERY CAREFULL!
 */

/* save/restaure current node: */
void *
tree_pos_save(ptree_t t) {
	if (!t) return NULL;
	return t->curr;
}

void *
tree_pos_restore(ptree_t t, void *pos) {
	if (!t || !pos) return NULL;
	return t->curr = (pnode_t)pos;
}

/* navigation: */
int 
tree_children_count(ptree_t t) {
	if (!t)        return -1;
	if (!t->curr)  return -2;
	return t->curr->nchildren;
}

void *
tree_current(ptree_t t) {
	if (!t) return NULL;
	return (void *) t->curr;
}

void *
tree_root(ptree_t t) {
	if (!t) return NULL;
	return t->curr = t->root;
}

void *
tree_down(ptree_t t) {
	if (!t) return NULL;
	if (!t->curr || !t->curr->parent) return NULL;
	return t->curr = t->curr->parent;
}

void *tree_parent(ptree_t t) { return tree_down(t); }

void *tree_child_first(ptree_t t) {
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->nchildren)  return NULL;
	return (void *) t->curr->children[t->curr->curr_child = 0];
}

void *tree_child_next(ptree_t t) {
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->nchildren)  return NULL;
	if (t->curr->nchildren <= t->curr->curr_child + 1) return NULL;
	return t->curr->children[++(t->curr->curr_child)];
}

void *tree_child_prev(ptree_t t) {
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->nchildren)  return NULL;
	if (!t->curr->curr_child) return NULL;
	return t->curr->children[--(t->curr->curr_child)];
}

void *tree_child_last(ptree_t t) {
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->nchildren)  return NULL;
	return (void *) t->curr->children[t->curr->curr_child = t->curr->nchildren - 1];
}

void *tree_up(ptree_t t) {
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->nchildren)  return NULL;
	return (void *) (t->curr = t->curr->children[t->curr->curr_child]);
}

void *tree_child(ptree_t t) {
	return tree_up(t);
}

void *
tree_brother_first(ptree_t t) {
	pnode_t parent;
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->parent)     return NULL;

	parent = t->curr->parent;
	if (!parent->nchildren)   return NULL;
	return (void *) (t->curr = parent->children[parent->curr_child = 0]);
}

void *
tree_brother_last(ptree_t t) {
	pnode_t parent;
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->parent)     return NULL;

	parent = t->curr->parent;
	if (!parent->nchildren)   return NULL;
	return (void *) (t->curr = parent->children[parent->curr_child = parent->nchildren - 1]);
}

void *
tree_brother_next(ptree_t t) {
	pnode_t parent;
	if (!t)                   return NULL;
	if (!t->curr)             return NULL;
	if (!t->curr->parent)     return NULL;

	parent = t->curr->parent;
	if (parent->curr_child >= parent->nchildren - 1) return NULL;
	return (void *) (t->curr = parent->children[++(parent->curr_child)]);
}

void *
tree_brother_prev(ptree_t t) {
	pnode_t parent;
	if (!t)		              return NULL;
	if (!t->curr) 	          return NULL;
	if (!t->curr->parent)     return NULL;
	parent = t->curr->parent;
	if (!parent->curr_child)  return NULL;
	return (void *) (t->curr = parent->children[--(parent->curr_child)]);
}

void *
tree_children_realloc(ptree_t t, int sign) {
	void *p;
	if (!t)       return NULL;
	if (!t->curr) return NULL;
	if (t->curr->children) 
		p = realloc(t->curr->children, (t->curr->nchildren + sign) * sizeof(pnode_t));
	else  
		p = malloc(sizeof(pnode_t));

	if (!p)       return NULL;
	t->curr->children = p;
	return p;
}

/* add/delete: */
void *
tree_child_add(ptree_t t, void *data) {
	pnode_t n;

	if (!t) return NULL;
	n = node_new();
	n->data = data;

	n->curr_child = n->nchildren = 0;
	n->children = NULL;

	if (!t->curr) {
		t->root = t->curr = n;
		n->parent  = NULL;
	} else if (tree_children_realloc(t, 1)) {
		t->curr->children[t->curr->curr_child = (t->curr->nchildren)++] = n;
		n->parent = t->curr;
	}
	return data;
}

void *
tree_child_insert(ptree_t t, void *data) {
	pnode_t n, c;

	if (!t) return NULL;

	if (!t->curr || !t->curr->nchildren)
		return tree_child_add(t, data);

	n = node_new();
	n->data = data;

	n->curr_child = n->nchildren = 0;
	n->children = NULL;

	if (!tree_children_realloc(t, 1)) return NULL;

	c = t->curr;

	/* move all children: */
	memmove(c->children + c->curr_child + 1, c->children + c->curr_child, (c->nchildren - c->curr_child) * sizeof(pnode_t)); 

	c->nchildren++;
	c->children[c->curr_child] = n;
	n->parent = t->curr;

	return data;
}

void *
tree_child_del(ptree_t t) {
	void *p;
	pnode_t c, o;

	if (!t) return NULL;
	if (!(c = t->curr)) return NULL;
	
	if (!c->nchildren) {
		/* no child left: */
		if (c->parent) {
			/* delete curr_child in parent: */
			tree_down(t);
			return tree_child_del(t);
		} 
		/* delete curr node (which is also the last node): */
		p = c->data;
		node_destroy(c);
		t->curr = t->root = NULL;
		return p;
	}
	
	/* keep node for deletion: */
	o = c->children[c->curr_child];

	/* keep data pointer to return: */
	p = o->data;

	/* Actually destroy node */
	node_destroy(o);
	
	/* Vacuum */
	if (c->curr_child == c->nchildren - 1) c->curr_child--;
	else memmove(c->children + c->curr_child, c->children + c->curr_child + 1, (c->nchildren - c->curr_child - 1) * sizeof (pnode_t));

	/* realloc */
	tree_children_realloc(t, -1);
	
	/* decrease children count: */
	c->nchildren--;

	/* return pointer to data: */
	return p;
}

/* reparent: */
void *
tree_subtree_add(ptree_t dest, ptree_t child) {
	if (!dest || !child) return NULL;	

	if (!dest->curr) { /* Destination tree is emty: */
		/* add a new empty node: */
		tree_child_add(dest, NULL);
		
		/* free that new node: */
		free(dest->curr);

		/* destination */
		dest->root = dest->curr = child->root;
		free(child);
		return (void *) dest->curr;
	} 

	/* create a new node: */
	tree_child_add(dest, NULL);		

	/* free the new node: */
	free(dest->curr->children[dest->curr->curr_child]);

	/* set the child subtree root node instead: */  
	dest->curr->children[dest->curr->curr_child] = child->root;

	/* reparent the subtree child root node: */
	child->curr->parent = dest->curr;

	/* free child subtree structure: */
	child->root = child->curr = NULL;
	free(child);
	
	return (void *) dest->curr;
}

void *
tree_subtree_insert(ptree_t dest, ptree_t child) {
	if (!dest || !child) return NULL;
	if (!dest->curr || !dest->curr->nchildren) 
		return tree_subtree_add(dest, child);

	/* insert a new empty node: */
	tree_child_insert(dest, NULL);

	free(dest->curr->children[dest->curr->curr_child]);
	/* set the child subtree root node instead: */  
	dest->curr->children[dest->curr->curr_child] = child->root;

	/* reparent the subtree child root node: */
	child->curr->parent = dest->curr;

	/* free child subtree structure: */
	child->root = child->curr = NULL;
	tree_destroy(child);
	
	return (void *) dest->curr;
}

ptree_t 
tree_subtree_cut(ptree_t t) {
	pnode_t n, s;
	ptree_t st;
	if (!t || !t->curr)     return NULL;
	if (t->curr == t->root) return NULL;

	if (!(st = tree_new())) return NULL;

	s = t->curr->parent;	

	t->curr->parent = NULL;

	/* The new root is current node: */
	st->root = st->curr = t->curr;

	/* create an empty node: */
	n = node_new();

	/* Plug this note as our current node: */
	n->parent = s;
	n->parent->children[n->parent->curr_child] = n;

	/* replace current node by our empty node: */
	t->curr = n;
	
	/* And delete our new current empty node: */
	tree_child_del(t);

	/* return subtree: */
	return st;
}

ptree_t
tree_subtree_destroy(ptree_t t) {
	pnode_t n;
	if (!t || !t->curr) return NULL;
	n = t->curr;
	tree_down(t);
	node_destroy(n);
	return t;
}

static int tree_node_foreach(pnode_t n, void * param, int count, int depth, void (*userf)(void *, int, int, void *)) {
	int i;
	
	if (!n) return count;		
	
	userf(param, ++count, depth, n);

	++depth;
	for (i = 0; i < n->nchildren; i++) 
	 	count = tree_node_foreach(n->children[i], param, count, depth, userf);
	
	return count;
}

int
tree_subtree_foreach(ptree_t t, void *param, void (*userf)(void *, int, int, void *)) {
	if (!t || !t->curr) return 0;
	return tree_node_foreach(t->curr, param, 0, 0, userf);
}

int 
tree_foreach(ptree_t t, void *param, void (*userf)(void *, int, int, void *)) {
	if (!t || !t->curr) return 0;	
	return tree_node_foreach(t->root, param, 0, 0, userf);
}

#ifdef _test_tree_
#include <stdio.h>

void
tree_node_print(void *param, int count, int depth, void *node) {
	pnode_t n;
	int i;
	
	n = (pnode_t) node;
	
	printf("%4d: level: %4d:", count, depth);
	for (i = 0; i <= depth; i++) printf("  ");
	printf("node x%08x parent x%09x with %d children (current = %d) -> data: ", n, n->parent, n->nchildren, n->curr_child);
	printf("%2d\n", (long) n->data);
}

void
_tree_dump(ptree_t t) {
	if (!t) return;
	printf("@%08x root = %08x current = %08x: \n", t, t->root, t->curr);
	tree_foreach(t, NULL, tree_node_print);
	printf("--------\n");
}

#define tree_dump(x) { printf("tree %s ", #x); _tree_dump(x); }
#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }

int main(void) {
	ptree_t t, s;
	t = tree_new();
	echo_cmd(tree_child_add(t, (void *) 1));
	tree_dump(t);
	echo_cmd(tree_child_add(t, (void *) 2));
	tree_dump(t);
	echo_cmd(tree_child_add(t, (void *) 4));
	tree_dump(t);

	echo_cmd(tree_child_add(t, (void *) 5));
	tree_dump(t);
	
	echo_cmd(tree_child_prev(t));

	tree_dump(t);
	echo_cmd(tree_child_insert(t, (void *) 3));
	tree_dump(t);

	echo_cmd(tree_child_first(t));	
	echo_cmd(tree_child(t));

	echo_cmd(tree_child_add(t, (void *) 6));
	echo_cmd(tree_child_add(t, (void *) 7));
	echo_cmd(tree_brother_next(t));
	echo_cmd(tree_child_add(t, (void *) 9));
	echo_cmd(tree_child_add(t, (void *) 10));
	echo_cmd(tree_brother_next(t));	
	echo_cmd(tree_child_add(t, (void *) 12));
	echo_cmd(tree_child_add(t, (void *) 13));

	echo_cmd(tree_brother_prev(t));
	echo_cmd(tree_child_add(t, (void *) 11));

	echo_cmd(tree_brother_first(t));
	echo_cmd(tree_child_add(t, (void *) 8));

	echo_cmd(tree_brother_last(t));
	echo_cmd(tree_child_insert(t, (void *) 20));
	echo_cmd(tree_child_insert(t, (void *) 17));
	echo_cmd(tree_child_insert(t, (void *) 16));
	echo_cmd(tree_child_last(t));
	echo_cmd(tree_child_insert(t, (void *) 18));
	echo_cmd(while (tree_child_prev(t)));
	echo_cmd(tree_child_insert(t, (void *) 14));
	echo_cmd(tree_child_next(t));	
	echo_cmd(tree_child_insert(t, (void *) 15));
	echo_cmd(while (tree_child_next(t)));
	echo_cmd(tree_child_insert(t, (void *) 19));
	echo_cmd(tree_child_add(t, (void *) 21));
	
	tree_dump(t);

	echo_cmd(s = tree_new());
	echo_cmd(tree_child_add(s, (void *) 11));
	echo_cmd(tree_child_add(s, (void *) 111));
	echo_cmd(tree_child_add(s, (void *) 112));
	echo_cmd(tree_child_add(s, (void *) 113));
	echo_cmd(tree_child_add(s, (void *) 114));
	echo_cmd(tree_dump(s));

	echo_cmd(tree_root(t));
	echo_cmd(tree_subtree_add(t, s));

	tree_dump(t);

	echo_cmd(tree_child(t));
	echo_cmd(s = tree_subtree_cut(t));
	echo_cmd(tree_child_first(t));
	echo_cmd(tree_subtree_insert(t, s));
	
	tree_dump(t);
		
 	echo_cmd(tree_destroy(t));	
	return 0;
}
#endif
